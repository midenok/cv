/** \cond undoc */

#include "win32.h"

/*
uint32_t
htonl(x)
uint32_t x;
{
    u_char *s = (u_char *)&x;
    return (uint32_t)(s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3]);
}
*/

/*
 * Formulate an Internet address from network + host.  Used in
 * building addresses stored in the ifnet structure.
 */

struct in_addr
inet_makeaddr(u_long net, u_long host)
{
    u_long addr;

    if (net < 128)
        addr = (net << IN_CLASSA_NSHIFT) | (host & IN_CLASSA_HOST);
    else if (net < 65536)
        addr = (net << IN_CLASSB_NSHIFT) | (host & IN_CLASSB_HOST);
    else if (net < 16777216L)
        addr = (net << IN_CLASSC_NSHIFT) | (host & IN_CLASSC_HOST);
    else
        addr = net | host;
    addr = htonl(addr);
	return (*(struct in_addr *)&addr);
}

/** \endcond */
