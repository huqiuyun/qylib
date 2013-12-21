#ifndef QY_THREAD_WIN_H__
#define QY_THREAD_WIN_H__

#include "qytask-config.h"
#include "qythread.h"
#include <windows.h>

DEFINE_NAMESPACE(qy)

class QyThreadWindowPrivate;
class QyThreadWindow : public QyThread {
public:
    QyThreadWindow(QyThreadRunFunction func, QyThreadObj obj, eThreadPriority prio, const char* thread_name);
    virtual ~QyThreadWindow();
    
    void setNotAlive();
    bool start();
    bool stop(unsigned long msec = 2000);
    void wake();
    bool setAffinity(const int* processor_numbers, const unsigned int amount_of_processors);
    QyThreadId threadId() const;
    void sleep(int msecs);

    static unsigned int WINAPI startThread(LPVOID lp_parameter);
    
protected:
    virtual void run();
    
private:
    QyThreadWindowPrivate* d_ptr;
};

END_NAMESPACE(qy)

#endif  // QY_THREAD_WIN_H__
