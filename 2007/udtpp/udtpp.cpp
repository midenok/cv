/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

/*! \mainpage UDTPP Муксер-демуксер TCP каналов через UDT
*   \section sec_app О программе
*   \anchor appmode_anchor Приложение UDTPP работает в одном из двух режимов: клиента и сервера.
*   В режиме \b клиента программа ожидает входящие TCP соединения, устанавливает UDT канал связи с сервером и
*   передаёт данные по этому каналу серверу. Сервер -- это второй режим программы.\n \n
*   В режиме \b сервера программа принимает данные от клиента по UDT каналу,
*   устанавливает TCP соединения с пунктом назначения и передаёт по ним соответствующие данные.\n \n
*   На стороне клиента все TCP каналы мультиплексируются в один UDT канал, а на стороне сервера они демультиплексируются.
*   Таким образом, каждому входящему TCP соединению на клиенте соответствует исходящее TCP соединение на сервере. \n \n
*   UDTPP может работать в режиме \b Rendezvous библиотеки UDT для прохождения некоторых файрволлов NAT-боксов
*   (http://udt.sourceforge.net/doc/doc/t-firewall.htm). \n \n
*   UDT соединение устанавливается только по необходимости, т.е. после первого TCP-коннекта. При разрыве UDT соединения по нештатной причине
*   срабатывает механизм прозрачного восстановления. \n \n
*   Приложение совместимо с платформами Linux и Windows NT.
*   \section sec_use Использование UDTPP
*   На стороне клиента: \n
*   <tt> udtpp -c ip_server:port [-b ip_bind:port] [-r] </tt>\n \n
*   На стороне сервера: \n
*   <tt> udtpp -s ip_proxy:port [-b ip_bind:port] [-r] </tt>\n \n
*   \arg \b ip_server адрес udtpp-сервера
*   \arg \b ip_proxy адрес TCP proxy
*   \arg \b ip_bind адрес прослушиваемого сокета (по умолчанию 0.0.0.0:8888)
*
*   Для справки: \n
*   <tt> udtpp -h </tt>
*   \section sec_udt О протоколе UDT
*   UDT это высо копроизводительный протокол передачи данных, основанный на базе протокола UDP. Протокол спроектирован для
*   приложений, активно использующих глобальные сети передачи данных, таких как вычислительные грид-системы.
*   Протокол UDT задуман так, чтобы лучше справляться с проблемами неэффективности и несправедливости, которые присущи
*   протоколу TCP. UDT поддерживает оба режима работы сокета -- как с установлением соединения, так и без установления.\n \n
*   Домашняя страница UDT: \n
*   http://udt.sourceforge.net/ \n \n
*   Документация по UDT SDK: \n
*   http://udt.sourceforge.net/doc/ \n \n
*   \anchor anchor_udt_download Исходные коды библиотеки UDT: \n
*   http://sourceforge.net/projects/udt \n
*   \section sec_arch Архитектура UDTPP
*   Основную функциональную роль в UDTPP играют \b транспорты. Поскольку в UDTPP происходит преобразование одного протокола в другой,
*   то понятие транспорта в UDTPP несколько отличается от общепринятого. Обычный транспорт соединяет два сокета.
*   В UDTPP транспорт соединяет сокет и другой транспорт. Это означает, что каждый транспорт работает в паре со \b смежным транспортом.
*   Одному транспорту может соответствовать более одного смежного транспорта, если данные пришедшие с сокета демультиплексируются.
*   Сокет, о котором идёт речь будем называть \b внешним сокетом, для того чтобы было понятно, что данные поступают извне и отправляются вовне
*   приложения UDTPP. \n \n
*   Подобная модель с единым интерфейсом стыковки транспортов позволяет легко конструировать сложные приложения по обработке
*   и преобразованию сетевых потоков. \n \n
*   Каждый транспорт (как и само приложение UDTPP) имеет два режима: режим клиента и режим сервера. В режиме клиента транспорт инициирует соединение.
*   В режиме сервера транспорт ожидает соединение. \n \n
*   В UDTPP работают два транспорта: PTcpTransport со стороны внешних TCP приложений и PUdtMuxer со стороны UDT канала.
*   Архитектура обслуживания сокетов многопоточная с блокирующими сокетами. Функционал работы потоков (\b трэдов) заложен в классах транспорта.
*   В будущем его необходимо будет выделить в отдельные классы. И, возможно, усложнить архитектуру по модели thread pool где каждый worker
*   будет обслуживать список сокетов. Это повысит максимальное число одновременных соединений. \n \n
*   Обслуживание входящих соединений выведено в отдельные классы PTcpServer и PUdtServer.
*   Подобная конструкция позволяет использовать несколько соединений в одном приложении. Специфика соединения для приложения UDTPP выведена в классы
*   PMainClient и PMainServer. \n \n
*   На данный момент, архитектура имеет недостатки, значительно осложняющие многопоточное программирование.
*   Решение состоит в сериализации send, recv и запросов на разрыв соединения. \ref known_bugs "Подробнее..."
*   \section sec_compile Компиляция
*   Для компиляции требуется библиотека UDT (URL указан \ref anchor_udt_download "выше").
*   Для корректной работы режима Rendezvous необходима последняя версия библиотеки, скачанная с CVS (http://sourceforge.net/cvs/?group_id=115059).
*   \subsection sec_compile_linux Под Linux
*   - откомпилировать библиотеку UDT3: \n
*   <tt>cd udt3-cvs/src; make</tt> \n
*   - в udtpp/makefile отредактировать SRC_DIR и UDT_DIR
*   - откомпилировать UDTPP: \n
*   <tt>cd udtpp; make</tt> \n
*   \subsection sec_compile_winnt Под Win NT/2k/XP/2k3
*   - откомпилировать библиотеку UDT3: \n
*   В VisualStudio открыть udt3-cvs/win/udt.vcproj \n
*   - откомпилировать POSIX Threads for Win32 (http://sourceware.org/pthreads-win32/): \n
*   <tt>nmake clean VC</tt> \n
*   - откомпилировать UDTPP: \n
*   Открыть udtpp/dotnet/udtpp.vcproj; указать пути до библиотек UDT и PTHREADS. \n
*   - Для запуска udtpp.exe необходимо скопировать udt.dll и pthreadVC2.dll в udtpp/dotnet/Debug \n
*
*   \author Aleksey Midenkov (asm@uezku.kemsu.ru)
*/

/*! \page known_bugs Известные проблемы
*
        <b>На данный момент известны следующие глобальные проблемы:</b>

        1) recv и send вызываются из разных потоков, и это осложняет ситуацию, когда 
        один из них, либо оба этих вызова возвращают ошибку. 

        2) завершение цикла приёма может инициироваться как в потоке цикла, так и из 
        других потоков. Это и тот факт, что выход из цикла влечёт за собой разрушение 
        объекта, который используется одновременно в двух потоках также сильно 
        осложняют ситуацию.

        Эти проблемы решаются следующим образом. recv, send и обработка ситуации 
        завершения соединения должны работать в одном цикле (соответственно в одном 
        потоке). Другие потоки, желающие что-либо отправить сокету, либо закрыть его 
        должны ставить свои задания в очередь, которая будет обрабатываться тем же 
        самым циклом.

        Постановка заданий send и shutdown в очередь всё таки происходит из другого 
        потока. И поэтому должна быть гарантия того, что во время вызова постановки в 
        очередь цикл обработки не завершён (иначе вызов может произойти для уже 
        несуществующего объекта).

        Это решается при помощи класса-регистратора всех многопоточных объектов. В 
        данном случае я предполагаю модель, в которой за каждым многопоточным 
        объектом закреплён свой основной поток, но доступ к объекту должен быть 
        возможен также и из других потоков. Объект-регистратор Registry регистрирует 
        все многопоточные объекты. Каждая запись регистратора о многопоточном объекте 
        Slave содержит также список всех Master-объектов (работающих в других 
        потоках) имеющих доступ к многопоточному объекту. Перед тем, как обратиться к 
        Slave, Master должен справиться у Registry существует ли ещё Slave или уже 
        разрушен. Если Slave разрушается, он оповещает об этом Registry. Registry не 
        удаляет запись о разрушенном Slave до тех пор, пока не ответит всем 
        Master-объектам что этот Slave разрушен. Если же разрушенный Slave сам 
        является мастером для другого многопоточного объекта Slave2, то 
        соответствующая запись Slave2 в Registry должна быть исправлена. Иначе в 
        ситуации, когда Slave2 разрушится, Registry будет ждать запроса от Slave на 
        оповощение. Но Slave уже разрушен, и запись соответствующая Slave2 никогда не 
        удалится. Следует также заметить, что адрес объекта в такой модели не может 
        использоваться, как идентификатор объекта, потому что после разрушения Slave 
        вновь созданный объект может иметь тот же адрес. Поэтому должна вестись 
        уникальная нумерация для всех многопоточных объектов в Registry.

        <b>Кроме того, в текущей версии не реализовано следующее:</b>

        Нет реконнекта, когда разрывается UDT соединение на стороне сервера в момент send.
        См. PUdtTransport::Send (TODO: when broken on send on server side).
*/

#include "app.h"
#include "mainclient.h"

#include <getopt.h>
#include <iostream>

// REMOVEIT:
#include <stdio.h>

using namespace std;

#define DEFAULT_PORT 8888

int setup_args (int & argc, char ** & argv);

int main (int argc, char ** argv)
{
//     PApp & app = (PApp &)PApp::Instance();
//     gApp = &app;

#ifdef WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD( 2, 2);

    err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 ) {
        cerr << "Winsock error\n";
        return 1;
    }

    /* Confirm that the WinSock DLL supports 2.0.*/
    /* Note that if the DLL supports versions greater    */
    /* than 2.0 in addition to 2.0, it will still return */
    /* 2.0 in wVersion since that is the version we      */

    /* requested.                                        */

    if ( LOBYTE( wsaData.wVersion ) != 2 ||
            HIBYTE( wsaData.wVersion ) != 2 ) {
        cerr << "Winsock version mismatch\n";
        WSACleanup( );
        return 1;
    }
#endif // WIN32

    switch (setup_args (argc, argv)) {
    case MODE_SERVER:
        gApp.StartMainServer();
        break;

    case MODE_CLIENT:
        gApp.StartMainClient();
        break;
    }

    return 0;
}

int setup_args (int & argc, char ** & argv)
{
    int mode = MODE_NONE;
    int return_status = 0;
    bool bind_set = false;
    char c = 0;
    char p = 0;

    while (true) {

        p = c;
        c = getopt (argc, argv, ":hr:s:c:b:d:");

        if (c == -1)
            break;

        if (c == ':') {
            switch (optopt) {
            case 'r':
                break;
            default:
                cerr << "Switch '-" << (char)optopt << "' requires an argument!\n\n";
                return_status = 1;
                goto help;
            }
            c = optopt;
        }

        switch (c) {
        case 's':
        case 'c':
            if (mode != MODE_NONE) {
                cerr << "Must have either -s or -c option!\n\n";
                return_status = 1;
                goto help; /* v00d00: do you think i'm a bad guy? no! i'm just free guy! ;) */
            }
            mode = c == 's' ? MODE_SERVER : MODE_CLIENT;
            if (gApp.SetConnectSocket(optarg))
                continue;
            cerr << "Bad socket argument <ip_address>:<port>!\n\n";
            return_status = 1;
            goto help;

        case 'b':
            if (bind_set) {
                cerr << "Only one -b argument allowed!\n\n";
                return_status = 1;
                goto help;
            }
            if (gApp.SetBindSocket(optarg)) {
                bind_set = true;
                continue;
            }
            cerr << "Bad bind socket argument <ip_address>:<port>!\n\n";
            return_status = 1;
            goto help;

        case 'r':
            gRendezvous = true;

            if (!optarg || gApp.SetRendezvousIP(optarg))
                continue;

            cerr << "Bad rendezvous <ip_address> argument!\n\n";
            return_status = 1;
            goto help;

            continue;

        case 'd':
            if (gApp.SetDebugOptions(optarg))
                continue;
            cerr << "Bad debug argument!\n\n";
            return_status = 1;
            goto help;

        help:
        case 'h':
            cout << "Usage: " << argv[0] << " -s|-c <ip_address>:<port> [-b <ip_address>:<port>] [-r [<ip_address>]] [-d <L>a | <L>gntxm]\n";
            cout << "  -s  Run in server mode. All UDT traffic will go to TCP <ip_address>:<port>.\n";
            cout << "  -c  Run in client mode. All TCP traffic will go to UDT <ip_address>:<port>.\n";
            cout << "  -b  Listen on <ip_address>:<port> for incoming UDT (server mode)\n";
            cout << "      or TCP (client mode) connections. Defaults to 0.0.0.0:" << DEFAULT_PORT << ".\n";
            cout << "  -r Use rendezvous connection setup (to punch through firewalls and NAT-boxes)\n";
            cout << "     <ip_address> explicitly sets external IP address (NYI: currently works on client side only)\n";
            cout << "\n  <ip_address> can be IP address or hostname.\n";
            cout << "  <port> is decimal numeric representation of port.\n";
            cout << "\n  -d <L>a | <L>gntxm  ***NYI*** Set debug level and select debug facilities:\n";
            cout << "     <L>  Number 0-9 (0 turns of debugging, 9 is max debugging)\n";
            cout << "      a   Select all facilities (9a is max debug output)\n";
            cout << "\n  You can choose a set of individual facilities by adding a letter:\n";
            cout << "      g   General facility\n";
            cout << "      n   Network traffic\n";
            cout << "      t   Threads\n";
            cout << "      x   Muxer\n";
            cout << "      m   Memory\n";
            cout << "\n  -h  Show this help screen.\n";
            exit (return_status);
        }
    }

    if (mode == MODE_NONE)
        goto help;

    if (!bind_set)
        gApp.SetBindSocket(INADDR_ANY, DEFAULT_PORT);

    return mode;
}
