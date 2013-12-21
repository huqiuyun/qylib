#ifndef QY_THREAD_POSIX_H__
#define QY_THREAD_POSIX_H__

#include "qythread.h"
#include <pthread.h>

DEFINE_NAMESPACE(qy)

class QyThreadPosixPrivate;
class QyThreadPosix : public QyThread
{
public:
    static QyThread* create(QyThreadRunFunction func, QyThreadObj obj,
                            ThreadPriority prio, const char* thread_name);
    
    QyThreadPosix(QyThreadRunFunction func, QyThreadObj obj, ThreadPriority prio,
                  const char* thread_name);
    ~QyThreadPosix();
    
    void setNotAlive();
    bool start();
    bool stop(unsigned long msec = 2000);
    void wake();
    // Not implemented on Mac.
    bool setAffinity(const int* processor_numbers,
                     unsigned int amount_of_processors);
    
    QyThreadId threadId() const;
    void sleep(int msecs);
    void run();
    
private:
    int construct();
    
private:
    QyThreadPosixPrivate* d_ptr;
};

END_NAMESPACE(qy)

#endif  // QY_THREAD_POSIX_H__
