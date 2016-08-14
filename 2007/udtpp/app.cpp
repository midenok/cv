/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

#include "app.h"
#include "mainclient.h"
#include "mainserver.h"

#include <string>
#include <map>

#ifndef WIN32
#include <netdb.h>
#endif

using namespace std;

#include <string.h>
#include <time.h>
#include <iostream>
#include "Parse.h"

// TODO: these global variables should be inside PApp

int gLogLevel;
int gLogFacilities;
bool gRendezvous;
uint gMagicNumber;

PApp & gApp = PApp::Instance();
mutex_t atomic_debug_print;

string operator+ (string a, int b)
{
    char s[12];
    snprintf(s, 12, "%d", b);
    return a + s;
}

void debug_print (int level, const void * obj, buffer_t func_name, buffer_t file, int line, std::string msg)
{
    debug_print(level, obj, func_name, file, line, msg.c_str());
}

#ifdef WIN32
bool operator < (const thread_t t1, const thread_t t2)
{
    return (long &)t1 < (long &)t2;
}
#endif

void debug_print (int level, const void * obj, buffer_t func_name, buffer_t file, int line, buffer_t msg)
{
    static int num_threads = 0;
    static map<thread_t, char> threads;
    lock(atomic_debug_print);
    thread_t t = thread_self();
    if (threads.find(t) == threads.end())
        threads[t] = 'A' + num_threads++;

    static int num_objs = 0;
    static map<const void *, int> objs;
    if (objs.find(obj) == objs.end())
        objs[obj] = ++num_objs;

    std::cout << level << "  " <<
    threads[t] << " " <<
    objs[obj] << "->" <<
    func_name << '(' << file << ":" << line << "): " <<
    msg << std::endl;
    unlock(atomic_debug_print);
}

void msleep (int msecs)
{
#ifdef _WIN32
    Sleep(msecs);
#else
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = msecs * 1000000;
    nanosleep(&t, NULL);
#endif
}

// parse str_socket for ex. "127.0.0.1:9999"
// and set internal data members
// v00d00: sorry for C Library calls, don't have
// time to learn how to do it in C++...
// TODO: rework this function in C++

#define MIN_PORT_RANGE 1
#define MAX_PORT_RANGE 65535

bool isipv4(const std::string& str)
{
    for (size_t i = 0; i < str.size(); i++)
        if (!isdigit(str[i]) && str[i] != '.')
            return false;
    return true;
}

bool u2ip(const std::string& str, in_addr_t& l)
{
    if (isipv4(str)) {
        Parse pa((char *)str.c_str(), ".");
        union {
            struct {
                unsigned char b1;
                unsigned char b2;
                unsigned char b3;
                unsigned char b4;
            }
            a;
            in_addr_t l;
        } u;
        u.a.b1 = static_cast<unsigned char>(pa.getvalue());
        u.a.b2 = static_cast<unsigned char>(pa.getvalue());
        u.a.b3 = static_cast<unsigned char>(pa.getvalue());
        u.a.b4 = static_cast<unsigned char>(pa.getvalue());
        l = u.l;
        return true;
    } else {
        struct hostent *he = gethostbyname( str.c_str() );
        if (!he) {
            return false;
        }
        memcpy(&l, he -> h_addr, 4);
        return true;
    }
    return false;
}

bool ParseSocketString(buffer_t str_socket, PSocketInfo & socket)
{
    char * port = strchr (str_socket, ':');
    if (!port)
        return false;
    int len = (int)(port++ - str_socket);
    if (len > 15)
        return false;
    memcpy (socket.str_ip, str_socket, len);
    socket.str_ip[len] = 0;
    sockaddr_in_t * addr = (sockaddr_in_t *) &socket.addr;
    addr->sin_family = AF_INET;
    if (! u2ip (socket.str_ip, addr->sin_addr.s_addr))
        return false;
    register uint p;
    if (0 == (p = atoi(port)))
        return false;
    if ( p < MIN_PORT_RANGE || p > MAX_PORT_RANGE)
        return false;
    addr->sin_port = htons(p);
    strncpy (socket.str_port, port, 5);
    return true;
}

PApp::PApp() : mRendezvousIP(0)
{
#ifdef DEBUG
    mutex_init (atomic_debug_print);
#endif
    gLogLevel = 10;
    gLogFacilities = DBG_ALL;
    gRendezvous = false;
    gMagicNumber = 83760157;
    // gLogFacilities = DBG_TRANSFER + DBG_GENERAL;
    //gLogFacilities = DBG_THREADS;
}

PApp::~PApp()
{
#ifdef DEBUG
    mutex_destroy (atomic_debug_print);
#endif
}

void PApp::StartMainClient()
{
    // TODO: should be more proper place to init mutexes
    PTcpTransport::InitMutexes();
    PUdtTransport::InitMutex();
    PMainClient & c = PMainClient::Instance();
    c.StartService();
}

void PApp::StartMainServer()
{
    // TODO: should be more proper place to init mutexes
    PTcpTransport::InitMutexes();
    PUdtTransport::InitMutex();
    PMainServer & s = PMainServer::Instance();
    s.StartService();
}

bool PApp::SetRendezvousIP(buffer_t str_ip)
{
    in_addr_t ip;
    if (! u2ip (str_ip, ip))
        return false;
    mRendezvousIP = ip;
    return true;
}


bool PApp::SetDebugOptions (buffer_t dbgopt)
{
    buffer_t c = dbgopt;
    if (isdigit(*c))
        gLogLevel = *c++ - '0';

    for (; *c; ++c) {
        switch (*c) {
        case 'a':
            gLogFacilities = DBG_ALL;
            return true;
        }
    }
    return false;
}
