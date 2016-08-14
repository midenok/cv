//! \cond undoc

#ifndef __win32_h
#define __win32_h
#ifdef WIN32

#include <winsock2.h>
#include <Ws2tcpip.h>

typedef unsigned int uint;

typedef unsigned int uint32_t;
typedef unsigned char u_char;
typedef unsigned long u_long;

/*
struct in_addr {
    unsigned long s_addr;  // load with inet_aton()
};
*/

#include "ws-util.h"

#define IN_CLASSA_NSHIFT	24
#define IN_CLASSA_HOST      0x00ffffff
#define IN_CLASSB_NSHIFT    16
#define IN_CLASSB_HOST      0x0000ffff
#define IN_CLASSC_NSHIFT    8
#define IN_CLASSC_HOST      0x000000ff

struct in_addr inet_makeaddr(u_long, u_long);

#define snprintf _snprintf

#endif // WIN32
#endif // __win32_h

//! \endcond
