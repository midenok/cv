/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

#include "udttransp.h"
#include "app.h"
#include "udtserv.h"
#include "udtmuxer.h"
#include "rendezvous.h"

#ifndef WIN32
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#else
#include <winsock2.h>
#include <Ws2tcpip.h>
#endif

using namespace std;

#define RSIZE 100000

PUdtTransport::PUdtTransport(sockaddr_t & peer_addr, PUdtServer * serv) : mServer(serv), mNewConnection(NULL)
{
    memcpy (&mPeerAddr, &peer_addr, sockaddr_size);
    mutex_init(mReconnect);
}

PUdtTransport::~PUdtTransport()
{
    debug_entering(PUdtTransport::~PUdtTransport);
    mutex_destroy(mReconnect);
    if (mConnected) {
        debug(DBG_GENERAL, 4, "UDT closing socket");
        UDT::close(mPeer);
    }
    return_none();
}

void PUdtTransport::InitMutex()
{}

void PUdtTransport::Connect(bool reconnect)
{
    debug_entering(PUdtTransport::Connect);

    mPeer = UDT::socket(AF_INET, SOCK_STREAM, 0);
    PUdtServer::SetSocketOptions(mPeer);

    sockaddr_t udt_peer_addr;
    memcpy (&udt_peer_addr, &mPeerAddr, sockaddr_size);

    if (gRendezvous) {
        if (!reconnect) {
            SOCKET peer = socket(PF_INET, SOCK_STREAM, 0);
            if (-1 == connect(peer, &mPeerAddr, sockaddr_size)) {
                error (1, serror(serrno));
                exit (1);
            }

            sockaddr_in_t udt_bind_addr;
            // shitdows needs to initialize this! what the fuck!
            socklen_t socklen = sockaddr_size;
            // socket can be closed by remote peer, and its address will be invalidated
            // we need to get this address it before negotiation took place.
            // shitdows loses socket name after its disconnection and linux does not.
            getsockname (peer, (sockaddr_t *)&udt_bind_addr, &socklen);

            aware_transfer (send, peer, &gMagicNumber, sizeof(uint), 0);
            uint cmd;
            if (gApp.mRendezvousIP) {
                cmd = RZV_CLIENT_IP;
                aware_transfer (send, peer, &cmd, sizeof(uint), 0);
                aware_transfer (send, peer, &gApp.mRendezvousIP, sizeof(in_addr_t), 0);
            } else {
                cmd = RZV_GO_ON;
                aware_transfer (send, peer, &cmd, sizeof(uint), 0);
            }

            port_t peer_port;
            aware_transfer (recv, peer, &peer_port, sizeof(peer_port), 0);

            sclose (peer);
            udt_bind_addr.sin_port = htons (peer_port);
            memcpy (&mBindAddr, &udt_bind_addr, sockaddr_size);
            ((sockaddr_in_t &)mPeerAddr).sin_port = mBindAddr.sin_port;
        } // if (!reconnect)

        debug (DBG_GENERAL, 4, string("binding to ") + inet_ntoa(mBindAddr.sin_addr) + ":" + ntohs(mBindAddr.sin_port));

        if (UDT::ERROR == UDT::bind(mPeer, (sockaddr_t *) &mBindAddr, sockaddr_size)) {
            int error = UDT::getlasterror().getErrorCode();
            error(1, string("bind: ") + error + ": " + UDT::getlasterror().getErrorMessage());
            exit (1);
        }
    }

#ifdef WIN32
    UDT::setsockopt(mPeer, 0, UDT_MSS, new int(1052), sizeof(int));
#endif

    debug(DBG_TRANSFER, 4, string("UDT connecting to ") + inet_ntoa(((sockaddr_in_t &)mPeerAddr).sin_addr) + ":" + ntohs(((sockaddr_in_t &)mPeerAddr).sin_port));

    if (UDT::ERROR == UDT::connect(mPeer, &mPeerAddr, sockaddr_size)) {
        error(1, UDT::getlasterror().getErrorMessage());
        exit (1);
    }

    SetSocketOptions();
    mConnected = true;
    return_none();
}

void PUdtTransport::SetSocketOptions()
{
    debug_entering(PUdtTransport::SetSocketOptions);
    bool nonblock = false;
    UDT::setsockopt(mPeer, 0, UDT_RCVSYN, (char *)&nonblock, sizeof(bool));
    UDT::setsockopt(mPeer, 0, UDT_SNDSYN, (char *)&nonblock, sizeof(bool));
    return_none();
}

bool PUdtTransport::Send(buffer_t buf, uint size)
{
    debug_entering(PUdtTransport::Send);
#ifdef DEBUG
    if (!size) {
        error(1, "Bad send request with 0 size!");
        exit(1);
    }
#endif // DEBUG
    lock(mReconnect);
    unlock(mReconnect);
    if (mNewConnection)
        return_bool(mNewConnection->Send(buf, size));
    if (! mConnected) {
        if (mMode == MODE_SERVER) {
            debug (DBG_GENERAL, 2, "UDT dropping because not connected");
            return_bool(false);
        }

        Connect();
    }

    // CHECKIT: using CC method (grep by these words)

    debug (DBG_TRANSFER, 5, "UDT sending data");

    int n;

    while (true) {
        // now send does all or nothing TODO: make partial send for future versions
        n = UDT::send(mPeer, buf, size, 0);
        if (UDT::ERROR == n) {
            int error = UDT::getlasterror().getErrorCode();
            if (error == 6001) { // insufficient buffer space
                debug(DBG_TRANSFER, 4, "Warning: insufficient buffer space!");
                msleep(50);
                continue;
            }
            // shitdows constantly loses connection
            // TODO: when broken on send on server side
            else if (error == 2001 && (gRendezvous || mMode & MODE_CLIENT)) {
                debug (DBG_ALL, 2, "Warning! (2001) UDT connection was broken before send is completed. Reconnecting...");
                mConnected = false;
                UDT::close(mPeer);
                Connect(true);
                continue;
            }

            error(1, UDT::getlasterror().getErrorMessage());
            return_bool(false);
        }
        if (n == 0) {
            debug(DBG_ALL, 2, string ("Warning: send returned 0"));
            msleep(LOOP_DELAY);
            continue;
        }
        break;
    }
    debug (DBG_TRANSFER, 5, "UDT sending successful");
    return_bool(true);
}

/***************/
/* W: A-V=>W-B */
/* V: control  */
/***************/

void PUdtTransport::ServiceLoop(UDTSOCKET peer)
{
    debug_entering(PUdtTransport::ServiceLoop);
    LoopEntered(LOOP_SERVICE);
    // old style:
    // mTransport->StartClient();
    mMode = MODE_SERVER;
    mPeer = peer;
    mConnected = true; // protects mPeer
    // mTransport->mPeer = NULL; // TODO: vvsg
    SetSocketOptions();
    RecieveLoop();
    UDT::close(mPeer);
    if (!mShutdown) {
        debug(DBG_THREADS, 4, "shutting down client");
        mTransport->ShutdownThread();
    }
    LoopExited(LOOP_SERVICE);
    return_none();
}

void PUdtTransport::RecieveLoop()
{
    debug_entering(PUdtTransport::RecieveLoop);
    char buf[RSIZE];
    int n;

    while (!mShutdown) {
        n = UDT::recv(mPeer, buf, RSIZE, 0);
        // TODO: bad situation when PUdtTransoprt::Send locks(mReconnect) first
        if (n == UDT::ERROR) {
            lock(mReconnect);
            int error = UDT::getlasterror().getErrorCode();
            if (error == 6002) {
                unlock(mReconnect);
                msleep(LOOP_DELAY);
                continue;
            }
            // shitdows constantly loses connection
            else if (error == 2001) {
                mConnected = false;
                UDT::close(mPeer);
                if (gRendezvous || mMode & MODE_CLIENT) {
                    debug (DBG_ALL, 2, "Warning! (2001) UDT connection was broken before recv is completed. Reconnecting...");
                    Connect(true);
                    unlock(mReconnect);
                    continue;
                }
                // mMode & MODE_SERVER
                debug(DBG_GENERAL, 3, "UDT waiting for reconnection");
                mNewConnection = mServer->Reconnect();
                // before GiveTransport transports call this->Send()
                GiveTransport(mNewConnection);
                // after GiveTransport transports call new_connection->Send()
                // wait to ensure all this->Send() are done
                msleep(50);
                // TODO: check that connection has come from the same host
            } else {
                // TODO: very long delay detecting broken connection
                debug (DBG_ALL, 3, string("UDT connection error: (") + error + ") " + UDT::getlasterror().getErrorMessage());
            }
            unlock(mReconnect);
            break;
        }
        if (n > 0) {
            debug (DBG_TRANSFER, 5, "UDT received some data");
            if (! Reply(buf, n))
                break;
        } else {
            debug (DBG_TRANSFER, 4, "UDT connection shutdown");
            break;
        }
        msleep(LOOP_DELAY);
    }
    return_none();
}

#if 0
void PUdtTransport::ShutdownThread(PUdtTransport * t)
{
    debug_entering2(t, PUdtTransport::ShutdownThread);
    debug2(t, DBG_THREADS, 4, "UDT thread shutdown");
    if (t->mShutdown) {
        debug2(t, DBG_THREADS, 3, "Shutting down already");
        return_none2(t);
    }
    t->mShutdown = true;
    lock(t->mThreadRunning);
    unlock(t->mThreadRunning);
    return_none2(t);
}
#endif

void PUdtTransport::StartClient()
{
    debug_entering(PUdtTransport::StartClient);
    mMode = MODE_CLIENT;

    thread_run(PUdtTransport::ClientWrapper);
    return_none();
}

void * PUdtTransport::ClientWrapper(PUdtTransport * t)
{
    debug_entering2(NULL, PUdtTransport::ClientWrapper);
    debug2(NULL, DBG_THREADS, 3, "UDT thread started");
    t->ClientThread();
    debug2(NULL, DBG_THREADS, 3, "UDT thread exited");
    t->mShutdown = false;
    return_value2(NULL, void *, NULL);
}

/***************/
/* V: A-V<=W-B */
/***************/

void PUdtTransport::ClientThread()
{
    debug_entering(PUdtTransport::ClientThread);
    LoopEntered(LOOP_CLIENT);
    while (!mConnected) {
        if (mShutdown) {
            LoopExited(LOOP_CLIENT);
            return_none();
        }
        msleep(20);
    }

    RecieveLoop();

    UDT::close (mPeer);
    if (!mShutdown) {
        debug(DBG_THREADS, 4, "shutting down service");
        mTransport->ShutdownThread();
    }
    LoopExited(LOOP_CLIENT);
    return_none();
}
