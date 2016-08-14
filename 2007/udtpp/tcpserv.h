/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

#ifdef __tcpserv_h
class PTcpServer;
#else
#define __tcpserv_h

#include "tcptransp.h"
#include "udtmuxer.h"

#ifndef WIN32
typedef int SOCKET;
#else
#include "dotnet/win32.h"
#endif

// #include <Socket.h>

class PApp;

//! TCP ������

/*! ��������� �������� TCP ����������.\n
*   �� ������������ ���������� ������� ����� ���� � ��������� � Σ� \link PTcpServer::ServiceThread ServiceThread \endlink.
*/

// TODO: generalize PTcpServer & PUdtServer into PServer
class PTcpServer
{
public:
    //! ������ ������� ��� �������� ����������
    void StartService();

protected:
    PTcpServer();
    virtual ~PTcpServer();

    SOCKET mTempSock;
    sockaddr_t mTempSockAddr;
    cond_declare (mTempSockWait);

private:
    static void * ThreadWrapper(PTcpServer * s);
    int Bind(sockaddr_t *);
    void Accept();
    //! ��������� ��������� TCP ����������
    virtual void ServiceThread() = 0;
    SOCKET mSock;
};
#endif //__tcpserv_h
