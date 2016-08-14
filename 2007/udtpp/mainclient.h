/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

#ifndef __mainclient_h
#define __mainclient_h

#include "tcpserv.h"

//! ������ ����������

/*! ������� TCP ���������� �� �������� ����������� ������ (HTTP, FTP, ICQ)
*   � �������������� ������ �� UDT ������ ������ PMainServer.\n
*   ��� ������������ ���������� � PMainServer � �������� ������ ���������� ��������� PUdtMuxer.
*/

class PMainClient : public PTcpServer, public PSingleton<PMainClient>
{
    friend class PSingleton<PMainClient>;

    public:
    void ServiceThread();

    private:
    PMainClient();
    ~PMainClient();
    PUdtMuxer * mUdtMuxer;
};

#endif // __mainclient_h
