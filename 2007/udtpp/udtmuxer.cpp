/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

#include "udtmuxer.h"
#include "udttransp.h"
#include "tcptransp.h"
#include "util.h"

using namespace std;

int PUdtMuxer::mMtu = 1500;
int PUdtMuxer::mMinChanSize = 200;
int PSendBuffer::mBlockSize = 1500;

// template<>
// PTransportStaticData PTransport<PUdtMuxer>::mStatic;

PSendBuffer::PSendBuffer() : mBufferCapacity(0), mHeader(NULL), mBuffer(NULL), mBufferSize(0)
{}

PSendBuffer::~PSendBuffer()
{
    if (mBuffer)
        free(mBuffer);
}

void PSendBuffer::SetHeader(PSendHeader * h)
{
    debug_entering(PSendBuffer::SetHeader);
#ifdef DEBUG
    if (mBufferCapacity - int(mHeader - mBuffer) - 1 < sizeof(PSendHeader)) {
        error(1, "PSendBuffer::SetHeader   First do ReserveHeader()!");
        exit(1);
    }
#endif // DEBUG
    memcpy(mHeader, h, sizeof(PSendHeader));
    return_none();
}

void PSendBuffer::ReserveHeader()
{
    ExpandCapacity(sizeof(PSendHeader));
    mHeader = mBuffer + mBufferSize;
    mBufferSize += sizeof(PSendHeader);
}

void PSendBuffer::ExpandCapacity(int size)
{
    debug_entering(PSendBuffer::ExpandCapacity);
    int required_capacity = mBufferSize + size;
    if (required_capacity > mBufferCapacity) {
        debug(DBG_MEMORY, 5, string("need ") + required_capacity + " have " + mBufferCapacity);
        mBufferCapacity += ((required_capacity - mBufferCapacity) / mBlockSize + 1) * mBlockSize;
        debug(DBG_MEMORY, 5, string("expanding to ") + mBufferCapacity);
        mBuffer = (char *)realloc (mBuffer, mBufferCapacity);
        if (!mBuffer) {
            error(1, "PSendBuffer::ExpandBy   Couldn't reallocate memory!");
            exit(1);
        }
    }
    return_none();
}

void PSendBuffer::Copy(buffer_t buf, uint size)
{
    ExpandCapacity(size);
    memcpy(mBuffer + mBufferSize, buf, size);
    mBufferSize += size;
}

bool PSendBuffer::Send(PUdtMuxer * transport)
{
    debug_entering(PSendBuffer::Send);
    bool res = transport->PUdtTransport::Send(mBuffer, mBufferSize);
    mBufferSize = 0;
    unlock(transport->atomic_send);
    return_bool (res);
}

void PUdtMuxer::Init(sockaddr_t & peer_addr)
{
    memset(&mReplyHeader, 0, sizeof(PSendHeader));
    memset(&mSendHeader, 0, sizeof(PSendHeader));
    // TODO: guard aganst multple PUdtMuxers (what is unneccessary now)
    mutex_init(atomic_send);
    mutex_init(atomic_reply);
    mutex_init(atomic_transports);
    mutex_init(atomic_request);
    mutex_init(atomic_shutdown);
    mTransport = this;
}

PUdtMuxer::PUdtMuxer(sockaddr_t & peer_addr) : PUdtTransport(peer_addr, (PUdtServer *)NULL), mPrevChannel(-1), mPartialHeader(NULL), mTransportList(NULL)
{
    Init(peer_addr);
}

PUdtMuxer::PUdtMuxer(sockaddr_t & peer_addr, PUdtServer * serv) : PUdtTransport(peer_addr, serv), mPrevChannel(-1), mPartialHeader(NULL), mTransportList(NULL)
{
    Init(peer_addr);
}


PUdtMuxer::~PUdtMuxer()
{
    mutex_destroy(atomic_send);
    mutex_destroy(atomic_reply);
    mutex_destroy(atomic_transports);
    if (mTransportList)
        delete mTransportList;
}

/*! Выбирает смежный транспорт (PTcpTransport), соответствующий номеру канала. Если канал под обозначенным номером
*   не существует, по необходимости создаёт его вместе с новым смежным транспортом.
*/

PTcpTransport * PUdtMuxer::SelectChannel(int chan)
{
    debug_entering(PUdtMuxer::SelectChannel);
    lock_debug (atomic_transports);
    if (!mTransportList)
        mTransportList = new transport_list_t;
    if (mTransportList->find(chan) == mTransportList->end()) {
        unlock_debug (atomic_transports);
        if (mMode == MODE_CLIENT) {
            debug (DBG_ALL, 2, "Warning! Ignoring request for non-existent channel");
            return NULL;
        }
        debug (DBG_MUXER, 5, "not found channel, got to create it first");
        PTcpTransport * tcp = new PTcpTransport();
        tcp->mMode = MODE_CLIENT;
        tcp->mChannel = chan;
        tcp->SetTransport(this);
        tcp->StartClient();
        lock_debug (atomic_transports);
        (*mTransportList)[chan] = tcp;
        unlock_debug (atomic_transports);
        return_value (PTcpTransport *, tcp);
    }
    PTcpTransport * t = (*mTransportList)[chan];
    unlock_debug (atomic_transports);
    return_value (PTcpTransport *, t);
}

/*! \copydoc:detail PTransport::SetTransport Создаёт новый канал для смежного транспорта.
*/

void PUdtMuxer::SetTransport(PTcpTransport * tcp)
{
    static uint nextchan = 0;
    debug_entering(PUdtMuxer::SetTransport);
    if (++nextchan == (uint) -1)
        nextchan = 1;
    lock_debug (atomic_transports);
    if (!mTransportList)
        mTransportList = new transport_list_t;
    while (mTransportList->find(nextchan) != mTransportList->end())
        if (++nextchan == (uint) -1)
            nextchan = 1;
    debug(DBG_MUXER, 4, string("new channel ") + nextchan + " created");
    (*mTransportList)[nextchan] = tcp;
    unlock_debug (atomic_transports);
    tcp->mChannel = nextchan;
    return_none();
}

/*! \copydoc:detail PTransport::Send
*   Мультиплексирует данные от разных смежных транспортов PTcpTransport.
*   \arg \b channel -- номер канала
*/

bool PUdtMuxer::Send (buffer_t buf, uint size, uint channel)
{
    debug_entering(PUdtMuxer::Send);
    lock(atomic_send);
    mSendHeader.channel = channel;
    mSendHeader.cmd = MUX_DATA;
    mSendHeader.size = size;
    debug(DBG_MEMORY, 5, string("got to send ") + size + " bytes of data");
    mSendBuffer.ReserveHeader();
    mSendBuffer.SetHeader(&mSendHeader);
    mSendBuffer.Copy(buf, size);
    debug(DBG_MUXER, 4, string("UDT sending via ch. ") + channel + " size " + size );
    return_bool(mSendBuffer.Send(this));
}

// Demuxes UDT channel to TCP channels

/*! \copydoc:detail PTransport::Reply
*   Демультиплексирует данные на каналы. Выбирает для каждого канала соответствующий смежный транспорт PTcpTransport
*   и передаёт данные каждого канала соответствующему смежному транспорту.
*/

bool PUdtMuxer::Reply(buffer_t buf, uint size)
{
    debug_entering(PUdtMuxer::Reply);
    lock(atomic_reply);
    if (mPartialHeader) {
        debug(DBG_MUXER, 4, "partial header from previous chunk");
        int rest = min((int)size, (int)sizeof(PSendHeader) - int(mPartialHeader - (char *)&mReplyHeader));
        memcpy(mPartialHeader, buf, rest);
        buf += rest;
        size -= rest;
        mPartialHeader += rest;
        if (int(mPartialHeader - (char *)&mReplyHeader) == sizeof(PSendHeader))
            mPartialHeader = NULL;
        if (!mPartialHeader && mReplyHeader.cmd == MUX_DISCONNECT) {
            debug(DBG_MUXER, 4, string("shutting down channel ") + mReplyHeader.channel);
            ShutdownChannel(mReplyHeader.channel, true);
        }
        /* at this point if mPartialHeader is non-null, then size is 0 */
    }

    PTcpTransport * tcp;

    if (mReplyHeader.size > 0) {
        int rest = min(mReplyHeader.size, (int)size);
        if (tcp = SelectChannel(mReplyHeader.channel)) {
            debug(DBG_MUXER, 4, string("(continue) replying data at ch. ") + mReplyHeader.channel + " size " + rest);
            if (!tcp->Send(buf, rest))
                debug (DBG_ALL, 2, string("(continue) Warning! Replying failed!"));
        } else
            debug (DBG_ALL, 2, string("(continue) Warning! Discarding data on nonexistent ch. ") + mReplyHeader.channel + " size " + rest);
        size -= rest;
        mReplyHeader.size -= rest;
        buf += rest;
    }
    while (size) {
        if (sizeof(PSendHeader) > size) {
            debug(DBG_MUXER, 4, "got partial header");
            memcpy (&mReplyHeader, buf, size);
            mPartialHeader = (char *)&mReplyHeader + size;
            break;
        }
        memcpy (&mReplyHeader, buf, sizeof(PSendHeader));
        size -= sizeof(PSendHeader);
        buf += sizeof(PSendHeader);
        if (mReplyHeader.cmd == MUX_DISCONNECT) {
            debug(DBG_MUXER, 4, string("shutting down channel ") + mReplyHeader.channel);
            ShutdownChannel(mReplyHeader.channel, true);
            continue;
        }
        if (!size)
            break;
        int rest = min((int)size, mReplyHeader.size);
        if (tcp = SelectChannel(mReplyHeader.channel)) {
            debug(DBG_MUXER, 4, string("replying data at ch. ") + mReplyHeader.channel + " size " + rest);
            if (!tcp->Send(buf, rest))
                debug (DBG_ALL, 2, string("Warning! Replying failed!"));
        } else
            debug (DBG_ALL, 2, string("Warning! Discarding data on nonexistent ch. ") + mReplyHeader.channel + " size " + rest);
        mReplyHeader.size -= rest;
        size -= rest;
        buf += rest;
    }
    unlock(atomic_reply);
    return_bool (true);
}

/* The meaning of this method's name now changes.
In UDT multichannel model it meant 'shutdown UDT client on this end'.
Now it means 'shutdown TCP client on other end'. */

/*! Запрашивает удаление канала \b channel через внешний сокет на другом конце UDT соединения.
*   После чего вызывывает ShutdownChannel().
*/

void PUdtMuxer::RequestShutdown(uint channel)
{
    debug_entering(PUdtMuxer::RequestShutdown);
    // lock_debug(atomic_request);
    PSendHeader h;
    h.channel = channel;
    h.cmd = MUX_DISCONNECT;
    h.size = 0;
    debug(DBG_MUXER, 3, string("sending shutdown request for ch. ") + channel);
    PUdtTransport::Send((char *)&h, sizeof(PSendHeader));
    // unlock_debug(atomic_request);
    // if (mShuttingChannel != channel) {
    ShutdownChannel(channel);
    // }
    return_none();
}

// deadlock sit 1:
// 1: PTcpTransport::ClientThread -> RequestShutdown lock(atomic_request)
// 2: PUdtMuxer::Reply -> ShutdownChannel lock(atomic_shutdown)
// 1: locks on atomic_shutdown
// 2: PTcpTransport::ShutdownThread -> locks on mThreadRunning

// req: ClientThread must be running on moment when ShutdownChannel called

/*! Удаляет канал под номером \b chan. Если \b remote == \b true, удаляет также и смежный транспорт,
*   соответствующий этому каналу.
*/

void PUdtMuxer::ShutdownChannel(uint chan, bool remote)
{
    debug_entering(PUdtMuxer::ShutdownChannel);
    // if (mShuttingChannel == chan)
    //    return_none();
    lock_debug (atomic_shutdown);
    // mShuttingChannel = chan;
    lock_debug (atomic_transports);
    if (!mTransportList || mTransportList->find(chan) == mTransportList->end()) {
        unlock(atomic_transports);
        debug(DBG_ALL, 2, string("Warning! Nothing to shutdown (channel ") + chan + ")");
        // mShuttingChannel = 0;
        unlock(atomic_shutdown);
        return_none ();
    }
    debug (DBG_MUXER, 3, string("shutting down channel ") + chan);
    PTcpTransport * t;
    if (remote) {
        t = (*mTransportList)[chan];
        if (t->CheckShutting()) {
            debug (DBG_THREADS, 3, string("transport at ch. ") + chan + " already shutting");
            unlock_debug (atomic_transports);
            unlock_debug (atomic_shutdown);
            return;
        }
    }
    mTransportList->erase (chan);
    unlock_debug (atomic_transports);
    if (remote) {
        t->ShutdownThread();
        // TODO: PMainClient::ServiceThread automatic variables cleanup (maybe)
    }
    // TODO: why windows dumps in this
    //    if (mMode == MODE_SERVER)
    //        delete t;
    // mShuttingChannel = 0;
    unlock_debug (atomic_shutdown);
    return_none ();
}

// called when UDT connection got broken

/*! \copydoc:detail PTransport::ShutdownThread Удаляет все каналы и их смежные транспорты.
*/

void PUdtMuxer::ShutdownThread()
{
    debug_entering (PUdtMuxer::ShutdownThread);
    lock_debug (atomic_transports);
    if (!mTransportList) {
        unlock_debug (atomic_transports);
        return_none ();
    }
    while (mTransportList->size() > 0) {
        PTcpTransport * t = mTransportList->begin()->second;

        if (t->CheckShutting()) {
            debug (DBG_THREADS, 3, string("transport at ch. ") + t->mChannel + " already shutting");
            unlock_debug (atomic_transports);
            return;
        }

        t->ShutdownThread();
        if (mMode == MODE_SERVER)
            delete t;
        mTransportList->erase(mTransportList->begin());
    }
    unlock_debug (atomic_transports);
    return_none ();
}

void PUdtMuxer::GiveTransport(PTransport<PUdtMuxer, UDTSOCKET> * t)
{
    debug_entering(PUdtMuxer::GiveTransport);
    PUdtMuxer * new_muxer = (PUdtMuxer *) t;
    lock_debug(atomic_transports);
    if (!mTransportList) {
        unlock_debug (atomic_transports);
        return_none ();
    }
    for (transport_list_t::const_iterator t = mTransportList->begin();t != mTransportList->end(); ++t) {
        t->second->SetTransport(new_muxer);
    }
    new_muxer->mTransportList = mTransportList;
    mTransportList = NULL;
    unlock_debug(atomic_transports);
    return_none();
}

#if 0
// Muxes TCP channels to UDT channel
bool PUdtMuxer::Send(uint channel, char * buf, int size)
{
    debug_entering(PUdtMuxer::Send);
    // TODO: init mPrevChannel = -1; mHeader.size = 0;
    if (mPrevChannel == -1) {
        mPrevChannel = channel;
        mSendSize = sizeof(PSendHeader);
        mSendBuffer.ReserveHeader();
        mSendHeader.channel = channel;
    }
    if (channel != mPrevChannel) {
        mSendBuffer.SetHeader(&mSendHeader);
        if (mSendSize > mMtu - mMinChanSize) {
            mSendBuffer.Send(this);
            mSendSize = 0;
        }
        mSendSize += sizeof(PSendHeader);
        mSendBuffer.ReserveHeader();
        mSendHeader.channel = channel;
        mSendHeader.size = 0;
        mPrevChannel = channel;
    }
    mSendSize += size;
    /* Yet do not restrict sending by MTU */
#if 0
    if (mSendSize > mMtu) {
        size_rest = mSendSize - mMtu;
        size -= size_rest;
        mSendSize = mMtu;
        mReserveBuffer.Copy(buf + size, size_rest);
    }
#endif
    mSendHeader.size += size;
    mSendBuffer.Copy(buf, size);
    if (mSendSize >= mMtu) {
        mSendBuffer.SetHeader(&mSendHeader);
        mSendBuffer.Send(this);
        mSendSize = 0;
    }
    // PUdtTransport::Send(buf, size);
    return_none();
}
#endif
