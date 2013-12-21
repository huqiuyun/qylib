#ifndef QY_TASK_H__
#define QY_TASK_H__

#include "qytask-config.h"
#include "qyctypes.h"
#include "qylock.h"
#include <string>
#include <set>

DEFINE_NAMESPACE(qy)

class QyTaskRunner;
class QyTask;
typedef std::set<QyTask *> TaskChildSet;

class QYTASK_API QyTask
{
public:
	QyTask(QyTask *parent);
	virtual ~QyTask();

public:
    QyTask *parent() { return mParent; }
    QyTaskRunner *runner() { return mRuuner; }

    int32 get_unique_id() { return mUniqueId; }

    int  getState() const { return mState; }
	bool hasError() const { return (getState() == STATE_ERROR); }
    bool blocked()  const { return mBlocked; }
    bool isDone()   const { return mDone; }
    bool isError()  const { return mError;}
    bool isAbort()  const { return mAborted;}
    bool isChild2() const;

	// For managing children
	bool allChildrenDone();
	bool anyChildError() const;

	bool timedOut();

    int64 get_timeout_time() { return mTimeout; }
	void  set_timeout_seconds(int timeout_seconds);

    /** (CurrentTime() - mStartTime;) */
    int64 elapsedTime() { return currentTime() - mStartTime; }

    /** runner()->startTask(this) */
	void start();

	// Called from outside to stop task without any more callbacks
	void abort();

	// Called inside the task to signal that the task may be unblocked
	void wake(int cmsDelay = 0);
    virtual void run();
protected:
	enum
	{
		STATE_BLOCKED = -1,
		STATE_INIT = 0,
		STATE_START = 1,
		STATE_DONE = 2,
		STATE_ERROR = 3,
		STATE_RESPONSE = 4,
        STATE_NEXT = 5 // Subclasses which need more states start here and higher
	};

	// Called inside to advise that the task should wake and signal an error
	void error();

	int64 currentTime();

	// for managing children (if any)
	void addChild(QyTask *child);
	void removeChild(QyTask *child);
	void abortAllChildren();

	void resetTimeout();
	void clearTimeout();

	void suspendTimeout();
	void resumeTimeout();

	virtual std::string getStateName(int state) const;
	virtual int  process(int state);
	virtual void stop();
	virtual int  processInit() {  return STATE_START; }
	virtual int  processStart() = 0;
	virtual int  processResponse() { return STATE_DONE; }
protected:

	// by default, we are finished after timing out
	virtual int onTimeout() { return STATE_DONE;}

private:
	void done();
    bool runAllChildren();
	bool needRunAllChildren();

protected:
    QyTask *mParent;
    QyTaskRunner *mRuuner;

    int   mState;
    bool  mBlocked;
    bool  mDone;
    bool  mAborted;
    bool  mBusy;
    bool  mError;
    bool  mChildError;
    int64 mStartTime;
    int64 mTimeout;
    int   mTimeoutSeconds;
    bool  mTimeoutSuspended;
    int32 mUniqueId;
    static int32 sUniqueIdSeed;

	// for managing children
    TaskChildSet* mChildren;
    QySLCS  mChildCrit;
};

END_NAMESPACE(qy)

#endif  // QY_TASK_H__
