/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

#include "tcpserv.h"
#include "app.h"

#include <string>

using namespace std;

#define LISTEN_DEPTH 30

PTcpServer::PTcpServer() : mSock(0)
{
    cond_init (mTempSockWait);
}

PTcpServer::~PTcpServer()
{
    debug_entering(PTcpServer::~PTcpServer);
    cond_destroy (mTempSockWait);
    if (mSock)
        sclose(mSock);
    return_none();
}

/*! Стартует сервис на сконфигурированном сокете. Ожидает в цикле входящие соединения и для каждого из них
*   в новом трэде запускает ServiceThread.
*/

void PTcpServer::StartService()
{
    debug_entering(PTcpServer::StartService);
    if (Bind(&gApp.mBindSocket.addr)) {
        error (1, serror(serrno));
        exit( -1 );
    }
    while (true) {
        Accept();
    }
    return_none();
}

int PTcpServer::Bind(sockaddr_t * addr)
{
    debug_entering(PTcpServer::Bind);
    mSock = socket( AF_INET, SOCK_STREAM, 0 );
    if (mSock == -1) {
        error (1, serror(serrno));
        return_int(-1);
    }

    if (bind(mSock, addr, sockaddr_size) == -1) {
        error (1, serror(serrno));
        sclose(mSock);
        return_int (-1);
    }

    sockaddr_in_t * & addr_in = (sockaddr_in_t * &)addr;
    debug (DBG_GENERAL, 2, string("listening at ") + inet_ntoa(addr_in->sin_addr) + ":" + ntohs(addr_in->sin_port));

    if ( listen( mSock, LISTEN_DEPTH ) == -1 ) {
        error (1, serror(serrno));
        sclose(mSock);
        return_int (-1);
    }
    return_int (0);
}

void PTcpServer::Accept()
{
    debug_entering(PTcpServer::Accept);
    socklen_t sa_len = sizeof(struct sockaddr);
    debug(DBG_TRANSFER, 5, "TCP waiting for connection");
    mTempSock = accept (mSock, &mTempSockAddr, &sa_len);

    if (mTempSock == -1) {
        error(1, serror(serrno));
        return_none();
    }

    thread_run(PTcpServer::ThreadWrapper);
    cond_wait(mTempSockWait);
    return_none();
}

void * PTcpServer::ThreadWrapper( PTcpServer * s )
{
    s->ServiceThread();
    return NULL;
}
