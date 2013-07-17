#ifndef __QY_THREAD_WIN_H__
#define __QY_THREAD_WIN_H__

#include <windows.h>
#include "qyutil/qythread.h"

BEGIN_QYUTIL_NAMESPACE

class QyThreadWindowPrivate;
class QYUTIL_API QyThreadWindow : public QyThread {
public:
    QyThreadWindow(QyThreadRunFunction func, QyThreadObj obj, eThreadPriority prio, const char* thread_name);
    virtual ~QyThreadWindow();
    
    void setNotAlive();
    bool start();
    bool stop(int msec = 2000);
    void wait(int msec);
    void waitUp();
    bool setAffinity(const int* processor_numbers, const unsigned int amount_of_processors);
    QyThreadId threadId() const;
    static unsigned int WINAPI startThread(LPVOID lp_parameter);
    
protected:
    virtual void run();
    
private:
    QyThreadWindowPrivate d_ptr_;
};

END_QYUTIL_NAMESAPCE

#endif  // __QY_THREAD_WIN_H__
