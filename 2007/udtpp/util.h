/* 
    Copyright (c) 2006 Aleksey Midenkov. All rights reserved.
*/

#ifndef __util_h
#define __util_h

#ifdef WIN32
//#define PTW32_STATIC_LIB
#include <pthread.h>
typedef unsigned int uint;
typedef unsigned long in_addr_t;
#endif

typedef unsigned long int ulong;
typedef const char * buffer_t;

#include <string>

std::string operator+ (std::string, int);
void debug_print (int level, const void * obj, buffer_t func_name, buffer_t file, int line, buffer_t msg);
void debug_print (int level, const void * obj, buffer_t func_name, buffer_t file, int line, std::string msg);
void msleep (int milliseconds);

extern int gLogLevel;
extern int gLogFacilities;
extern bool gRendezvous;
extern uint gMagicNumber;
extern class PApp & gApp;

#ifdef DEBUG

enum {
    DBG_GENERAL = 1,
    DBG_TRANSFER = 2,
    DBG_THREADS = 4,
    DBG_MUXER = 8,
    DBG_MEMORY = 16,
    DBG_ALL = 0xffff
};

#define debug2(OBJ, F, L, M) { \
    if (F & gLogFacilities && L <= gLogLevel) \
        debug_print(L, (void *)OBJ, __func_name, __FILE__, __LINE__, M); \
}

#define debug(F, L, M) debug2(this, F, L, M)

#define debug_entering2(OBJ, F) \
    char * __func_name = #F; \
    debug2 (OBJ, DBG_GENERAL|DBG_THREADS, 4, "entering")

#define debug_entering(F) debug_entering2(this, F)

#if 0
// Fucking Microsoft did not know variadic macros until 2005
#define debug_exiting2(OBJ, ...) \
    __debug_exiting((void *)OBJ, __func_name, __VA_ARGS__)

#define debug_exiting(...) debug_exiting2(this, ##__VA_ARGS__)
#endif

#define return_none2(OBJ) { \
    if ((DBG_GENERAL|DBG_THREADS) & gLogFacilities && 5 <= gLogLevel) \
        debug_print(4, OBJ, __func_name, __FILE__, __LINE__, "exiting"); \
    return; \
}

#define return_value2(OBJ, type, V) { \
    type ret = V; \
    if ((DBG_GENERAL|DBG_THREADS) & gLogFacilities && 5 <= gLogLevel) \
        debug_print(4, OBJ, __func_name, __FILE__, __LINE__, std::string("returning ") + (ulong)ret); \
    return ret; \
}

#define return_none() return_none2(this)
#define return_value(type, V) return_value2(this, type, V)
#define return_bool(V) return_value(bool, V)
#define return_int(V) return_value(int, V)

#define error(L, M) debug_print(L, (void *) this, __func_name, __FILE__, __LINE__, M)

#else
#define debug(F, L, M)
#define debug_entering(F)
#define return_none2() return
#define return_value2() return V
#define return_none() return
#define return_value(V) return V
#define return_bool(V) return V
#define return_int(V) return V
#define error(L, M) debug_print(L, NULL, "", __FILE__, __LINE__, M)
#endif

typedef pthread_t thread_t;
typedef pthread_mutex_t mutex_t;
typedef pthread_cond_t cond_t;
#define thread_self() pthread_self()
#define mutex_init(M) pthread_mutex_init(&M, NULL)
#define cond_declare(C) \
    cond_t C; \
    mutex_t C ## _lock;
#define cond_init(C) \
    mutex_init(C ## _lock); \
    pthread_cond_init(&C, NULL)
#define mutex_init_errorcheck(M) { \
    pthread_mutexattr_t M ## _attr; \
    pthread_mutexattr_init (&M ## _attr); \
    pthread_mutexattr_settype(&M ## _attr, PTHREAD_MUTEX_ERRORCHECK); \
    pthread_mutex_init (&M, &M ## _attr); \
}
#define mutex_destroy(M) pthread_mutex_destroy(&M)
#define cond_destroy(C) \
    mutex_destroy(C ## _lock); \
    pthread_cond_destroy(&C)
#define lock(M) pthread_mutex_lock(&M)
#define trylock(M) (EBUSY != pthread_mutex_trylock(&M))
#define unlock(M) pthread_mutex_unlock(&M)
#define cond_wait(C) \
    lock(C ## _lock); \
    pthread_cond_wait(&C, &C ## _lock); \
    unlock(C ## _lock)
#define cond_signal(C) \
    lock(C ## _lock); \
    pthread_cond_signal(&C); \
    unlock(C ## _lock)
#define cond_wait_debug(C) \
    debug(DBG_THREADS, 3, std::string("Waiting signal ") + #C); \
    cond_wait(C); \
    debug(DBG_THREADS, 3, std::string("Got signal ") + #C);
#define cond_signal_debug(C) \
    debug(DBG_THREADS, 3, std::string("Signalling ") + #C); \
    cond_signal(C);
#define thread_run(T) { \
    pthread_t rcvthread; \
    pthread_create(&rcvthread, NULL, (void* (*)(void*))T, this); \
    pthread_detach(rcvthread); \
}


#ifdef WIN32
#include <WinSock2.h>
//typedef HANDLE thread_t;
//typedef HANDLE mutex_t;
//#define thread_self() (thread_t)GetCurrentThreadId()
//#define mutex_init(M) M = CreateMutex(NULL, false, NULL)
//#define mutex_init_errorcheck(M) mutex_init(M)
//#define mutex_destroy(M) CloseHandle(M)
//#define lock(M) WaitForSingleObject(M, INFINITE)
//#define trylock(M) (WAIT_OBJECT_0 == WaitForSingleObject(M, 0))
//#define unlock(M) ReleaseMutex(M)
//#define thread_run(T) CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)T, this, 0, NULL)

inline void inet_aton(buffer_t ip_addr, in_addr * sin_addr)
{
	sin_addr->s_addr = inet_addr(ip_addr);
}
#endif

#define lock_debug(M) { \
    debug(DBG_THREADS, 3, std::string("Locking ") + #M); \
    lock(M); \
    debug(DBG_THREADS, 3, std::string("Locked ") + #M); \
}

#define unlock_debug(M) { \
    unlock(M); \
    debug(DBG_THREADS, 3, std::string("Unlocked ") + #M); \
}

typedef struct sockaddr sockaddr_t;
typedef struct sockaddr_in sockaddr_in_t;
typedef unsigned short int port_t;
#define sockaddr_size sizeof(sockaddr_t)

#define aware_transfer(FUNC, SOCK, BUF, SIZE, FLAGS) \
{ \
    register int n = FUNC (SOCK, (char *) BUF, SIZE, FLAGS); \
    if (n != SIZE) { \
        error (2, string(serror(serrno)) + " (maybe partial data was transferred)"); \
        sclose (SOCK); \
        return_none(); \
    } \
}

#endif //__util_h
