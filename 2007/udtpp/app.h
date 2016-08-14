/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

#ifdef __app_h
class PApp;
#else
#define __app_h

#include "util.h"

#ifndef WIN32
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#else
#include "dotnet/win32.h"
#endif

#define LOOP_DELAY 2

//! \cond undoc

struct PSocketInfo
{
    sockaddr_t addr;
    char str_ip[16];
    char str_port[6];
};

//! \endcond

bool ParseSocketString(buffer_t str_socket, PSocketInfo & socket);

//! ����������� ���������� �������� ������ ����� ��������

template<class T>
class PSingleton
{
public:
    static T& Instance()
    {
        static T obj;
        return obj;
    }

protected:
    PSingleton<T> () {}
    ~PSingleton() {}

private:
    PSingleton<T> (PSingleton<T> const&);
    PSingleton<T> operator=(PSingleton<T> const&);
};

//! ������� ����������
/*! �������� ��������� ���������� ����������.\n
*   �������� ��������������� ������ ��� ������ ����������. \n
*   ���������� ���������� ���� ��� ������ StartMainClient(), ���� StartMainServer()
*   � ����������� �� ���������� \ref appmode_anchor "������ ������".
*/

class PApp : public PSingleton<PApp>
{
    friend class PSingleton<PApp>;

public:
    //! ������ ���������� � ������ �������
    void StartMainClient();
    //! ������ ���������� � ������ �������
    void StartMainServer();
    //! ��������� ������ ����������
    /*! ������������� �����, � �������� ����� ����������� ����������.
    *   � ������ ������� ������������� ����� UDTPP-�������. � ������ ������� ������������� ����� ������ ����������,
    *   �������� ����� �������� TCP ����������.
    */
    bool SetConnectSocket(buffer_t str_socket) { return ParseSocketString(str_socket, mConnectSocket); }
    //! ��������� ������ �������������
    /*! � ������ ������� ������������� �����, �� ������� ����� ����������� �������� ���������� �� ���������� ��������.
    *   � ������ ������� ������������� �����, �� ������� ����� ����������� ���������� �� UDTPP � ������ �������.
    */
    bool SetBindSocket(buffer_t str_socket) { return ParseSocketString(str_socket, mBindSocket); }
    //! ��������� �������� IP � ������ Rendezvous ��� UDTPP-�������
    /*! ������������� ������� IP-����� UDTPP-������� � ������ Rendezvous ��� ������, ����� UDTPP-������ ��������� ��
    *   NAT-������. ���� �� �������� ��� �������, �� ����� ����� ������������ ������������� UDTPP-�������� (��. PMainServer::ServiceThread()).
    */
    bool SetRendezvousIP(buffer_t str_ip);

    //! \copydoc:brief SetBindSocket()
    /*! \copydoc:detail SetBindSocket()
    */
    void SetBindSocket(uint32_t ip, port_t port)
    {
        sockaddr_in_t * addr = (sockaddr_in_t *) &mBindSocket.addr;
        addr->sin_family = AF_INET;
        addr->sin_addr = inet_makeaddr(ip, 0);
        addr->sin_port = htons(port);
        strncpy(mBindSocket.str_ip, inet_ntoa(addr->sin_addr), 15);
        mBindSocket.str_ip[15] = 0;
        snprintf(mBindSocket.str_port, 6, "%u", port);
    }
    //! ��������� ���������� ������ ���������� ����������
    /*! ������������� ������ �� ����� ���������� ����������
    */
    bool SetDebugOptions(buffer_t dbgopt);
    PSocketInfo mConnectSocket;
    PSocketInfo mBindSocket;
    in_addr_t mRendezvousIP;

protected:
    PApp ();
    ~PApp ();
};

#endif // __app_h
