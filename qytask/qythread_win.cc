
#include "qythread_win.h"
#include "qylock.h"
#include <assert.h>
#include <process.h>
#include <stdio.h>

DEFINE_NAMESPACE(qy)

struct THREADNAME_INFO
{
    DWORD dwType;     // must be 0x1000
    LPCSTR szName;    // pointer to name (in user addr space)
    DWORD dwThreadID; // thread ID (-1 = caller thread)
    DWORD dwFlags;    // reserved for future use, must be zero
};

void setThreadName(DWORD dwThreadID, LPCSTR szThreadName)
{
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = szThreadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
    
    __try
    {
        RaiseException(0x406D1388, 0, sizeof(info) / sizeof(DWORD),
                       (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_CONTINUE_EXECUTION)
    {
    }
}

class QyThreadWindowPrivate
{
public:
    QyThreadWindowPrivate(QyThreadRunFunction func, QyThreadObj obj,
                          eThreadPriority prio, const char* thread_name)
    :run_function_(func),
    obj_(obj),
    alive_(false),
    dead_(true),
    do_not_close_handle_(false),
    prio_(prio),
    thread_(NULL),
    id_(0),
    set_thread_name_(false) {
        
		name_[0] = '\0';
        if (thread_name != NULL) {
            // Set the thread name to appear in the VS debugger.
            set_thread_name_ = true;
            strncpy(name_, thread_name, kThreadMaxNameLength);
        }
    }
    
    ~QyThreadWindowPrivate()
    {
        if (thread_) {
            CloseHandle(thread_);
            thread_ = NULL;
        }
    }
public:
    QyThreadRunFunction     run_function_;
    QyThreadObj             obj_;
    
    bool                    alive_;
    bool                    dead_;
    
    // TODO(hellner)
    // do_not_close_handle_ member seem pretty redundant. Should be able to remove
    // it. Basically it should be fine to reclaim the handle when calling stop
    // and in the destructor.
    bool                    do_not_close_handle_;
    eThreadPriority         prio_;
    QySignalEvent           event_;
    QySignalEvent           thread_hook_;
    QySmartLock<QY_CS>      critsect_stop_;
    
    HANDLE                  thread_;
    unsigned int            id_;
    char                    name_[kThreadMaxNameLength];
    bool                    set_thread_name_;
};

QyThreadWindow::QyThreadWindow(QyThreadRunFunction func, QyThreadObj obj,
                               eThreadPriority prio, const char* thread_name)
: QyThread()
, d_ptr(new QyThreadWindowPrivate(func,obj,prio,thread_name))
{
}

QyThreadWindow::~QyThreadWindow() {
    delete d_ptr;
}

uint32_t QyThread::currentThreadId() {
	return ::GetCurrentThreadId();
}

unsigned int WINAPI QyThreadWindow::startThread(LPVOID lp_parameter) {
    static_cast<QyThreadWindow*>(lp_parameter)->run();
    return 0;
}

bool QyThreadWindow::start() {
    QY_D(QyThreadWindow);
    if (!d->run_function_) {
        return false;
    }
    if (d->thread_){
		wakeUp();
        unsigned long dw = WaitForSingleObject(d->thread_, 0);
        if (dw != WAIT_OBJECT_0)
        {
            return true;
        }
    }
    d->do_not_close_handle_ = false;
    
	unsigned int thread_id = 0;
    // Set stack size to 1M
    d->thread_ = (HANDLE)_beginthreadex(NULL, 1024 * 1024, startThread, (void*)this,
                                        0, &thread_id);
    if (d->thread_ == NULL) {
        return false;
    }
    d->id_ = thread_id;
    d->event_.wait(INFINITE);//wait thread run
    
    switch (d->prio_) {
        case kLowPriority:
            SetThreadPriority(d->thread_, THREAD_PRIORITY_BELOW_NORMAL);
            break;
        case kNormalPriority:
            SetThreadPriority(d->thread_, THREAD_PRIORITY_NORMAL);
            break;
        case kHighPriority:
            SetThreadPriority(d->thread_, THREAD_PRIORITY_ABOVE_NORMAL);
            break;
        case kHighestPriority:
            SetThreadPriority(d->thread_, THREAD_PRIORITY_HIGHEST);
            break;
        case kRealtimePriority:
            SetThreadPriority(d->thread_, THREAD_PRIORITY_TIME_CRITICAL);
            break;
    };
    return true;
}

bool QyThreadWindow::setAffinity(const int* processor_numbers,
                                 const unsigned int amount_of_processors) {
    QY_D(QyThreadWindow);
    DWORD_PTR processor_bit_mask = 0;
    for (unsigned int processor_index = 0;
         processor_index < amount_of_processors;
         ++processor_index) {
        // Convert from an array with processor numbers to a bitmask
        // Processor numbers start at zero.
        // TODO(hellner): this looks like a bug. Shouldn't the '=' be a '+='?
        // Or even better |=
        processor_bit_mask = 1 << processor_numbers[processor_index];
    }
    return SetThreadAffinityMask(d->thread_, processor_bit_mask) != 0;
}

void QyThreadWindow::setNotAlive() {
    QY_D(QyThreadWindow);
    QyAutoLocker cs(&d->critsect_stop_);
    d->alive_ = false;
    d->thread_hook_.wakeUp();
}

bool QyThreadWindow::stop(unsigned long msec) {
    QY_D(QyThreadWindow);
    d->critsect_stop_.lock();
    
    // Prevents the handle from being closed in QyThreadWindow::run()
    d->do_not_close_handle_ = true;
    d->alive_ = false;
    d->thread_hook_.wakeUp();

    bool signaled = false;
    if (d->thread_ && !d->dead_) {
        d->critsect_stop_.unlock();
        // Wait up to msec seconds for the thread to complete.
        if (WAIT_OBJECT_0 == WaitForSingleObject(d->thread_, msec)) {
            signaled = true;
        }
        d->critsect_stop_.lock();
    }
    if (d->thread_) {
        CloseHandle(d->thread_);
        d->thread_ = NULL;
    }
    d->critsect_stop_.unlock();
    
    //
    d->run_function_(d->obj_,kTStoped);
    
    if (d->dead_ || signaled) {
        return true;
    } else {
        return false;
    }
}

void QyThreadWindow::wake()
{
    QY_D(QyThreadWindow);
    d->critsect_stop_.lock();
    d->thread_hook_.wake();
    d->critsect_stop_.unlock();
}

QyThreadId QyThreadWindow::threadId() const {
    QY_D(QyThreadWindow);
    return d->id_;
}

void QyThreadWindow::run() {
    QY_D(QyThreadWindow);
    d->alive_ = true;
    d->dead_ = false;
	d->event_.wakeUp();
    
    // All tracing must be after event_->Set to avoid deadlock in Trace.
    if (d->set_thread_name_) {
        setThreadName(-1, d->name_); // -1, set thread name for the calling thread.
    }
    
    do {
        int res = d->run_function_(d->obj_,kTRunning);
        if (kTResExit == res) {
            d->alive_ = false;
        }
        else if(kTResHook == res){
            d->thread_hook_.wait(INFINITE);
        }
        
    } while (d->alive_);
    
    d->critsect_stop_.lock();
    
    if (d->thread_ && !d->do_not_close_handle_) {
        HANDLE thread = d->thread_;
        d->thread_ = NULL;
        CloseHandle(thread);
    }
    d->dead_ = true;
    
    d->critsect_stop_.unlock();
}


void QyThreadWindow::sleep(int msecs)
{
    Sleep(msecs);
}

END_NAMESPACE(qy)
