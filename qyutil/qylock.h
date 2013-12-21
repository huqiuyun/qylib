#ifndef __QY_LOCK_H__
#define __QY_LOCK_H__

#include "qyutil/qyconfig.h"
#include "qyutil/qylock_platform.h"

DEFINE_NAMESPACE(qy)

#if defined(WIN32) || defined(_WIN32) || defined(WINCE)

class QySemaphore
{
private:
	QySemaphore& operator=(const QySemaphore& r);
public:
	QySemaphore(PHQyEvent handle)
		:handle_(handle)
	{
		*handle_ = ::CreateSemaphore(0,1,0XFFFF,0);
	}
	~QySemaphore()
	{
		ReleaseSemaphore(*handle_,1,0);
		qyCloseHandle(*handle_);
	}
	void lock()
	{
		::WaitForSingleObject(*handle_,INFINITE);
	}
	void unlock()
	{
		ReleaseSemaphore(*handle_,1,0);
	}
private:
	PHQyEvent   handle_;
};
#endif //WIN32

class QyMutex
{
private:
	PHQyMutex handle_;

public:
	QyMutex(PHQyMutex handle)
		:handle_(handle)
	{
		qythread_createMutex(handle_,NULL,FALSE,NULL);
	}

	QyMutex()
	{
		qythread_closeMutex(handle_);
	}

	void lock()
	{
		qythread_lockMutex(handle_);
	}

	void unlock()
	{
		qythread_unlockMutex(handle_);
	}
};

class QyCriticalSection
{
private:
	PHQyCriticalSection handle_;

public:
	QyCriticalSection(PHQyCriticalSection handle)
		:handle_(handle)
	{
		qythread_init_crisection(handle_);
	}

	~QyCriticalSection()
	{
		qythread_destroy_crisection(handle_);
	}

	void lock()
	{
		qythread_enter_crisection(handle_);
	}

	void unlock()
	{
		qythread_leave_crisection(handle_);
	}
};

class QyEvent
{
protected:
	PHQyEvent handle_;

public:
	QyEvent(PHQyEvent handle,BOOL bManual)
		:handle_(handle)
	{
		qythread_createEvent(handle_,NULL,bManual,FALSE,NULL);
	}

	~QyEvent()
	{
		qythread_closeEvent(handle_);
	}

	BOOL set()
	{
		return qythread_setEvent(handle_);
	}

	BOOL reset()
	{
		return qythread_resetEvent(handle_);
	}
	
	int wait(unsigned long cms)
	{
		return qythread_waitEvent(handle_,cms);
	}

	void wakeUp()
	{
		qythread_setEvent(handle_);
	}
};

class QyManualEvent : public QyEvent
{
public:
	QyManualEvent(PHQyEvent handle)
		:QyEvent(handle,TRUE)
	{
	}

	void lock()
	{
		wait(INFINITE);
		reset();
	}

	void unlock()
	{
		set();
	}
};

class QyAutoEvent : public QyEvent
{
public:
	QyAutoEvent(PHQyEvent handle)
		:QyEvent(handle,FALSE)
	{

	}
	void lock()
	{
		qythread_waitEvent(handle_,INFINITE);
	}

	void unlock()
	{
		set();
	}
};

class QySignalEvent : public QyEvent
{
private:
	HQyEvent event_;
public:

	QySignalEvent()
		:QyEvent(&event_,FALSE)
	{

	}
};

template<typename Type, typename LockPolicy>
class QySmartLock
{
private:
	Type		type_;
	LockPolicy	policy_;

public:
	QySmartLock():policy_(&type_)
	{
	}

	void lock()
	{
		policy_.lock();
	}

	void unlock()
	{
		policy_.unlock();
	}

	LockPolicy* operator->()
	{
		return &policy_;
	}
};

template<typename Type, typename LockPolicy>
class QyAutoLock
{
private:
	QySmartLock<Type,LockPolicy> *lock_;
public:
	QyAutoLock(QySmartLock<Type,LockPolicy> *lock)
		:lock_(lock)
	{
		lock_->lock();
	}

	~QyAutoLock()
	{
		lock_->unlock();
	}
};

#define QY_CS              HQyCriticalSection,  qy::QyCriticalSection
#define QY_EVENT           HQyEvent,            qy::QyEvent
#define QY_MUTEX           HQyMutex,            qy::QyMutex
typedef QyAutoLock<QY_CS>  QyAutoLocker;
typedef QySmartLock<QY_CS> QySLCS;
END_NAMESPACE(qy)

#endif //__QY_LOCK_H__
