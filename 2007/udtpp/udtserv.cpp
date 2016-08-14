/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

#include "udtserv.h"
#include "app.h"
#include "classmuxer.h"
#include "udtmuxer.h"

#include <iostream>

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#endif

using namespace std;

#define LISTEN_DEPTH 30

PUdtServer::PUdtServer() : mReconnect (false), mTempTransport(NULL), mSock(0)
{
    cond_init(mTempSockWait2);
    cond_init(mConnectWait);
}

PUdtServer::~PUdtServer()
{
    cond_destroy(mTempSockWait2);
    cond_destroy(mConnectWait);
    if (mSock)
        UDT::close (mSock);
}

void PUdtServer::StartService()
{
    if (Bind(&gApp.mBindSocket.addr)) {
        exit(-1);
    }
    while (true) {
        Accept();
    }
}

// #define UDT_BUFSIZE 8388608
// #define UDT_BUFSIZE 4194304
#define UDT_BUFSIZE 262144

void PUdtServer::SetSocketOptions(UDTSOCKET mSock)
{
    int bufsize = UDT_BUFSIZE;
    bool enable = true;
    UDT::setsockopt(mSock, 0, UDT_SNDBUF, (char *)&bufsize, sizeof(int));
    UDT::setsockopt(mSock, 0, UDT_RCVBUF, (char *)&bufsize, sizeof(int));
    UDT::setsockopt(mSock, 0, UDP_RCVBUF, (char *)&bufsize, sizeof(int));
    if (gRendezvous)
        UDT::setsockopt(mSock, 0, UDT_RENDEZVOUS, (char *)&enable, sizeof(bool));
}

int PUdtServer::Bind(sockaddr_t * addr)
{
    debug_entering(PUdtServer::Bind);

    mSock = UDT::socket(AF_INET, SOCK_STREAM, 0);
    SetSocketOptions(mSock);

    sockaddr_in_t * & addr_in = (sockaddr_in_t * &)addr;
    debug (DBG_GENERAL, 4, string("binding to ") + inet_ntoa(addr_in->sin_addr) + ":" + ntohs(addr_in->sin_port));

    // TODO: check if bind IP address is correct
    if (UDT::ERROR == UDT::bind(mSock, addr, sockaddr_size)) {
        error(1, string("bind: ") + UDT::getlasterror().getErrorMessage());
        exit (1);
    }

    if (! gRendezvous) {
        debug(DBG_GENERAL, 2, string("listening at ") + inet_ntoa(addr_in->sin_addr) + ":" + ntohs(addr_in->sin_port));

        if (UDT::ERROR == UDT::listen(mSock, LISTEN_DEPTH)) {
            error(1, string("listen: ") + UDT::getlasterror().getErrorMessage());
            return_int (-1);
        }
    }

    return_int (0);
}

void PUdtServer::Accept()
{
    debug_entering(PUdtServer::Accept);
    int addrlen = sockaddr_size;

    debug(DBG_GENERAL, 3, "UDT waiting for connection");
    if (UDT::INVALID_SOCK == (mTempSock = UDT::accept(mSock, &mTempSockAddr, &addrlen))) {
        error (1, UDT::getlasterror().getErrorMessage());
        exit (1);
    }

    thread_run(PUdtServer::ThreadWrapper);
    cond_wait(mTempSockWait2);
    return_none();
}

PUdtTransport * PUdtServer::Reconnect()
{
    debug_entering(PUdtServer::Reconnect);
    mReconnect = true;
    cond_wait(mConnectWait);
    PUdtTransport * t = mTempTransport;
    mReconnect = false;
    return_value (PUdtTransport *, t);
}

void * PUdtServer::ThreadWrapper(PUdtServer * s)
{
    s->ServiceThread();
    return NULL;
}

