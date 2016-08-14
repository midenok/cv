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

//! ��������� TCP �������
/*! �������� ������ � TCP-������ �������� ����������.\n \n
*   � ������� \b ��������� ��. \link PTransport PTransport ��������� �������� \endlink
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
    //! �������� ��������� ����� ����������
    /*! ���������� \b true ���� ���� ���������� ��������� � ��������� ����������.
    *   ������� ��������� ������ �������� ���� ����� ����� ���, ��� ���������� ���� ���������. \n \n
    *   �� ������ ������ CheckShutting ������������ ������, ����� � PUdtMuxer ���ۣ� ������ �� ������ ������.
    *   �� PTcpTransport ��� ����� ���� � ��������� ����������, ��������� ��� ����� �������� �����.
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
