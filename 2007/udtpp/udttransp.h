/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

#ifdef __udttransp_h
class PUdtTransport;
#else
#define __udttransp_h

#include "transp.h"
#include "udtserv.h"

#ifdef WIN32
#include "dotnet/win32.h"
#endif

#include <udt.h>

class PUdtMuxer;

//! Транспорт UDT потоков
/*! Передаёт данные с UDT-сокета смежному транспорту.\n \n
*   О понятии \b транспорт см. \link PTransport PTransport подробное описание \endlink
*/

class PUdtTransport : public PTransport<PUdtMuxer, UDTSOCKET>
{
    friend class PTcpTransport;

public:
    PUdtTransport(sockaddr_t &, PUdtServer *);
    ~PUdtTransport();
    static void InitMutex();
    //! \copydoc:brief PTransport::Connect
    /*! \copydoc:detail PTransport::Connect
    */
    void Connect(bool reconnect = false);
    inline void SetSocketOptions();
    bool Send (buffer_t buf, uint size);
    //! \copydoc:brief PTransport::ServiceLoop
    void ServiceLoop(UDTSOCKET);
    void RecieveLoop();
    // static void ShutdownThread(PUdtTransport *);
    static void * ServiceWrapper(class PUdtTransport *);
    void StartClient();
    static void * ClientWrapper(PUdtTransport *);
    void ClientThread();
    void SetBindAddr (sockaddr_t & bind_addr)
    {
        memcpy (&mBindAddr, &bind_addr, sockaddr_size);
    }

private:
    UDTSOCKET mPeer;
    sockaddr_t mPeerAddr;
    sockaddr_in_t mBindAddr;
    PUdtServer * mServer;
    PUdtTransport * mNewConnection;
    mutex_t mReconnect;
};

#endif //__udttransp_h
