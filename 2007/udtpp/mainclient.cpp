/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

#include "mainclient.h"

PMainClient::PMainClient()
{
    mUdtMuxer = new PUdtMuxer(gApp.mConnectSocket.addr);
}

PMainClient::~PMainClient()
{
    delete mUdtMuxer;
}

//! ��������� �������� ����������

/*!
*   ��������� \link PUdtTransport::ClientThread ClientThread \endlink ���������� ����� � PMainServer ��� ������ PUdtTransport::StartClient. ��������� ����� � PMainServer -- PUdtMuxer.\n
*   ������� ����� ������ PTcpTransport � ������� ��� � PUdtMuxer. � ������ ������� PTcpTransport ��������� \link PTcpTransport::ServiceLoop ServiceLoop \endlink.\n
*   �������� �������� ��, ��� �������� ������ ���� �������� PTcpTransport ����� �������� ��� ������ PUdtMuxer.
*   � �������� -- �������� PUdtMuxer ����� �������� ���������������� PTcpTransport.
*/

void PMainClient::ServiceThread()
{
    debug_entering(PMainClient::ServiceThread);
    SOCKET peer = mTempSock;

    cond_signal (mTempSockWait);

    debug(DBG_THREADS, 3, "TCP new thread started");
    PTcpTransport tcp;
    mUdtMuxer->SetTransport( &tcp );
    if (!mUdtMuxer->AtLoop(LOOP_CLIENT))
        mUdtMuxer->StartClient();
    tcp.SetTransport(mUdtMuxer);
    tcp.ServiceLoop(peer);
    debug(DBG_THREADS, 3, "TCP thread exited");
    return_none();
}
