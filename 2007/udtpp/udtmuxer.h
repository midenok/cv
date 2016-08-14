/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

#ifdef __udtmuxer_h
class PUdtMuxer;
#else
#define __udtmuxer_h

#include "udttransp.h"
#include "tcptransp.h"
#include "app.h"

#include <map>

enum MuxerCmds
{
    MUX_DATA = 0,
    MUX_DISCONNECT = 7
};

//! \cond undoc

struct PSendHeader
{
    uint channel;
    char cmd;
    int size;
};

class PUdtMuxer;

class PSendBuffer
{
public:
    PSendBuffer();
    ~PSendBuffer();
    void ReserveHeader();
    void Copy(buffer_t buf, uint size);
    void SetHeader(PSendHeader * header);
    bool Send(PUdtMuxer *);

private:
    void ExpandCapacity(int size);
    char * mBuffer;
    uint mBufferSize;
    char * mHeader;
    int mBufferCapacity;
    static int mBlockSize;
};

//! \endcond

//! Транспорт мультиплексор-демультиплексор каналов через UDT

/*! Управляет списком каналов. Каждый канал имеет свой номер и соответствующий PTcpTransport. \n \n
*   О понятии \b транспорт см. \link PTransport PTransport подробное описание \endlink
*/

class PUdtMuxer : public PUdtTransport
{
    friend class PSendBuffer;
    inline void Init(sockaddr_t &);

public:
    PUdtMuxer(sockaddr_t &);
    PUdtMuxer(sockaddr_t &, PUdtServer *);
    ~PUdtMuxer();

    //! Удаление канала с удалённым оповещением
    void RequestShutdown(uint channel);

    //! Удаление канала
    void ShutdownChannel(uint channel, bool remote=false);

    //! Выбор смежного транспорта по номеру канала
    PTcpTransport * SelectChannel(int chan);

    // wrapper methods
    //! \copydoc:brief PTransport::Send
    bool Send(buffer_t buf, uint size, uint channel = 0);

    //! \copydoc:brief PTransport::Reply
    bool Reply(buffer_t buf, uint size);

    //! \copydoc:brief PTransport::SetTransport
    void SetTransport(PTcpTransport *);

    //! \copydoc:brief PTransport::ShutdownThread
    void ShutdownThread();

protected:
    mutex_t atomic_send;

private:
    void GiveTransport (PTransport<PUdtMuxer, UDTSOCKET> *);

    // int mShuttingChannel;
    int mSendSize;
    PSendHeader mSendHeader;
    PSendHeader mReplyHeader;
    char * mPartialHeader;
    int mPrevChannel;
    PSendBuffer mSendBuffer;
    static int mMtu;
    static int mMinChanSize;
    mutex_t atomic_transports;
    mutex_t atomic_reply;
    mutex_t atomic_request;
    mutex_t atomic_shutdown;
    typedef std::map<int, PTcpTransport *> transport_list_t;
    transport_list_t * mTransportList;
};

#endif //__udtmuxer_h
