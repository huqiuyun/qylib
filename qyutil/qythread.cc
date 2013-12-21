#include "qythread.h"

#if defined(_WIN32)
#include "qythread_win.h"
#else
#include "qythread_posix.h"
#endif

DEFINE_NAMESPACE(qy)

QyThread* QyThread::createThread(QyThreadRunFunction func,
                                 QyThreadObj obj, eThreadPriority prio,
                                 const char* thread_name) {
#if defined(_WIN32)
    return new QyThreadWindow(func, obj, prio, thread_name);
#else
    return QyThreadPosix::create(func, obj, prio, thread_name);
#endif
}

END_NAMESPACE(qy)
