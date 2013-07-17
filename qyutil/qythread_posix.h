#ifndef __QY_THREAD_POSIX_H__
#define __QY_THREAD_POSIX_H__

#include "qyutil/qythread.h"
#include <pthread.h>

DEFINE_NAMESPACE(qy)

class QyThreadPosixPrivate;
class QyThreadPosix : public QyThread {
public:
    static QyThread* create(QyThreadRunFunction func, QyThreadObj obj,
                            eThreadPriority prio, const char* thread_name);
    
    QyThreadPosix(QyThreadRunFunction func, QyThreadObj obj, eThreadPriority prio,
                  const char* thread_name);
    ~QyThreadPosix();
    
    void setNotAlive();
    bool start();
    bool stop(int msec = 2000);
    void waitUp();
    // Not implemented on Mac.
    bool setAffinity(const int* processor_numbers,
                     unsigned int amount_of_processors);
    
    QyThreadId threadId() const;
    void run();
    
private:
    int construct();
    
private:
    QyThreadPosixPrivate* d_ptr_;
};

END_NAMESPACE(qy)

#endif  // __QY_THREAD_POSIX_H__
