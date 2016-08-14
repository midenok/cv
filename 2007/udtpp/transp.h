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

//! Транспорт данных между клиентом и сервером

/*! Задача транспорта не в том, чтобы соединять два внешних сокета напрямую.
*   А в том, чтобы соединить внешний сокет с другим (смежным) транспортом. Все данные,
*   принятые с внешнего сокета перенаправляются смежному транспорту.
*   И наоборот -- все данные, принятые от смежного транспорта перенаправляются
*   на внешний сокет.\n
*   Параметр шаблона \c T задаёт тип смежного транспорта. \n
*   Параметр шаблона \c P задаёт тип внешнего сокета.
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

    //! Запуск RecieveLoop() сервиса
    void ServiceLoop(P);

    //! Запуск ClientThread() в новом трэде
    void StartClient();

    //! Запуск RecieveLoop() клиента
    void ClientThread();

    //! Цикл приёма/передачи данных
    /*! Принимает данные с внешнего сокета и передаёт их смежному транспорту.
    */
    void RecieveLoop();

    //! Остановка трэда транспорта
    /*! Закрывает внешний сокет и завершает трэд транспорта.
    */
    void ShutdownThread();

    //! Установление соединения с внешним сокетом
    void Connect();

    //! Отправка данных внешнему сокету
    /*! Отправляет данные внешнему сокету.
    *   \arg \b buf -- буфер с данными для отправки
    *   \arg \b size -- размер данных в байтах
    */
    bool Send(buffer_t buf, uint size);

    //! Обработка и отправка данных внешнему сокету смежного транспорта
    /*! Вызывает метод Send() смежного транспорта. Перед вызовом делает все необходимые
    *   преобразования данных.
    */
    virtual bool Reply(buffer_t buf, uint size)
    {
        debug_entering(PTransport::Reply);
        return_bool (mTransport->Send(buf, size));
    }

    //! Установка смежного транспорта
    /*! Устанавливает смежный транспорт.
    */
    void SetTransport(T * t) { mTransport = t; }

    //! Проверка исполнения RecieveLoop() для режимов клиента и сервера
    /*! Возвращает \b true, если в данный момент времени у транспорта исполняется
    *   RecieveLoop() для режима \b loop. Допустимые значения для loop: \b LOOP_SERVICE, \b LOOP_CLIENT.
    */
    bool AtLoop(int loop) { return mState & loop; }

    //! Передача смежных транспортов другому транспорту
    /*! Используется при разрыве соединения с внешним сокетом для прозрачного восстановления соединения.
    *   Все смежные транспорты передаются новому соединению \b t.
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
