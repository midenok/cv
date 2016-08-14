#ifndef __utils_h
#define __utils_h

#define __msg_get(error) \
        char * msg = NULL; \
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, error, \
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), \
                      (LPTSTR)msg, 0, NULL)

#define __print_error(msg) \
        fprintf(stderr, "Error: (%s:%d) %s\n", __FILE__, __LINE__, msg);

#define print_win_error(error) { \
    __msg_get(error); \
    __print_error(msg); \
}

#define show_win_error(error) { \
    __msg_get(error); \
    MessageBox (NULL, msg, "QuoteExtractor error!", MB_OK|MB_ICONERROR|MB_TASKMODAL); \
}


#define check_errors(FUNCTION_CALL) \
    if (!(bool)(FUNCTION_CALL)) { \
        DWORD error = GetLastError(); \
        print_win_error(error); \
        exit (1); \
    }

#define check_err2(FUNCTION_CALL) \
    if (!(bool) (FUNCTION_CALL)) { \
        _ExitProcess (1); \
    }

#define check_err3(TEST_VALUE, ERR_MSG) \
    if (!(bool) (TEST_VALUE)) { \
        __print_error(ERR_MSG); \
        exit (1); \
    }

#define check_err4(FUNCTION_CALL) \
    if (!(bool) (FUNCTION_CALL)) { \
        DWORD error = GetLastError(); \
        show_win_error(error); \
        exit (1); \
    }

typedef unsigned int offset_t;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef int int32;

#endif // __utils_h
