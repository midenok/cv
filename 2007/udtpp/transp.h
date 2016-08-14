/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

#ifdef __transp_h
// class PTransport;
#else
#define __transp_h

#include "util.h"

enum {
    MODE_NONE   = 0,
    MODE_SERVER = 1,
    MODE_CLIENT = 2,
    MODE_CONTROL = 3,
    MODE_STUB = 4
};

enum LoopEnum {
    LOOP_SERVICE = 1,
    LOOP_CLIENT = 2
};

//! ��������� ������ ����� �������� � ��������

/*! ������ ���������� �� � ���, ����� ��������� ��� ������� ������ ��������.
*   � � ���, ����� ��������� ������� ����� � ������ (�������) �����������. ��� ������,
*   �������� � �������� ������ ���������������� �������� ����������.
*   � �������� -- ��� ������, �������� �� �������� ���������� ����������������
*   �� ������� �����.\n
*   �������� ������� \c T ������ ��� �������� ����������. \n
*   �������� ������� \c P ������ ��� �������� ������.
*/

template <class T, class P>
class PTransport
{
    //! \cond undoc
    struct PTransportStaticData
    {
        PTransportStaticData() : mPendingServiceLocked(false), mPendingClientLocked(false)
        {
            mutex_init(mPendingServiceLoop);
            mutex_init(mPendingClientLoop);
        }
        ~PTransportStaticData()
        {
            mutex_destroy(mPendingServiceLoop);
            mutex_destroy(mPendingClientLoop);
        }
        mutex_t mPendingServiceLoop;
        mutex_t mPendingClientLoop;
        bool mPendingServiceLocked;
        bool mPendingClientLocked;
    };
    //! \endcond

public:

    PTransport<T, P> () : mShutdown(false), mConnected(false), mState(0) { mutex_init(mThreadRunning); }
    ~PTransport () { mutex_destroy(mThreadRunning); }

    //! ������ RecieveLoop() �������
    void ServiceLoop(P);

    //! ������ ClientThread() � ����� �����
    void StartClient();

    //! ������ RecieveLoop() �������
    void ClientThread();

    //! ���� ��ɣ��/�������� ������
    /*! ��������� ������ � �������� ������ � �������� �� �������� ����������.
    */
    void RecieveLoop();

    //! ��������� ����� ����������
    /*! ��������� ������� ����� � ��������� ���� ����������.
    */
    void ShutdownThread();

    //! ������������ ���������� � ������� �������
    void Connect();

    //! �������� ������ �������� ������
    /*! ���������� ������ �������� ������.
    *   \arg \b buf -- ����� � ������� ��� ��������
    *   \arg \b size -- ������ ������ � ������
    */
    bool Send(buffer_t buf, uint size);

    //! ��������� � �������� ������ �������� ������ �������� ����������
    /*! �������� ����� Send() �������� ����������. ����� ������� ������ ��� �����������
    *   �������������� ������.
    */
    virtual bool Reply(buffer_t buf, uint size)
    {
        debug_entering(PTransport::Reply);
        return_bool (mTransport->Send(buf, size));
    }

    //! ��������� �������� ����������
    /*! ������������� ������� ���������.
    */
    void SetTransport(T * t) { mTransport = t; }

    //! �������� ���������� RecieveLoop() ��� ������� ������� � �������
    /*! ���������� \b true, ���� � ������ ������ ������� � ���������� �����������
    *   RecieveLoop() ��� ������ \b loop. ���������� �������� ��� loop: \b LOOP_SERVICE, \b LOOP_CLIENT.
    */
    bool AtLoop(int loop) { return mState & loop; }

    //! �������� ������� ����������� ������� ����������
    /*! ������������ ��� ������� ���������� � ������� ������� ��� ����������� �������������� ����������.
    *   ��� ������� ���������� ���������� ������ ���������� \b t.
    */
    virtual void GiveTransport(PTransport<T,P> * t) {};

protected:

    void LoopEntered(int loop)
    {
        debug_entering(PTransport::LoopEntered);
        lock_debug(mThreadRunning);
        mState |= loop;
        if (loop & LOOP_SERVICE && mStatic.mPendingServiceLocked) {
            mStatic.mPendingServiceLocked = false;
            unlock_debug(mStatic.mPendingServiceLoop);
        }
        if (loop & LOOP_CLIENT && mStatic.mPendingClientLocked) {
            mStatic.mPendingClientLocked = false;
            unlock_debug(mStatic.mPendingClientLoop);
        }
        return_none ();
    }

    void LoopExited(int loop)
    {
        debug_entering(PTransport::LoopExited);
        mShutdown = true;
        mConnected = false;
        mState -= mState & loop;
        unlock_debug(mThreadRunning);
        msleep (10);
        // lock_debug(mThreadRunning);
        // unlock_debug(mThreadRunning);
        return_none ();
    }

    int mMode;
    int mChannel;
    int mState;
    T * mTransport;
    bool mShutdown;
    bool mConnected;
    mutex_t mThreadRunning;

private:
    static PTransportStaticData mStatic;
};

template <typename T, typename P>
typename PTransport<T, P>::PTransportStaticData PTransport<T, P>::mStatic;

#endif //__transp_h
