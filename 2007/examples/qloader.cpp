/* 
    Copyright (c) 2006 +sigi1. All rights reserved.
*/

//---------------------------------------------------------------------------
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#pragma hdrstop

//---------------------------------------------------------------------------

#include "utils.h"

inline char * lower_case (char * str)
{
    for (char * c = str; *c; ++c)
        *c = tolower(*c);
    return str;
}

void gain_debug_privilege()
{
    HANDLE master = GetCurrentProcess();
    HANDLE master_token;
    check_errors (OpenProcessToken (master, TOKEN_ALL_ACCESS, &master_token));
    LUID debug_privilege;
    check_errors (LookupPrivilegeValue(TEXT(""), SE_DEBUG_NAME, &debug_privilege));
    TOKEN_PRIVILEGES tkp;
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = debug_privilege;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(master_token, FALSE, &tkp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
    {
        DWORD error = GetLastError();
        if (error != ERROR_SUCCESS) {
            print_win_error(error);
            exit (1);
        }
    }
    check_errors (CloseHandle(master_token));
}

DWORD find_process_by_exe(const char * process_exe)
{
    HANDLE WINAPI th = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    check_errors (th != (HANDLE)-1);
    PROCESSENTRY32 pe;
    memset(&pe, sizeof(pe), 0);
    pe.dwSize = sizeof(pe);
    check_errors (Process32First(th, &pe));
    DWORD pid = 0;
    do {
        if (0 == strncmp(pe.szExeFile, process_exe, MAX_PATH)) {
            pid = pe.th32ProcessID;
            break;
        }
    } while (Process32Next(th, &pe));
    check_errors (CloseHandle(th));
    return pid;
}

bool find_process_module_by_name(DWORD pid, const char * module_name, MODULEENTRY32 & module_data)
{
    HANDLE WINAPI th = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    check_errors (th != (HANDLE)-1);
    MODULEENTRY32 me;
    memset(&me, sizeof(me), 0);
    me.dwSize = sizeof(me);
    char mod_name [MAX_MODULE_NAME32 + 1];
    strncpy (mod_name, module_name, MAX_MODULE_NAME32);
    lower_case (mod_name);
    check_errors (Module32First(th, &me));
    register bool found = false;
    do {
        // printf ("%X %X %08X %7X %-25s %s\n", me.th32ModuleID, me.th32ProcessID, me.modBaseAddr, me.modBaseSize, lower_case(me.szModule), lower_case(me.szExePath));
        if (0 == strncmp(lower_case(me.szModule), mod_name, MAX_MODULE_NAME32)) {
            memcpy(&module_data, &me, sizeof(me));
            found = true;
            break;
        }
    } while (Module32Next(th, &me));
    check_errors (CloseHandle(th));
    return found;
}

typedef struct callback_s
{
    DWORD pid_in;
    DWORD tid_out;
} callback_t;

BOOL CALLBACK ftbp_enum_proc (HWND hwnd, LPARAM lParam)
{
    callback_t &callback = *(callback_t *) lParam;
    DWORD pid;
    DWORD tid = GetWindowThreadProcessId (hwnd, &pid);
    check_errors (tid);
    if (pid == callback.pid_in) {
        callback.tid_out = tid;
        return FALSE;
    }
    return TRUE;
}

DWORD find_thread_by_process(DWORD pid)
{
    callback_t callback = { pid, 0 };
    EnumWindows ((WNDENUMPROC)ftbp_enum_proc, (LPARAM) &callback);
    return callback.tid_out;
}

bool suspend_not_in_range(HANDLE thread, ulong start, ulong end)
{
    for (uint elapsed = 0; elapsed < 10000; elapsed += 5, Sleep(5)) {
        check_errors ((unsigned)-1 != SuspendThread(thread));
        CONTEXT ctx;
        memset (&ctx, sizeof(ctx), 0);
        ctx.ContextFlags = CONTEXT_CONTROL;
        check_errors (GetThreadContext (thread, &ctx));
        if (ctx.SegCs != 0x1b || ctx.Eip < start || ctx.Eip > end)
            return true;
        check_errors ((unsigned)-1 != ResumeThread(thread));
    }
    return false;
}

#define SEM_SLAVE "sem.quoteextractor.slave"

const char * SLAVE_EXE = "PoorVictimApp";
const char * SLAVE_DLL = "poor_dll.dll";
const char * EXTRACTOR_DLL = "qextract.dll";
const char * EXTRACTOR_PROC = "quote_hook";
const offset_t HORSE_ADDR = 0x1234;
const offset_t LOG_END = 0x5678; // log end
const offset_t EXCHANGE_BOX = 0x89ab; // some cosy place in dll for data exchange
const offset_t QUOTE_CALL = 0xcdef; // when screen is updated
const offset_t GETMODHANDLE_ENTRY = 0x12345; // in slave dll
// const char * MSVCRT_DLL = "msvcrt.dll";
// const offset_t GETPROCADDR_ENTRY = 0x320cc; // in msvcrt.dll

#define EXCHANGE_STRING_LEN 32
typedef struct exchange_box_s
{
    char semaphore_name[EXCHANGE_STRING_LEN];
    char dll_name[EXCHANGE_STRING_LEN];
    char dll_proc_name[EXCHANGE_STRING_LEN];
    char kernel32_name[EXCHANGE_STRING_LEN];
    char opensemaphore_name[EXCHANGE_STRING_LEN];
    // char getlasterr_name[EXCHANGE_STRING_LEN];
    // char fmtmessage_name[EXCHANGE_STRING_LEN];
    char exitprocess_name[EXCHANGE_STRING_LEN];
    char loadlibrary_name[EXCHANGE_STRING_LEN];
    char closehandle_name[EXCHANGE_STRING_LEN];
    char releasesem_name[EXCHANGE_STRING_LEN];
    int32 getproc_offset;
    FARPROC dll_proc_out;
} exchange_box_t;

const size_t exchange_box_size = sizeof(exchange_box_t);

void troyan_horse ()
{
    char * base;
    // detect base
    asm {
        pushad;
        db 0xe8; dd 0; // call +0
        pop eax;
        sub eax, 12; // hack -- check for correct address offset
        mov base, eax;
    }
    base -= HORSE_ADDR;
    exchange_box_t * eb = (exchange_box_t *)base;
    (char *) eb += EXCHANGE_BOX;

    ////////////////////////////////////////////////////////////////////////////
    // retrieve system api
    HMODULE _stdcall (* _GetModuleHandle) (LPCTSTR) = (HMODULE _stdcall (*) (LPCTSTR))*(DWORD *)(base + GETMODHANDLE_ENTRY);
    FARPROC _stdcall (* _GetProcAddress) (HMODULE, LPCSTR) = (FARPROC _stdcall (*) (HMODULE, LPCSTR))((int32)_GetModuleHandle + eb->getproc_offset);

    HMODULE kernel32_mod = _GetModuleHandle (eb->kernel32_name); //unguarded because _ExitProcess is not known yet
    VOID _stdcall (* _ExitProcess) (UINT) = (VOID _stdcall (*) (UINT)) _GetProcAddress (kernel32_mod, eb->exitprocess_name);

    HANDLE _stdcall (* _OpenSemaphore) (DWORD, BOOL, LPCTSTR) = (HANDLE _stdcall (*) (DWORD, BOOL, LPCTSTR)) _GetProcAddress (kernel32_mod, eb->opensemaphore_name);
    check_err2 (_OpenSemaphore);
    // DWORD (* _GetLastError) (VOID) = (DWORD (*) (VOID)) _GetProcAddress (kernel32_mod, eb->getlasterr_name);
    // DWORD (* _FormatMessage) (DWORD, LPCVOID, DWORD, DWORD, LPTSTR, DWORD, va_list *) = (DWORD (*) (DWORD, LPCVOID, DWORD, DWORD, LPTSTR, DWORD, va_list *)) _GetProcAddress (kernel32_mod, eb->fmtmessage_name);
    HINSTANCE _stdcall (* _LoadLibrary) (LPCTSTR) = (HINSTANCE _stdcall (*) (LPCTSTR)) _GetProcAddress (kernel32_mod, eb->loadlibrary_name);
    check_err2 (_LoadLibrary);
    BOOL _stdcall (* _CloseHandle) (HANDLE) = (BOOL _stdcall (*) (HANDLE)) _GetProcAddress (kernel32_mod, eb->closehandle_name);
    check_err2 (_CloseHandle);
    BOOL _stdcall (* _ReleaseSemaphore) (HANDLE, LONG, LPLONG) = (BOOL _stdcall (*) (HANDLE, LONG, LPLONG)) _GetProcAddress (kernel32_mod, eb->releasesem_name);
    check_err2 (_ReleaseSemaphore);

    ////////////////////////////////////////////////////////////////////////////
    // get the things done
    HANDLE sem = _OpenSemaphore (SEMAPHORE_MODIFY_STATE, false, eb->semaphore_name);
    check_err2(sem);
    // load the dll
    HANDLE dll = _LoadLibrary (eb->dll_name);
    check_err2 (dll);
    // put some dll procedure address in exchange_box
    check_err2 (eb->dll_proc_out = _GetProcAddress (dll, eb->dll_proc_name));
    // check_err2 (_CloseHandle(dll));
    // now we have done our dirty job. let's master continue its dirty job!
    check_err2 (_ReleaseSemaphore (sem, 1, NULL));
    asm { // TODO: may be put this before ReleaseSemaphore
        popad;
        mov esp, ebp;
        pop ebp;
    }
    while (true) {
        asm { nop; nop; nop; nop; nop; nop; nop; nop; }
    }
}

void troyan_horse_end ()
{
}

#pragma argsused
int main(int argc, char* argv[])
{
    gain_debug_privilege();

    DWORD slave_pid = find_process_by_exe(SLAVE_EXE);
    check_err3 (slave_pid, "Program process was not found. May be it is not running?");

    DWORD slave_tid = find_thread_by_process(slave_pid);
    check_err3 (slave_tid, "Strange thing: main thread was not found!");

    MODULEENTRY32 slave_mod;
    check_err3 (find_process_module_by_name (slave_pid, SLAVE_DLL, slave_mod),
        "Unusual thing: slave module was not found!");

    /* MODULEENTRY32 msvcrt_mod;
    check_err3 (find_process_module_by_name (slave_pid, MSVCRT_DLL, msvcrt_mod),
        "Unusual thing: msvcrt module was not found!"); */

    HANDLE slave = OpenProcess (PROCESS_ALL_ACCESS, false, slave_pid);
    check_errors (slave);
    HANDLE slave_thread = OpenThread (THREAD_ALL_ACCESS, false, slave_tid);
    ulong start = (ulong)slave_mod.modBaseAddr + HORSE_ADDR;
    ulong end = (ulong)slave_mod.modBaseAddr + LOG_END;
    check_errors (slave_thread);
    check_err3 (suspend_not_in_range(slave_thread, start, end), "Could not suspend thread!");
    HANDLE slave_sem = CreateSemaphore (NULL, 0, 1, SEM_SLAVE);
    check_errors (slave_sem != NULL && GetLastError() == 0);

    ////////////////////////////////////////////////////////////////////////////
    // troyan_horse and exchange box data copy


    size_t code_length = (ulong)troyan_horse_end - (ulong)troyan_horse;

    // save old data and code
    char * saved_data = (char *)malloc (exchange_box_size * 2 + code_length);
    char * saved_code = saved_data + exchange_box_size;
    BYTE * & base = slave_mod.modBaseAddr;
    check_errors (ReadProcessMemory(slave, EXCHANGE_BOX + base, saved_data, exchange_box_size, NULL));
    check_errors (ReadProcessMemory(slave, HORSE_ADDR + base, saved_code, code_length, NULL));

    // write new data and code
    exchange_box_t * eb = (exchange_box_t *)saved_code;
    (char *)eb += code_length;
#define safe_copy(DST, STR, MAXLEN) { \
    register size_t l; \
    l = strlen(STR) + 1; \
    check_err3 (l <= MAXLEN, "Error: string length exceeds capacity!"); \
    memcpy (DST, STR, l); \
}
    safe_copy (eb->semaphore_name, SEM_SLAVE, EXCHANGE_STRING_LEN);
    safe_copy (eb->dll_name, EXTRACTOR_DLL, EXCHANGE_STRING_LEN);
    safe_copy (eb->dll_proc_name, EXTRACTOR_PROC, EXCHANGE_STRING_LEN);
    safe_copy (eb->kernel32_name, "kernel32.dll", EXCHANGE_STRING_LEN);
    safe_copy (eb->opensemaphore_name, "OpenSemaphoreA", EXCHANGE_STRING_LEN);
    // safe_copy (eb->getlasterr_name, "GetLastError", EXCHANGE_STRING_LEN);
    // safe_copy (eb->fmtmessage_name, "FormatMessageA", EXCHANGE_STRING_LEN);
    safe_copy (eb->exitprocess_name, "ExitProcess", EXCHANGE_STRING_LEN);
    safe_copy (eb->loadlibrary_name, "LoadLibraryA", EXCHANGE_STRING_LEN);
    safe_copy (eb->closehandle_name, "CloseHandle", EXCHANGE_STRING_LEN);
    safe_copy (eb->releasesem_name, "ReleaseSemaphore", EXCHANGE_STRING_LEN);

    // compute GetProcAddress offset from GetModuleHandleA
    {
        HMODULE kernel32 = GetModuleHandle("kernel32.dll");
        check_errors (kernel32);
        FARPROC getprocaddress = GetProcAddress(kernel32, "GetProcAddress");
        check_errors (getprocaddress);
        FARPROC getmodhandle = GetProcAddress(kernel32, "GetModuleHandleA");
        eb->getproc_offset = (DWORD)getprocaddress - (DWORD)getmodhandle;
    }

    printf ("Breaching slave address space... ");
    check_errors (WriteProcessMemory(slave, EXCHANGE_BOX + base, eb, exchange_box_size, NULL));
    printf ("ok!\nBeware: do not interrupt or slave will freeze!\n");
    check_errors (WriteProcessMemory(slave, HORSE_ADDR + base, troyan_horse, code_length, NULL));

    ////////////////////////////////////////////////////////////////////////////
    // that troyan_horse executes and releases sem.quoteextractor.slave
    // and waits infinitely in simple loop (not in system call
    // like WaitForSingleObject or Sleep because it could harm the system)
    check_errors ((unsigned)-1 != ResumeThread (slave_thread));
    printf("Waiting for quote... ");
    check_errors (WAIT_FAILED != WaitForSingleObject (slave_sem, INFINITE));
    check_errors (CloseHandle (slave_sem));
    Sleep (100); // wait little more to be sure it did mov esp, ebp; pop ebp;
    check_errors ((unsigned)-1 != SuspendThread (slave_thread));
    printf ("ok!\n");
    // read dll_proc_out
    offset_t dp_offset = (char *)&eb->dll_proc_out - (char *)eb;
    check_errors (ReadProcessMemory(slave, dp_offset + EXCHANGE_BOX + base, &eb->dll_proc_out, 4, NULL));
    // put quote hook
    char new_call[6] = { 0xe8, 0, 0, 0, 0, 0x90 };
    char * new_call_addr = QUOTE_CALL + base;
    *(DWORD *)(new_call + 1) = (DWORD)((char *)eb->dll_proc_out - new_call_addr - 5);
    check_errors (WriteProcessMemory(slave, new_call_addr, new_call, 6, NULL));
    // old code and data restore (look at stack to restore registers,
    // check that esp is correct)
    check_errors (WriteProcessMemory(slave, EXCHANGE_BOX + base, saved_data, exchange_box_size, NULL));
    check_errors (WriteProcessMemory(slave, HORSE_ADDR + base, saved_code, code_length, NULL));
    // set EIP to where it should be
    CONTEXT ctx;
    memset (&ctx, sizeof(ctx), 0);
    ctx.ContextFlags = CONTEXT_FULL;
    check_errors (GetThreadContext (slave_thread, &ctx));
    ctx.Eip = HORSE_ADDR + (ulong)base;
    check_errors (SetThreadContext (slave_thread, &ctx));
    // resume thread
    check_errors ((unsigned)-1 != ResumeThread (slave_thread));
    printf ("All done! Slave normal work resumed.\n");
    // free resources
    free (saved_data);
    check_errors (CloseHandle(slave_thread));
    check_errors (CloseHandle(slave));
    printf ("Success!\n");
    return 0;
}
//---------------------------------------------------------------------------
