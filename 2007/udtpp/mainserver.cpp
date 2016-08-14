/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

#include "mainserver.h"
#include "udtmuxer.h"
#include "rendezvous.h"

#include <string>

using namespace std;

void PMainServer::StartService()
{
    if (gRendezvous)
        PTcpServer::StartService();
    else
        PUdtServer::StartService();
}


/*!
    Запускает \link PUdtTransport::ServiceLoop ServiceLoop \endlink транспорта связи с PMainClient -- PUdtMuxer. \n \n
    О режиме \b Rendezvous протокола UDT см. http://udt.sourceforge.net/doc/doc/t-firewall.htm \n \n
    В режиме Rendezvous для определения параметров UDT соединения используется TCP канал.
    Сервер принимает по TCP каналу магическое число, генерирует номер свободного порта
    для установления UDT соединения и отправляет его по TCP каналу клиенту.
    IP-адрес UDT клиента определяется по входящему TCP соединению, если он явно не сконфигурирован на клиенте.
    В противном случае IP-адрес принимается по TCP каналу вместе с магическим числом.
    Магическое число используется как мера защиты от непредназначенных соединений.
*/

void PMainServer::ServiceThread()
{
    debug_entering(PMainServer::ServiceThread);

    UDTSOCKET udt_peer;
    sockaddr_in_t udt_peer_addr;
    sockaddr_in_t udt_bind_addr;

    debug(DBG_THREADS, 3, "UDT new service thread started");

    if (gRendezvous) {
        cond_signal (mTempSockWait2);
        SOCKET peer = PTcpServer::mTempSock;
        sockaddr_t tcp_peer_addr;
        memcpy (&tcp_peer_addr, &(PTcpServer::mTempSockAddr), sizeof(sockaddr_t));

        cond_signal (mTempSockWait);

        uint magic_num;
        aware_transfer (recv, peer, &magic_num, sizeof(uint), 0);
        if (magic_num != gMagicNumber) {
            debug (DBG_GENERAL, 3, "Warning! Magic number does not match!");
            sclose (peer);
            return_none();
        }
        uint & cmd = magic_num;
        aware_transfer (recv, peer, &cmd, sizeof(uint), 0);
        if (cmd == RZV_CLIENT_IP) {
            aware_transfer (recv, peer, &((sockaddr_in_t &)tcp_peer_addr).sin_addr.s_addr, sizeof(in_addr_t), 0);
        }
        port_t bind_port = GetFreePort();
        aware_transfer (send, peer, &bind_port, sizeof(bind_port), 0);
        socklen_t socklen = sockaddr_size;
        getsockname (peer, (sockaddr_t *)&udt_bind_addr, &socklen);
        sclose (peer);
        udt_bind_addr.sin_port = htons (bind_port);
        PUdtServer::Bind ((sockaddr_t *)&udt_bind_addr);
        memcpy (&udt_peer_addr, &tcp_peer_addr, sockaddr_size);
        udt_peer_addr.sin_port = udt_bind_addr.sin_port;
        // inet_aton("82.200.29.182", &udt_peer_addr.sin_addr);
        if (UDT::ERROR == UDT::connect(PUdtServer::mSock, (sockaddr_t *)&udt_peer_addr, sockaddr_size)) {
            error(2, UDT::getlasterror().getErrorMessage());
            return_none();
        }
        udt_peer = PUdtServer::mSock;
    } else {
        udt_peer = PUdtServer::mTempSock;
        memcpy (&udt_peer_addr, &(PUdtServer::mTempSockAddr), sockaddr_size);
        cond_signal(mTempSockWait2);
    }
    PUdtMuxer * udt_muxer = new PUdtMuxer((sockaddr_t &)udt_peer_addr, this);
    if (mReconnect) {
        mTempTransport = udt_muxer;
        cond_signal(mConnectWait);
        while (mReconnect);
    }
    if (gRendezvous)
        udt_muxer->SetBindAddr((sockaddr_t &)udt_bind_addr);
    udt_muxer->ServiceLoop(udt_peer);
    delete udt_muxer;
    debug(DBG_THREADS, 3, "UDT service thread exited");
    return_none();
}

/*! Резервирует порт из списка свободных портов и возвращает его номер.
*/

port_t PMainServer::GetFreePort()
{
    // TODO: infinite loop if all ports are reserved
    // TODO: ports freeing mechanism
    do {
        if (++mLastReserved > MAX_BIND_PORT)
            mLastReserved = MIN_BIND_PORT;
    } while (mPortSet.count(mLastReserved));
    return mLastReserved;
}
