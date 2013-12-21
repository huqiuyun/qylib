#include "qythread.h"

#if defined(H_OS_WIN)
#include "qythread_win.h"
#else
#include "qythread_posix.h"
#endif

DEFINE_NAMESPACE(qy)

QyThread* QyThread::createThread(QyThreadRunFunction func,
                                 QyThreadObj obj, ThreadPriority prio,
                                 const char* thread_name) {
#if defined(H_OS_WIN)
    return new QyThreadWindow(func, obj, prio, thread_name);
#else
    return QyThreadPosix::create(func, obj, prio, thread_name);
#endif
}

END_NAMESPACE(qy)
