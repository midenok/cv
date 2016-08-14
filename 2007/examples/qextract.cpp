#include <windows.h>
#include <stdio.h>

#include "utils.h"

HANDLE log_file;

#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, DWORD reason, LPVOID lpReserved)
{
    switch (reason) {
        case DLL_PROCESS_ATTACH:
        log_file = CreateFile ("qextract.log", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        check_err4 (log_file != INVALID_HANDLE_VALUE);
        break;

        case DLL_PROCESS_DETACH:
        check_err4 (CloseHandle(log_file));
        break;
    }

    return 1;
}

void quote_export(char * arg1, int arg2, int * arg3, double * arg4)
{
}

extern "C" void _stdcall __declspec(dllexport) quote_hook()
{
    // stack:
    // arg_6 - arg_1
    // ret
    // ebp
    // ebx
    // esi
    // edi
    asm {
        mov esi, [ebp + 0x20];
        mov edi, [esi];
        mov eax, [edi + 0xc];
        push eax;
        mov esi, [ebp + 0x1c];
        mov edi, [esi];
        mov eax, [edi + 0xc];
        push eax;
        mov eax, [ebp + 0x18];
        push eax;
        mov eax, [ebp + 0x10];
        push eax;
        call quote_export;
        add esp, 0x10;
        lea esi, [ebp + 8];
        lea edi, [ebp + 4];
        mov ebx, [edi];
        mov ecx, 7;
        repnz movsd;
        mov [edi], ebx;
        pop edi;
        pop esi;
        pop ebx;
        pop ebp;
        // push eax;
        call dword ptr [esi+0x18];
        mov ecx, [ebp-0x50];
        ret;
    }
}

