/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

#ifndef __mainserver_h
#define __mainserver_h

#include "udtserv.h"
#include "tcpserv.h"

#include <set>

#define MIN_BIND_PORT 10000
#define MAX_BIND_PORT 20000

using namespace std;

typedef set<port_t> set_port_t;

//! Сервер приложения

/*! Принимает данные от PMainClient и передаёт их на сконфигурированный TCP сокет.\n
    Для связи с PMainClient использует транспорт PUdtMuxer.\n
    Имеет два режима работы: обычный и \link PMainServer::ServiceThread Rendezvous \endlink.\n
    В обычном режиме запускает сервис, как PUdtServer. В Rendezvous режиме используется сервис от PTcpServer.
*/

class PMainServer : public PUdtServer, public PTcpServer, public PSingleton<PMainServer>
{
    friend class PSingleton<PMainServer>;

public:
    void StartService();

protected:
    PMainServer() : mLastReserved(MIN_BIND_PORT-1) {}

private:
    //! Обработка входящих соединений
    void ServiceThread();

    //! Резервирование свободного порта
    port_t GetFreePort();
    set_port_t mPortSet;
    port_t mLastReserved;
};

#endif // __mainserver_h
