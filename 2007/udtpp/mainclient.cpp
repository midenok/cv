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

//! Обработка входящих соединений

/*!
*   Запускает \link PUdtTransport::ClientThread ClientThread \endlink транспорта связи с PMainServer при помощи PUdtTransport::StartClient. Транспорт связи с PMainServer -- PUdtMuxer.\n
*   Создаёт новый объект PTcpTransport и стыкует его с PUdtMuxer. У нового объекта PTcpTransport запускает \link PTcpTransport::ServiceLoop ServiceLoop \endlink.\n
*   Стыковка означает то, что принятые данные этим объектом PTcpTransport будут переданы при помощи PUdtMuxer.
*   И наоборот -- принятое PUdtMuxer будет передано соответствующему PTcpTransport.
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
