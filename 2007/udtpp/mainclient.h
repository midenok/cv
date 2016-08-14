/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

#ifndef __mainclient_h
#define __mainclient_h

#include "tcpserv.h"

//! Клиент приложения

/*! Ожидает TCP соединения от клиентов прикладного уровня (HTTP, FTP, ICQ)
*   и перенаправляет данные по UDT каналу классу PMainServer.\n
*   Для установления соединения с PMainServer и передачи данных использует транспорт PUdtMuxer.
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
