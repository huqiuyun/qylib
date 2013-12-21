#ifndef QY_HTREAD_H__
#define QY_HTREAD_H__

#include "qytask-config.h"

DEFINE_NAMESPACE(qy)

enum ThreadEvent
{
    kTRunning = 1, ///< runing ...
    kTStoped = 2 ///< call stop
};

enum ThreadResult
{
    kTResNone = 1,
    kTResExit = 2,
    kTResHook = 3 ///< thread hook,and Need to call QyThread::wakeUp()
};

// Object that will be passed by the spawned thread when it enters the callback
// function.
#define QyThreadObj void*

// Callback function that the spawned thread will enter once spawned.
// A return value of false is interpreted as that the function has no
// more work to do and that the thread can be released.
typedef int (*QyThreadRunFunction)(QyThreadObj, ThreadEvent e);

enum ThreadPriority {
    kLowPriority = 1,
    kNormalPriority = 2,
    kHighPriority = 3,
    kHighestPriority = 4,
    kRealtimePriority = 5
};

#if defined(H_OS_WIN)
typedef unsigned long QyTheadKey;
typedef unsigned int  QyThreadId;
#else
#include <pthread.h>
typedef pthread_key_t QyThreadKey;
typedef pthread_t     QyThreadId;
#endif

const int kThreadMaxNameLength = 64;

class QYTASK_API QyThread
{
public:
	QyThread(){}
    virtual ~QyThread() {}
    
    // Factory method. Constructor disabled.
    //
    // func        Pointer to a, by user, specified callback function.
    // obj         Object associated with the thread. Passed in the callback
    //             function.
    // prio        Thread priority. May require root/admin rights.
    // thread_name  NULL terminated thread name, will be visable in the Windows
    //             debugger.
    static QyThread* createThread(QyThreadRunFunction func,
                                  QyThreadObj obj, ThreadPriority prio = kNormalPriority,
                                  const char* thread_name = 0);
    
    // Get the current thread's kernel thread ID.
    static QyThreadId currentThreadId();
    
    // Non blocking termination of the spawned thread. Note that it is not safe
    // to delete this class until the spawned thread has been reclaimed.
    virtual void setNotAlive() = 0;
    
    // Tries to spawns a thread and returns true if that was successful.
    // Additionally, it tries to set thread priority according to the priority
    // from when CreateThread was called. However, failure to set priority will
    // not result in a false return value.
    // TODO(henrike): add a function for polling whether priority was set or
    //                not.
    virtual bool start() = 0;
    
    // Stops the spawned thread and waits for it to be reclaimed with a timeout
    // of two seconds. Will return false if the thread was not reclaimed.
    // Multiple tries to Stop are allowed (e.g. to wait longer than 2 seconds).
    // It's ok to call Stop() even if the spawned thread has been reclaimed.
    virtual bool stop(unsigned long msec = 2000) = 0;
    
    virtual void wake() = 0;
    // Sets the threads CPU affinity. CPUs are listed 0 - (number of CPUs - 1).
    // The numbers in processor_numbers specify which CPUs are allowed to run the
    // thread. processor_numbers should not contain any duplicates and elements
    // should be lower than (number of CPUs - 1). amount_of_processors should be
    // equal to the number of processors listed in processor_numbers.
    virtual bool setAffinity(const int* processor_numbers,
                             const unsigned int amount_of_processors) {
								 QY_UNUSED(processor_numbers);
								 QY_UNUSED(amount_of_processors);
        return false;
    }
    
    virtual QyThreadId threadId() const = 0;

    virtual void sleep(int msecs) = 0;
};

END_NAMESPACE(qy)

#endif  // QY_HTREAD_H__
