/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

/*! \mainpage UDTPP ������-�������� TCP ������� ����� UDT
*   \section sec_app � ���������
*   \anchor appmode_anchor ���������� UDTPP �������� � ����� �� ���� �������: ������� � �������.
*   � ������ \b ������� ��������� ������� �������� TCP ����������, ������������� UDT ����� ����� � �������� �
*   �������� ������ �� ����� ������ �������. ������ -- ��� ������ ����� ���������.\n \n
*   � ������ \b ������� ��������� ��������� ������ �� ������� �� UDT ������,
*   ������������� TCP ���������� � ������� ���������� � �������� �� ��� ��������������� ������.\n \n
*   �� ������� ������� ��� TCP ������ ������������������ � ���� UDT �����, � �� ������� ������� ��� ��������������������.
*   ����� �������, ������� ��������� TCP ���������� �� ������� ������������� ��������� TCP ���������� �� �������. \n \n
*   UDTPP ����� �������� � ������ \b Rendezvous ���������� UDT ��� ����������� ��������� ���������� NAT-������
*   (http://udt.sourceforge.net/doc/doc/t-firewall.htm). \n \n
*   UDT ���������� ��������������� ������ �� �������������, �.�. ����� ������� TCP-��������. ��� ������� UDT ���������� �� ��������� �������
*   ����������� �������� ����������� ��������������. \n \n
*   ���������� ���������� � ����������� Linux � Windows NT.
*   \section sec_use ������������� UDTPP
*   �� ������� �������: \n
*   <tt> udtpp -c ip_server:port [-b ip_bind:port] [-r] </tt>\n \n
*   �� ������� �������: \n
*   <tt> udtpp -s ip_proxy:port [-b ip_bind:port] [-r] </tt>\n \n
*   \arg \b ip_server ����� udtpp-�������
*   \arg \b ip_proxy ����� TCP proxy
*   \arg \b ip_bind ����� ��������������� ������ (�� ��������� 0.0.0.0:8888)
*
*   ��� �������: \n
*   <tt> udtpp -h </tt>
*   \section sec_udt � ��������� UDT
*   UDT ��� ���� ������������������ �������� �������� ������, ���������� �� ���� ��������� UDP. �������� ������������� ���
*   ����������, ������� ������������ ���������� ���� �������� ������, ����� ��� �������������� ����-�������.
*   �������� UDT ������� ���, ����� ����� ����������� � ���������� ��������������� � ����������������, ������� �������
*   ��������� TCP. UDT ������������ ��� ������ ������ ������ -- ��� � ������������� ����������, ��� � ��� ������������.\n \n
*   �������� �������� UDT: \n
*   http://udt.sourceforge.net/ \n \n
*   ������������ �� UDT SDK: \n
*   http://udt.sourceforge.net/doc/ \n \n
*   \anchor anchor_udt_download �������� ���� ���������� UDT: \n
*   http://sourceforge.net/projects/udt \n
*   \section sec_arch ����������� UDTPP
*   �������� �������������� ���� � UDTPP ������ \b ����������. ��������� � UDTPP ���������� �������������� ������ ��������� � ������,
*   �� ������� ���������� � UDTPP ��������� ���������� �� �������������. ������� ��������� ��������� ��� ������.
*   � UDTPP ��������� ��������� ����� � ������ ���������. ��� ��������, ��� ������ ��������� �������� � ���� �� \b ������� �����������.
*   ������ ���������� ����� ��������������� ����� ������ �������� ����������, ���� ������ ��������� � ������ ��������������������.
*   �����, � ������� �ģ� ���� ����� �������� \b ������� �������, ��� ���� ����� ���� �������, ��� ������ ��������� ����� � ������������ �����
*   ���������� UDTPP. \n \n
*   �������� ������ � ������ ����������� �������� ����������� ��������� ����� �������������� ������� ���������� �� ���������
*   � �������������� ������� �������. \n \n
*   ������ ��������� (��� � ���� ���������� UDTPP) ����� ��� ������: ����� ������� � ����� �������. � ������ ������� ��������� ���������� ����������.
*   � ������ ������� ��������� ������� ����������. \n \n
*   � UDTPP �������� ��� ����������: PTcpTransport �� ������� ������� TCP ���������� � PUdtMuxer �� ������� UDT ������.
*   ����������� ������������ ������� ������������� � ������������ ��������. ���������� ������ ������� (\b ������) ������� � ������� ����������.
*   � ������� ��� ���������� ����� �������� � ��������� ������. �, ��������, ��������� ����������� �� ������ thread pool ��� ������ worker
*   ����� ����������� ������ �������. ��� ������� ������������ ����� ������������� ����������. \n \n
*   ������������ �������� ���������� �������� � ��������� ������ PTcpServer � PUdtServer.
*   �������� ����������� ��������� ������������ ��������� ���������� � ����� ����������. ��������� ���������� ��� ���������� UDTPP �������� � ������
*   PMainClient � PMainServer. \n \n
*   �� ������ ������, ����������� ����� ����������, ����������� ����������� ������������� ����������������.
*   ������� ������� � ������������ send, recv � �������� �� ������ ����������. \ref known_bugs "���������..."
*   \section sec_compile ����������
*   ��� ���������� ��������� ���������� UDT (URL ������ \ref anchor_udt_download "����").
*   ��� ���������� ������ ������ Rendezvous ���������� ��������� ������ ����������, ��������� � CVS (http://sourceforge.net/cvs/?group_id=115059).
*   \subsection sec_compile_linux ��� Linux
*   - ��������������� ���������� UDT3: \n
*   <tt>cd udt3-cvs/src; make</tt> \n
*   - � udtpp/makefile ��������������� SRC_DIR � UDT_DIR
*   - ��������������� UDTPP: \n
*   <tt>cd udtpp; make</tt> \n
*   \subsection sec_compile_winnt ��� Win NT/2k/XP/2k3
*   - ��������������� ���������� UDT3: \n
*   � VisualStudio ������� udt3-cvs/win/udt.vcproj \n
*   - ��������������� POSIX Threads for Win32 (http://sourceware.org/pthreads-win32/): \n
*   <tt>nmake clean VC</tt> \n
*   - ��������������� UDTPP: \n
*   ������� udtpp/dotnet/udtpp.vcproj; ������� ���� �� ��������� UDT � PTHREADS. \n
*   - ��� ������� udtpp.exe ���������� ����������� udt.dll � pthreadVC2.dll � udtpp/dotnet/Debug \n
*
*   \author Aleksey Midenkov (asm@uezku.kemsu.ru)
*/

/*! \page known_bugs ��������� ��������
*
        <b>�� ������ ������ �������� ��������� ���������� ��������:</b>

        1) recv � send ���������� �� ������ �������, � ��� ��������� ��������, ����� 
        ���� �� ���, ���� ��� ���� ������ ���������� ������. 

        2) ���������� ����� ��ɣ�� ����� �������������� ��� � ������ �����, ��� � �� 
        ������ �������. ��� � ��� ����, ��� ����� �� ����� ���ޣ� �� ����� ���������� 
        �������, ������� ������������ ������������ � ���� ������� ����� ������ 
        ��������� ��������.

        ��� �������� �������� ��������� �������. recv, send � ��������� �������� 
        ���������� ���������� ������ �������� � ����� ����� (�������������� � ����� 
        ������). ������ ������, �������� ���-���� ��������� ������, ���� ������� ��� 
        ������ ������� ���� ������� � �������, ������� ����� �������������� ��� �� 
        ����� ������.

        ���������� ������� send � shutdown � ������� �ӣ ���� ���������� �� ������� 
        ������. � ������� ������ ���� �������� ����, ��� �� ����� ������ ���������� � 
        ������� ���� ��������� �� �����ۣ� (����� ����� ����� ��������� ��� ��� 
        ��������������� �������).

        ��� �������� ��� ������ ������-������������ ���� ������������� ��������. � 
        ������ ������ � ����������� ������, � ������� �� ������ ������������� 
        �������� ������̣� ���� �������� �����, �� ������ � ������� ������ ���� 
        �������� ����� � �� ������ �������. ������-����������� Registry ������������ 
        ��� ������������� �������. ������ ������ ������������ � ������������� ������� 
        Slave �������� ����� ������ ���� Master-�������� (���������� � ������ 
        �������) ������� ������ � �������������� �������. ����� ���, ��� ���������� � 
        Slave, Master ������ ���������� � Registry ���������� �� �ݣ Slave ��� ��� 
        ��������. ���� Slave �����������, �� ��������� �� ���� Registry. Registry �� 
        ������� ������ � ����������� Slave �� ��� ���, ���� �� ������� ���� 
        Master-�������� ��� ���� Slave ��������. ���� �� ����������� Slave ��� 
        �������� �������� ��� ������� �������������� ������� Slave2, �� 
        ��������������� ������ Slave2 � Registry ������ ���� ����������. ����� � 
        ��������, ����� Slave2 ����������, Registry ����� ����� ������� �� Slave �� 
        ����������. �� Slave ��� ��������, � ������ ��������������� Slave2 ������� �� 
        ��������. ������� ����� ��������, ��� ����� ������� � ����� ������ �� ����� 
        ��������������, ��� ������������� �������, ������ ��� ����� ���������� Slave 
        ����� ��������� ������ ����� ����� ��� �� �����. ������� ������ ������� 
        ���������� ��������� ��� ���� ������������� �������� � Registry.

        <b>����� ����, � ������� ������ �� ����������� ���������:</b>

        ��� ����������, ����� ����������� UDT ���������� �� ������� ������� � ������ send.
        ��. PUdtTransport::Send (TODO: when broken on send on server side).
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
