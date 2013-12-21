#ifndef QY_THREAD_PLATFORM_H__
#define QY_THREAD_PLATFORM_H__

#if defined(H_OS_WIN)
#include "qylock_win.h"
#else //!//defined(H_OS_WIN)
#include "qylock_posix.h"
#endif


#endif //QY_THREAD_PLATFORM_H__

