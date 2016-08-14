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

//! Ограничение количества объектов класса одним объектом

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

//! Стартер приложения
/*! Содержит параметры исполнения приложения.\n
*   Содержит вспомогательные методы для старта приложения. \n
*   Приложение стартуется либо при помощи StartMainClient(), либо StartMainServer()
*   в зависимости от выбранного \ref appmode_anchor "режима работы".
*/

class PApp : public PSingleton<PApp>
{
    friend class PSingleton<PApp>;

public:
    //! Запуск приложения в режиме клиента
    void StartMainClient();
    //! Запуск приложения в режиме сервера
    void StartMainServer();
    //! Установка сокета назначения
    /*! Устанавливает сокет, к которому будет происходить соединение.
    *   В режиме клиента устанавливает сокет UDTPP-сервера. В режиме сервера устанавливает сокет пункта назначения,
    *   конечной точки доставки TCP соединения.
    */
    bool SetConnectSocket(buffer_t str_socket) { return ParseSocketString(str_socket, mConnectSocket); }
    //! Установка сокета прослушивания
    /*! В режиме клиента устанавливает сокет, на который будут приниматься входящие соединения от прикладных клиентов.
    *   В режиме сервера устанавливает сокет, на который будет приниматься соединение от UDTPP в режиме клиента.
    */
    bool SetBindSocket(buffer_t str_socket) { return ParseSocketString(str_socket, mBindSocket); }
    //! Установка внешнего IP в режиме Rendezvous для UDTPP-клиента
    /*! Устанавливает внешний IP-адрес UDTPP-клиента в режиме Rendezvous для случая, когда UDTPP-клиент находится за
    *   NAT-боксом. Если не вызывать эту функцию, то адрес будет определяться автоматически UDTPP-сервером (см. PMainServer::ServiceThread()).
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
    //! Установка параметров вывода отладочной информации
    /*! Устанавливает фильтр на вывод отладочной информации
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
