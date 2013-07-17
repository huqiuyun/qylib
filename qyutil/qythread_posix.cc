/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// The state of a thread is controlled by the two member variables
// alive_ and dead_.
// alive_ represents the state the thread has been ordered to achieve.
// It is set to true by the thread at startup, and is set to false by
// other threads, using SetNotAlive() and Stop().
// dead_ represents the state the thread has achieved.
// It is written by the thread encapsulated by this class only
// (except at init). It is read only by the Stop() method.
// The Run() method fires event_ when it's started; this ensures that the
// Start() method does not continue until after dead_ is false.
// This protects against premature Stop() calls from the creator thread, but
// not from other threads.

// Their transitions and states:
// alive_    dead_  Set by
// false     true   Constructor
// true      false  Run() method entry
// false     any    Run() method run_function failure
// any       false  Run() method exit (happens only with alive_ false)
// false     any    SetNotAlive
// false     any    Stop         Stop waits for dead_ to become true.
//
// Summarized a different way:
// Variable   Writer               Reader
// alive_     Constructor(false)   Run.loop
//            Run.start(true)
//            Run.fail(false)
//            SetNotAlive(false)
//            Stop(false)
//
// dead_      Constructor(true)    Stop.loop
//            Run.start(false)
//            Run.exit(true)

#include "qythread_posix.h"
#include "qylock.h"
#include "qyos.h"
#include <algorithm>

#include <assert.h>
#include <errno.h>
#include <string.h>  // strncpy
#include <unistd.h>

DEFINE_NAMESPACE(qy)

int convertToSystemPriority(eThreadPriority priority, int min_prio,
                            int max_prio) {
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

extern "C"
{
    static void* startThread(void* lp_parameter) {
        static_cast<QyThreadPosix*>(lp_parameter)->run();
        return 0;
    }
}

class QyThreadPosixPrivate
{
public:
    QyThreadPosixPrivate(QyThreadRunFunction func, QyThreadObj obj,
                         eThreadPriority prio, const char* thread_name)
    : run_function_(func)
    , obj_(obj)
    , alive_(false)
    , dead_(true)
    , prio_(prio)
    , name_()
    , set_thread_name_(false)
#if (defined(WEBRTC_LINUX) || defined(WEBRTC_ANDROID))
    , pid_(-1)
#endif
    , attr_()
    , thread_(0) {
        if (thread_name != NULL) {
            set_thread_name_ = true;
            strncpy(name_, thread_name, kThreadMaxNameLength);
            name_[kThreadMaxNameLength - 1] = '\0';
        }
    }
    
    ~QyThreadPosixPrivate()
    {
        pthread_attr_destroy(&attr_);
    }
    
    QyThreadRunFunction   run_function_;
    QyThreadObj           obj_;
    
    // Internal state.
    QySmartLock<QY_CS>    crit_state_;  // Protects alive_ and dead_
    QySignalEvent         event_;
    QySignalEvent         thread_hook_;
    
    bool                  alive_;
    bool                  dead_;
    eThreadPriority       prio_;
    
    // Zero-terminated thread name string.
    char                  name_[kThreadMaxNameLength];
    bool                  set_thread_name_;
    
    // Handle to thread.
#if (defined(WEBRTC_LINUX) || defined(WEBRTC_ANDROID))
    pid_t                 pid_;
#endif
    pthread_attr_t        attr_;
    pthread_t             thread_;
};

QyThread* QyThreadPosix::create(QyThreadRunFunction func, QyThreadObj obj,
                                eThreadPriority prio,
                                const char* thread_name) {
    QyThreadPosix* ptr = new QyThreadPosix(func, obj, prio, thread_name);
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
                             eThreadPriority prio, const char* thread_name)
: d_ptr_(new QyThreadPosixPrivate(func,obj,prio,thread_name))
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
    result = pthread_attr_init(&d->attr_);
    if (result != 0) {
        return -1;
    }
    return 0;
}

QyThreadPosix::~QyThreadPosix() {
    delete d_ptr_;
}

#define HAS_THREAD_ID !defined(WEBRTC_IOS) && !defined(WEBRTC_MAC)

bool QyThreadPosix::start()
{
    QY_D(QyThreadPosix);
    int result = pthread_attr_setdetachstate(&d->attr_, PTHREAD_CREATE_DETACHED);
    // Set the stack stack size to 1M.
    result |= pthread_attr_setstacksize(&d->attr_, 1024 * 1024);
#ifdef WEBRTC_THREAD_RR
    const int policy = SCHED_RR;
#else
    const int policy = SCHED_FIFO;
#endif
    d->event_.reset();
    // If pthread_create was successful, a thread was created and is running.
    // Don't return false if it was successful since if there are any other
    // failures the state will be: thread was started but not configured as
    // asked for. However, the caller of this API will assume that a false
    // return value means that the thread never started.
    result |= pthread_create(&d->thread_, &d->attr_, &startThread, this);
    if (result != 0) {
        return false;
    }
    {
        QyAutoLocker cs(&d->crit_state_);
        d->dead_ = false;
    }
    
    // Wait up to 10 seconds for the OS to call the callback function. Prevents
    // race condition if Stop() is called too quickly after start.
    if (kEventOk != d->event_.wait(10)) {
        
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
    param.sched_priority = convertToSystemPriority(d->prio_, min_prio, max_prio);
    result = pthread_setschedparam(d->thread_, policy, &param);
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
                               d->pid_,
                               sizeof(mask),
                               &mask);
#else
    // "Normal" Linux.
    const int result = sched_setaffinity(d->pid_,
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
// /usr/include/mach/thread_policy.h: thread_policy_set and mach_thread_self()
// instead of Linux gettid() syscall.
bool QyThreadPosix::setAffinity(const int* , const unsigned int) {
    return false;
}
#endif

void QyThreadPosix::setNotAlive() {
    QY_D(QyThreadPosix);
    QyAutoLocker cs(&d->crit_state_);
    d->alive_ = false;
}

bool QyThreadPosix::stop(int msec) {
    QY_D(QyThreadPosix);
    QY_UNUSED(msec);
    bool dead = false;
    {
        QyAutoLocker cs(&d->crit_state_);
        d->alive_ = false;
        d->thread_hook_.waitUp();
        dead = d->dead_;
    }
    
    // TODO(hellner) why not use an event here?
    // Wait up to 10 seconds for the thread to terminate
    for (int i = 0; i < 1000 && !dead; ++i) {
        qyos_sleep(10);
        {
            QyAutoLocker cs(&d->crit_state_);
            dead = d->dead_;
        }
    }
    d->run_function_(d->obj_,kTStoped);
    if (dead) {
        return true;
    } else {
        return false;
    }
}

void QyThreadPosix::waitUp()
{
    QY_D(QyThreadPosix);
    QyAutoLocker cs(&d->crit_state_);
    d->thread_hook_.waitUp();
}

void QyThreadPosix::run() {
    QY_D(QyThreadPosix);
    {
        QyAutoLocker cs(&d->crit_state_);
        d->alive_ = true;
    }
#if (defined(WEBRTC_LINUX) || defined(WEBRTC_ANDROID))
    d->pid_ = currentThreadId();
#endif
    // The event the Start() is waiting for.
    d->event_.set();
    
    if (d->set_thread_name_) {
#ifdef WEBRTC_LINUX
        prctl(PR_SET_NAME, (unsigned long)d->name_, 0, 0, 0);
#endif
        
    }
    bool alive = true;
    while (alive) {
        int res = d->run_function_(d->obj_,kTRunning);
        {
            QyAutoLocker cs(&d->crit_state_);
            if (kTResExit == res) {
                d->alive_ = false;
            }
            else if (kTResHook == res){
                d->thread_hook_.wait(INFINITE);
            }
            alive = d->alive_;
        }
    }
    
    {
        QyAutoLocker cs(&d->crit_state_);
        d->dead_ = true;
    }
}

QyThreadId QyThreadPosix::threadId() const
{
    QY_D(QyThreadPosix);
    return d->thread_;
}

END_NAMESPACE(qy)
