/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

#ifdef __udtserv_h
class PUdtServer;
#else
#define __udtserv_h

#include "udttransp.h"

#include <udt.h>

#ifdef WIN32
#include "dotnet/win32.h"
#endif

//#include <Socket.h>

class PApp;

//! UDT Сервер

/*! Принимает входящие UDT соединения.\n
*   По установлении соединения создаёт новый трэд и запускает в нём \link PUdtServer::ServiceThread ServiceThread \endlink.
*/

class PUdtServer
{
    friend class PUdtTransport;

public:
    PUdtServer();
    virtual ~PUdtServer();
    //! \copydoc:brief PTcpServer::StartService
    /*! \copydoc:detail PTcpServer::StartService
    */
    void StartService();
    static void SetSocketOptions(UDTSOCKET mSock);

protected:
    int Bind(sockaddr *);
    virtual PUdtTransport * Reconnect();
    UDTSOCKET mTempSock;
    sockaddr_t mTempSockAddr;
    UDTSOCKET mSock;
    cond_declare (mTempSockWait2);
    cond_declare(mConnectWait);
    PUdtTransport * mTempTransport;
    bool mReconnect;

private:
    static void * ThreadWrapper(PUdtServer * s);
    void Accept();

    //! Обработка входящего UDT соединения
    virtual void ServiceThread() = 0;
};

#endif //__udtserv_h
