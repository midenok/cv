/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

#ifdef __tcptransp_h
class PTcpTransport;

#else
#define __tcptransp_h

#include "transp.h"
#include "udtmuxer.h"
//#include "udttransp.h"

#ifdef WIN32
#include "dotnet/win32.h"
#endif

#ifndef WIN32
#define serrno errno
#define sclose(S) \
    if (close(S)) { error(1, string("Error! ") + strerror(errno)); exit (1); }
#define serror(E) strerror(E)
typedef int SOCKET;

#else // WIN32
#define EWOULDBLOCK WSAEWOULDBLOCK
#define ENOTCONN WSAENOTCONN
#define serrno WSAGetLastError()
#define sclose(S) closesocket(S)
#define serror(E) WSAGetLastErrorMessage("", E)
//typedef u_long in_addr_t;
#endif

// #include <socket_include.h>
#define TCP_BUFSIZE_READ 16400

//! Транспорт TCP потоков
/*! Передаёт данные с TCP-сокета смежному транспорту.\n \n
*   О понятии \b транспорт см. \link PTransport PTransport подробное описание \endlink
*/

class PTcpTransport : public PTransport<PUdtMuxer, SOCKET>
{
    friend class PUdtMuxer;

public:
    PTcpTransport();
    ~PTcpTransport();
    //! \copydoc PTransport::ServiceLoop
    void ServiceLoop(SOCKET);
    void RecieveLoop();
    //! Проверка состояния трэда транспорта
    /*! Возвращает \b true если трэд транспорта находится в состоянии завершения.
    *   Смежный транспорт должен вызывать этот метод перед тем, как остановить этот транспорт. \n \n
    *   На данный момент CheckShutting используется только, когда в PUdtMuxer пришёл запрос на разрыв канала.
    *   Но PTcpTransport уже может быть в состоянии завершения, поскольку его сокет разорвал связь.
    */
    bool CheckShutting();
    void ShutdownThread();
    void StartClient();
    static void * ClientWrapper(PTcpTransport *);
    void ClientThread();
    void Connect();
    bool Send(buffer_t buf, uint size);
    static void InitMutexes();

private:
    SOCKET mPeer;
    bool mBroken;
    pthread_t mThread;
    mutex_t mShutting;
};

#endif //__tcptransp_h
