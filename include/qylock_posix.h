#ifndef QY_LOCK_POSIX_H__
#define QY_LOCK_POSIX_H__

#include "qydefine.h"
#include <pthread.h>
#include <cerrno>
#include <errno.h>
#include <sys/time.h>
#include <string.h>

DEFINE_NAMESPACE(qy)

// criticalsection
typedef pthread_mutex_t HQyCriticalSection, *PHQyCriticalSection;
typedef pthread_mutex_t HQyMutex, *PHQyMutex;

typedef struct t_qyThreadEvent
{
	pthread_mutex_t		mutex_;
	pthread_cond_t		cond_;
    int state_;
}HQyEvent, *PHQyEvent;

typedef void*       LPSERCURITY_ATTRIBUTES;
typedef char*       LPTSTR;
const unsigned long INFINITE = -1;
const int  kEDown  = 0;
const int  kEUp    = 1;
const long int kE6 = 1000000;
const long int kE9 = 1000 * kE6;
//-------------------------------------------------------//
// criticalSection

inline int qythread_init_crisection(PHQyCriticalSection handle)
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	return pthread_mutex_init(handle, &attr);
}

inline int qythread_destroy_crisection(PHQyCriticalSection handle)
{
    return pthread_mutex_destroy(handle);
}

inline int qythread_enter_crisection(PHQyCriticalSection handle)
{
    return pthread_mutex_lock(handle);
}

inline int qythread_leave_crisection(PHQyCriticalSection handle)
{
    return pthread_mutex_unlock(handle);
}

//-------------------------------------------------------//
// mutex

inline BOOL qythread_createMutex(PHQyMutex handle,
                                 LPSERCURITY_ATTRIBUTES lpAttributes,
                                 BOOL bInitialOwner,
                                 LPTSTR lpName)

{
    QY_UNUSED(lpName);
    QY_UNUSED(bInitialOwner);
    QY_UNUSED(lpAttributes);
    
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutex_init(handle, &attr);
	pthread_mutexattr_destroy(&attr);
    return TRUE;
}

inline BOOL qythread_closeMutex(PHQyMutex handle)
{
	if (!handle)
	{
		return FALSE;
	}
	pthread_mutex_destroy(handle);
	return TRUE;
}


inline BOOL qythread_lockMutex(PHQyMutex handle)
{
	if (!handle)
	{
		return FALSE;
	}
	pthread_mutex_lock(handle);
	return TRUE;
}

inline BOOL qythread_unlockMutex(PHQyMutex handle)
{
	if (!handle)
	{
		return FALSE;
	}
	pthread_mutex_unlock(handle);
	return TRUE;
}

//-------------------------------------------------------//
// event

inline BOOL qythread_createEvent(PHQyEvent handle,
                                 LPSERCURITY_ATTRIBUTES lpEventAttributes,
                                 BOOL bManualReset,
                                 BOOL bInitialState,
                                 LPTSTR lpName)

{
    QY_UNUSED(lpName);
    QY_UNUSED(lpEventAttributes);
    QY_UNUSED(bManualReset);
    QY_UNUSED(bInitialState);

	memset(handle,0,sizeof(HQyEvent));
    
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutex_init(&handle->mutex_, &attr);
	pthread_mutexattr_destroy(&attr);
	pthread_cond_init(&handle->cond_, NULL);
    return TRUE;
}

inline BOOL qythread_closeEvent(PHQyEvent handle)
{
	pthread_cond_destroy(&handle->cond_);
	pthread_mutex_destroy(&handle->mutex_);
	
	return TRUE;
}

inline BOOL qythread_setEvent(PHQyEvent handle)
{
    if (0 != pthread_mutex_lock(&handle->mutex_))
    {
        return FALSE;
    }
	handle->state_ = kEUp;
    //唤醒所有被阻塞在条件变量cond_t上的线程。
	pthread_cond_broadcast(&handle->cond_);
	pthread_mutex_unlock(&handle->mutex_);
	
	return TRUE;
}

inline BOOL qythread_resetEvent(PHQyEvent handle)
{
    if (0 != pthread_mutex_lock(&handle->mutex_)) {
        return FALSE;
    }
    handle->state_ = kEDown;
    pthread_mutex_unlock(&handle->mutex_);
    
	return TRUE;
}

inline int qythread_waitEvent(PHQyEvent handle, unsigned long dwTime)
{
    if (0 != pthread_mutex_lock(&handle->mutex_))
    {
        return kEventError;
    }
    int res = 0;
    if (kEDown == handle->state_){
        if (dwTime == INFINITE)
        {
            //使线程阻塞在一个条件变量的互斥锁上，无条件等待
            res = pthread_cond_wait(&handle->cond_, &handle->mutex_);
        }else{
            
            struct timeval now;      /*time when we started waiting*/
            struct timespec timeout; /*timeout value for the wait function */
            
            //取当前时间
            gettimeofday(&now, NULL);
            
            //准备时间间隔值
            timeout.tv_sec  = now.tv_sec + dwTime / 1000;
            timeout.tv_nsec = ((now.tv_usec + dwTime) % 1000) * 1000;
            
            if (timeout.tv_nsec >= kE9) {
                timeout.tv_sec++;
                timeout.tv_nsec -= kE9;
            }
            //使线程阻塞在一个条件变量的互斥锁上，计时等待
            res = pthread_cond_timedwait(&handle->cond_, &handle->mutex_, &timeout);
        }
    }
    
    handle->state_ = kEDown;
    pthread_mutex_unlock(&handle->mutex_);		//unlock
	
    if (ETIMEDOUT == res)
	{
        return kEventTimeout;
	}
    return kEventOk;
}

END_NAMESPACE(qy)

#endif //QY_LOCK_POSIX_H__
