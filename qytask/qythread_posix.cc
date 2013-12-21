#include <assert.h>
#include <algorithm>
#include "qythread_posix.h"
#include "qylock.h"

DEFINE_NAMESPACE(qy)

class QyThreadPosixPrivate
{
    public:
    QyThreadPosixPrivate(QyThreadRunFunction func,QyThreadObj obj,
                         ThreadPriority prio, const char* name) :
        mFunction(func),
        mThreadObject(obj),
        mAlive(false),
        mDead(true),
        mThreadPriority(prio),
        mName(),
        mSetThreadName(false),
    #if (defined(WEBRTC_LINUX) || defined(WEBRTC_ANDROID))
        mPid(-1),
    #endif
        mAttr(),
        mThread(0)
    {
        if (name != NULL) {
            mSetThreadName = true;
            strncpy(mName, name, kThreadMaxNameLength);
            mName[kThreadMaxNameLength - 1] = '\0';
        }
    }
    
    ~QyThreadPosixPrivate()
    {
        pthread_attr_destroy(&mAttr);
    }

    int convertToSystemPriority(ThreadPriority priority, int min_prio, int max_prio)
    {
        assert(max_prio - min_prio > 2);
        const int top_prio = max_prio - 1;
        const int low_prio = min_prio + 1;

        switch (priority) {
        case kLowPriority:
            return low_prio;
        case kNormalPriority:
            // The -1 ensures that the kHighPriority is always greater or equal to
            // kNormalPriority.
            return (low_prio + top_prio - 1) / 2;
        case kHighPriority:
            return std::max(top_prio - 2, low_prio);
        case kHighestPriority:
            return std::max(top_prio - 1, low_prio);
        case kRealtimePriority:
            return top_prio;
        }
        assert(false);
        return low_prio;
    }

    static void* startThread(void* param)
    {
        static_cast<QyThreadPosix*>(param)->run();
        return 0;
    }

    QyThreadRunFunction   mFunction;
    QyThreadObj           mThreadObject;
    
    // Internal state.
    QySLCS                mStateCrit;  // Protects mAlive and mDead
    QySignalEvent         mEvent;
    QySignalEvent         mThreadHook;
    
    bool                  mAlive;
    bool                  mDead;
    ThreadPriority        mThreadPriority;
    
    // Zero-terminated thread name string.
    char                  mName[kThreadMaxNameLength];
    bool                  mSetThreadName;
    
    // Handle to thread.
#if (defined(WEBRTC_LINUX) || defined(WEBRTC_ANDROID))
    mPidt                 mPid;
#endif
    pthread_attr_t        mAttr;
    pthread_t             mThread;
};

QyThread* QyThreadPosix::create(QyThreadRunFunction func, QyThreadObj obj,ThreadPriority prio,const char* name) {
    QyThreadPosix* ptr = new QyThreadPosix(func, obj, prio, name);
    if (!ptr) {
        return NULL;
    }
    const int error = ptr->construct();
    if (error) {
        delete ptr;
        return NULL;
    }
    return ptr;
}

QyThreadPosix::QyThreadPosix(QyThreadRunFunction func, QyThreadObj obj,
                             ThreadPriority prio, const char* name)
    : d_ptr(new QyThreadPosixPrivate(func,obj,prio,name))
{
}

QyThreadId QyThread::currentThreadId() {
    return pthread_self();
}

int QyThreadPosix::construct() {
    QY_D(QyThreadPosix);
    int result = 0;
#if !defined(WEBRTC_ANDROID)
    // Enable immediate cancellation if requested, see Shutdown().
    result = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (result != 0) {
        return -1;
    }
    result = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (result != 0) {
        return -1;
    }
#endif
    result = pthread_attr_init(&d->mAttr);
    if (result != 0) {
        return -1;
    }
    return 0;
}

QyThreadPosix::~QyThreadPosix() {
    delete d_ptr;
}

#define HAS_mThreadID !defined(WEBRTC_IOS) && !defined(WEBRTC_MAC)

bool QyThreadPosix::start()
{
    QY_D(QyThreadPosix);
    int result = pthread_attr_setdetachstate(&d->mAttr, PTHREAD_CREATE_DETACHED);
    // Set the stack stack size to 1M.
    result |= pthread_attr_setstacksize(&d->mAttr, 1024 * 1024);
#ifdef WEBRTC_mThreadRR
    const int policy = SCHED_RR;
#else
    const int policy = SCHED_FIFO;
#endif
    d->mEvent.reset();
    // If pmThreadcreate was successful, a thread was created and is running.
    // Don't return false if it was successful since if there are any other
    // failures the state will be: thread was started but not configured as
    // asked for. However, the caller of this API will assume that a false
    // return value means that the thread never started.
    result |= pthread_create(&d->mThread, &d->mAttr, &QyThreadPosixPrivate::startThread, this);
    if (result != 0) {
        return false;
    }
    {
        QyAutoLocker cs(&d->mStateCrit);
        d->mDead = false;
    }
    
    // Wait up to 10 seconds for the OS to call the callback function. Prevents
    // race condition if Stop() is called too quickly after start.
    if (kEventOk != d->mEvent.wait(10)) {
        
        // Timed out. Something went wrong.
        return true;
    }
    
    sched_param param;
    
    const int min_prio = sched_get_priority_min(policy);
    const int max_prio = sched_get_priority_max(policy);
    
    if ((min_prio == EINVAL) || (max_prio == EINVAL)) {
        return true;
    }
    if (max_prio - min_prio <= 2) {
        // There is no room for setting priorities with any granularity.
        return true;
    }
    param.sched_priority = d->convertToSystemPriority(d->mThreadPriority, min_prio, max_prio);
    result = pthread_setschedparam(d->mThread, policy, &param);
    if (result == EINVAL) {
    }
    return true;
}

// CPU_ZERO and CPU_SET are not available in NDK r7, so disable
// SetAffinity on Android for now.
#if (defined(WEBRTC_LINUX) && (!defined(WEBRTC_ANDROID)))
bool QyThreadPosix::setAffinity(const int* processor_numbers,
                                const unsigned int amount_of_processors) {
    QY_D(QyThreadPosix);
    if (!processor_numbers || (amount_of_processors == 0)) {
        return false;
    }
    cpu_set_t mask;
    CPU_ZERO(&mask);
    
    for (unsigned int processor = 0;
         processor < amount_of_processors;
         ++processor) {
        CPU_SET(processor_numbers[processor], &mask);
    }
#if defined(WEBRTC_ANDROID)
    // Android.
    const int result = syscall(__NR_sched_setaffinity,
                               d->mPid,
                               sizeof(mask),
                               &mask);
#else
    // "Normal" Linux.
    const int result = sched_setaffinity(d->mPid,
                                         sizeof(mask),
                                         &mask);
#endif
    if (result != 0) {
        return false;
    }
    return true;
}

#else
// NOTE: On Mac OS X, use the Thread affinity API in
// /usr/include/mach/mThreadpolicy.h: mThreadpolicy_set and mach_mThreadself()
// instead of Linux gettid() syscall.
bool QyThreadPosix::setAffinity(const int* , const unsigned int) {
    return false;
}
#endif

void QyThreadPosix::setNotAlive() {
    QY_D(QyThreadPosix);
    QyAutoLocker cs(&d->mStateCrit);
    d->mAlive = false;
}

bool QyThreadPosix::stop(unsigned long msec) {
    QY_D(QyThreadPosix);
    QY_UNUSED(msec);
    bool dead = false;
    {
        QyAutoLocker cs(&d->mStateCrit);
        d->mAlive = false;
        d->mThreadHook.wake();
        dead = d->mDead;
    }
    
    // TODO(hellner) why not use an event here?
    // Wait up to 10 seconds for the thread to terminate
    for (int i = 0; i < 1000 && !dead; ++i) {
        sleep(10);
        {
            QyAutoLocker cs(&d->mStateCrit);
            dead = d->mDead;
        }
    }
    d->mFunction(d->mThreadObject,kTStoped);
    if (dead) {
        return true;
    } else {
        return false;
    }
}

void QyThreadPosix::wake()
{
    QY_D(QyThreadPosix);
    QyAutoLocker cs(&d->mStateCrit);
    d->mThreadHook.wake();
}

void QyThreadPosix::run() {
    QY_D(QyThreadPosix);
    {
        QyAutoLocker cs(&d->mStateCrit);
        d->mAlive = true;
    }
#if (defined(WEBRTC_LINUX) || defined(WEBRTC_ANDROID))
    d->mPid = currentThreadId();
#endif
    // The event the Start() is waiting for.
    d->mEvent.set();
    
    if (d->mSetThreadName) {
#ifdef WEBRTC_LINUX
        prctl(PR_SET_NAME, (unsigned long)d->name_, 0, 0, 0);
#endif
        
    }
    bool alive = true;
    while (alive) {
        int res = d->mFunction(d->mThreadObject,kTRunning);
        {
            QyAutoLocker cs(&d->mStateCrit);
            if (kTResExit == res) {
                d->mAlive = false;
            }
            else if (kTResHook == res){
                d->mThreadHook.wait(INFINITE);
            }
            alive = d->mAlive;
        }
    }
    
    {
        QyAutoLocker cs(&d->mStateCrit);
        d->mDead = true;
    }
}

QyThreadId QyThreadPosix::threadId() const
{
    QY_D(QyThreadPosix);
    return d->mThread;
}

void QyThreadPosix::sleep(int msecs)
{
    struct timespec short_wait;
    struct timespec remainder;
    short_wait.tv_sec = msecs / 1000;
    short_wait.tv_nsec = (msecs % 1000) * 1000 * 1000;
    nanosleep(&short_wait, &remainder);
}

END_NAMESPACE(qy)
