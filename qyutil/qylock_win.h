#ifndef __QY_LOCK_WIN_H__
#define __QY_LOCK_WIN_H__

#include "qyutil/qyconfig.h"
#include <windows.h>

BEGIN_QYUTIL_NAMESPACE

// criticalsection
typedef CRITICAL_SECTION HQyCriticalSection, *PHQyCriticalSection;
typedef HANDLE           HQyEvent, *PHQyEvent, HQyMutex, *PHQyMutex;


//-------------------------------------------------------//
// criticalSection

inline int qythread_init_crisection(PHQyCriticalSection handle)
{
    InitializeCriticalSection(handle);
    return 0;
}

inline int qythread_destroy_crisection(PHQyCriticalSection handle)
{
    DeleteCriticalSection(handle);
    return 0;
}

inline int qythread_enter_crisection(PHQyCriticalSection handle)
{
    EnterCriticalSection(handle);
    return 0;
}

inline int qythread_leave_crisection(PHQyCriticalSection handle)
{
    LeaveCriticalSection(handle);
    return 0;
}

//-------------------------------------------------------//
// mutex

inline BOOL qythread_createMutex(PHQyMutex handle,
                                 LPSERCURITY_ATTRIBUTES lpAttributes,
                                 BOOL bInitialOwner,
                                 LPTSTR lpName)

{
    *handle_ = CreateMutex(lpAttributes,bInitialOwner,lpName);
    return (*handle != NULL);
}

inline BOOL qythread_closeMutex(PHQyMutex handle)
{
	if (!handle)
	{
		return FALSE;
	}
    qyCloseHandle(*handle);
    *handle = NULL;
	return TRUE;
}


inline BOOL qythread_lockMutex(PHQyMutex handle)
{
	if (!handle)
	{
		return FALSE;
	}
	WaitForSingleObject(*handle,INFINITE);
    return TRUE;
}

inline BOOL qythread_unlockMutex(PHQyMutex handle)
{
	if (!handle)
	{
		return FALSE;
	}
    ReleaseMutex(*handle);
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
    *handle = ::CreateEvent(lpEventAttributes,bManualReset,bInitialState,lpName);
    return (*handle != NULL);
}

inline BOOL qythread_closeEvent(PHQyEvent handle)
{
    qyCloseHandle(*handle);
	return TRUE;
}

inline BOOL qythread_setEvent(PHQyEvent handle)
{
    ::SetEvent(*handle);
	return TRUE;
}

inline BOOL qythread_resetEvent(PHQyEvent handle)
{
	::ResetEvent(*handle);
	return TRUE;
}

inline int qythread_waitEvent(PHQyEvent handle, unsigned long dwTime)
{
    return ::WaitForSingleObject(*handle,dwTime);
}

END_NAMESPACE(qy)

#endif //__QY_LOCK_WIN_H__
