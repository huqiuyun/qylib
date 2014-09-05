#ifndef QY_CRITICALSECTION_H__
#define QY_CRITICALSECTION_H__

#include "qysysconfig.h"

#ifdef _DEBUG
#define CS_TRACK_OWNER 1
#endif  // _DEBUG

#if CS_TRACK_OWNER
#define TRACK_OWNER(x) x
#else  // !CS_TRACK_OWNER
#define TRACK_OWNER(x)
#endif  // !CS_TRACK_OWNER

#ifdef H_OS_WIN
   #include <windows.h>
#else
   #include <pthread.h>
#endif

namespace qy 
{

#ifdef H_OS_WIN

    class QyCriticalSection
	{
	public:
        QyCriticalSection()
		{
            InitializeCriticalSection(&mCrit);
			// Windows docs say 0 is not a valid thread id
			TRACK_OWNER(thread_ = 0);
		}
        ~QyCriticalSection()
		{
            DeleteCriticalSection(&mCrit);
		}
		void Enter()
		{
            EnterCriticalSection(&mCrit);
			TRACK_OWNER(thread_ = GetCurrentThreadId());
		}
		void Leave()
		{
			TRACK_OWNER(thread_ = 0);
            LeaveCriticalSection(&mCrit);
		}

#if CS_TRACK_OWNER
		bool CurrentThreadIsOwner() const { return thread_ == GetCurrentThreadId(); }
#else
		bool CurrentThreadIsOwner() const { return true;}
#endif  // CS_TRACK_OWNER

	private:
        CRITICAL_SECTION mCrit;
		TRACK_OWNER(DWORD thread_);  // The section's owning thread id
	};

#else
    class QyCriticalSection
	{
	public:
        QyCriticalSection()
		{
			pthread_mutexattr_t mutex_attribute;
			pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_RECURSIVE);
            pthread_mutex_init(&mMutex, &mutex_attribute);
		}
        ~QyCriticalSection()
		{
            pthread_mutex_destroy(&mMutex);
		}
		void Enter()
		{
            pthread_mutex_lock(&mMutex);
		}
		void Leave() 
		{
            pthread_mutex_unlock(&mMutex);
		}
	private:
        pthread_mutex_t mMutex;
	};
#endif // !H_OS_WIN

    // QyCritScope, for serializing exection through a scope
    class QyCritScope
	{
	public:
        QyCritScope(QyCriticalSection *pcrit)
		{
            mCrit = pcrit;
            mCrit->Enter();
		}
        ~QyCritScope() {
            mCrit->Leave();
		}
	private:
        QyCriticalSection *mCrit;
	};
} // namespace qy 

#endif // __CRITICALSECTION_H__
