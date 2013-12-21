#ifndef __QY_THREAD_WIN_H__
#define __QY_THREAD_WIN_H__

#include "qyutil/qyconfig.h"
#include "qyutil/qythread.h"
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
    void wakeUp();
    bool setAffinity(const int* processor_numbers, const unsigned int amount_of_processors);
    QyThreadId threadId() const;
    static unsigned int WINAPI startThread(LPVOID lp_parameter);
    
protected:
    virtual void run();
    
private:
    QyThreadWindowPrivate* d_ptr_;
};

END_NAMESPACE(qy)

#endif  // __QY_THREADWIN_H__
