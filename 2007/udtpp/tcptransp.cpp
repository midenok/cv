/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

#include "tcptransp.h"
#include "app.h"

#include <fcntl.h>

// static mutex_t mMutex;

using namespace std;

PTcpTransport::PTcpTransport() : mPeer(0), mBroken(false)
{
    debug_entering (CONSTRUCTOR);
    mutex_init (mShutting);
}

PTcpTransport::~PTcpTransport()
{
    debug_entering (DESTRUCTOR);
    //    if (mConnected)
    //        sclose(mPeer);
    mutex_destroy (mShutting);
}

void PTcpTransport::InitMutexes()
{
    //    mutex_init(mMutex);
}

// the constant TCP_BUFSIZE_READ is the maximum size of the standard input
// buffer of TcpSocket
#define RSIZE TCP_BUFSIZE_READ

/***************/
/* A: A=>V-W-B */
/***************/

void PTcpTransport::ServiceLoop(SOCKET peer)
{
    debug_entering (PTcpTransport::ServiceLoop);
    mThread = pthread_self();
    mMode = MODE_SERVER;
    mPeer = peer;
    mConnected = true; // protects mPeer

    RecieveLoop();

    debug(DBG_THREADS, 4, "shutting down channel");
    mTransport->RequestShutdown(mChannel);

    return_none ();
}

void PTcpTransport::RecieveLoop()
{
    debug_entering (PTcpTransport::RecieveLoop);
    char buf[RSIZE];
    int n;

#ifdef DEBUG
    sockaddr_in_t addr_in;
    socklen_t socklen = sockaddr_size;
    if (mMode == MODE_SERVER)
        getpeername (mPeer, (sockaddr_t *)&addr_in, &socklen);
    else
        getsockname (mPeer, (sockaddr_t *)&addr_in, &socklen);
    debug (DBG_GENERAL, 3, string("Socket address: ") + inet_ntoa(addr_in.sin_addr) + ":" + ntohs(addr_in.sin_port));
#endif // DEBUG

    while (true) {
        debug (DBG_TRANSFER, 5, "TCP waiting for some data");
        n = recv(mPeer, buf, RSIZE, 0);
        lock_debug (mShutting);
        if (mShutdown) {
            debug(DBG_THREADS, 5, "TCP thread waiting to be cancelled");
            unlock_debug (mShutting);
            while (true) {
                pthread_testcancel();
                msleep (5);
            }
        }
        if (n < 0) {
            int error = serrno;
            error (2, serror(error));
            break;
        }

        if (n > 0) {
            debug (DBG_TRANSFER, 5, "TCP received some data");
            if (! mTransport->Send(buf, n, mChannel)) {
                error (2, "Warning! Failed to send data!");
                break;
            }
        } else {
            debug (DBG_TRANSFER, 3, "TCP peer shut down connection");
            break;
        }

        unlock_debug (mShutting);
    }

    mShutdown = true;
    mBroken = true;

    debug (DBG_GENERAL, 3, string("Close socket: ") + inet_ntoa(addr_in.sin_addr) + ":" + ntohs(addr_in.sin_port));

    sclose (mPeer);
    unlock_debug (mShutting);

    // wait for some time for CheckShutting arrival
    msleep (10);

    return_none();
}

// Muxer thread
bool PTcpTransport::CheckShutting()
{
    debug_entering (PTcpTransport::CheckShutting);
    lock_debug (mShutting);
    if (mShutdown) {
        unlock_debug (mShutting);
        return_bool (true);
    }
    mShutdown = true;
    unlock_debug (mShutting);
    return_bool (false);
}

void PTcpTransport::ShutdownThread()
{
    debug_entering(PTcpTransport::ShutdownThread);
    debug(DBG_GENERAL, 4, "TCP thread shutdown");
    if (mConnected)
        sclose (mPeer);
    // Cancelling thread deletes automatic variable tcp in PMainClient::ServiceThread
    // therefore sclose (mPeer) must be before pthread_cancel (mThread)!
    pthread_cancel (mThread);
    return_none ();
}

/***************/
/* B: A-V-W=>B */
/***************/

void PTcpTransport::Connect()
{
    debug_entering(PTcpTransport::Connect);
    mPeer = socket(PF_INET, SOCK_STREAM, 0);
    debug (DBG_TRANSFER, 3, "TCP connecting");
    if (-1 == connect(mPeer, &gApp.mConnectSocket.addr, sockaddr_size)) {
        error (1, serror(serrno));
        exit (1);
    }

    mConnected = true;
    return_none();
}

void PTcpTransport::StartClient()
{
    debug_entering(PTcpTransport::StartClient);
    mMode = MODE_CLIENT;
    // very very strange glitch: mShutdown have not been initializing
    mShutdown = false;
    mConnected = false; //TODO: initialize in constructor

    thread_run(PTcpTransport::ClientWrapper);
    return_none();
}

void * PTcpTransport::ClientWrapper(PTcpTransport * t)
{
    debug_entering2(NULL, PTcpTransport::ClientWrapper);
    debug2(NULL, DBG_THREADS, 3, "TCP thread started");
    t->ClientThread();
    debug2(NULL, DBG_THREADS, 3, "TCP thread exited");
    return_value2(NULL, void *, NULL);
}

/***************/
/* B: A-V-W<=B */
/***************/

void PTcpTransport::ClientThread()
{
    debug_entering(PTcpTransport::ClientThread);

    mThread = pthread_self();

    // should connect on Forward
    // TODO: in future make ClientThread started on connection
    while (!mConnected) {
        pthread_testcancel();
        msleep(5);
    }

    RecieveLoop();

    debug(DBG_THREADS, 4, "shutting down service");
    mTransport->RequestShutdown(mChannel);

    return_none();
}

bool PTcpTransport::Send(buffer_t buf, uint size)
{
    debug_entering(PTcpTransport::Send);
    /* TODO: TCP_NODELAY should have some effect, but only if there's a significant
    delay in the sender between sending the packets. */

    // TODO: mBroken does not guard against SIGPIPE fully

    if (mBroken) {
        debug (DBG_GENERAL, 2, "TCP dropping because disconnected");
        return_bool (false);
    }

    if (! mConnected) {
        if (mMode == MODE_SERVER) {
            debug (DBG_GENERAL, 2, "TCP dropping because not connected");
            return_bool (false);
        }
        Connect();
    }

    debug (DBG_TRANSFER, 5, "TCP sending data");

    int n;

    while (true) {
        n = send(mPeer, buf, size, 0);
        if (-1 == n) {
            int error = serrno;
            if (error == ENOTCONN || error == EPIPE) {
                debug (DBG_TRANSFER, 3, "TCP connection was broken");
                return_bool (false);
            }
            if (error = EWOULDBLOCK) {
                debug(DBG_TRANSFER, 4, "TCP sending deferred");
                msleep(LOOP_DELAY);
                continue;
            }
            // TODO: ENOBUFS situation
            error(2, string("PTcpTransport::RecieveLoop   Error: ") + serror(error));
            return_bool (false);
        }
        size -= n;
        if (size == 0)
            break;
        buf += n;
        msleep(LOOP_DELAY);
    }

    debug (DBG_TRANSFER, 5, "TCP sending successful");
    return_bool (true);
}
