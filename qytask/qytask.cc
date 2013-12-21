#include "qytask.h"
#include "qytaskrunner.h"
#include <assert.h>
#include <algorithm>

DEFINE_NAMESPACE(qy)

int32 QyTask::sUniqueIdSeed = 0;

QyTask::QyTask(QyTask *parent)
: mParent(parent)
, mState(STATE_INIT)
, mBlocked(false)
, mDone(false)
, mAborted(false)
, mBusy(false)
, mError(false)
, mChildError(false)
, mStartTime(0)
, mTimeout(0)
, mTimeoutSeconds(0)
, mTimeoutSuspended(false)
{
    mChildren = new TaskChildSet();
    mRuuner = ((parent == NULL) ? reinterpret_cast<QyTaskRunner *>(this) : parent->runner());
    if (mParent != NULL) {
        mParent->addChild(this);
	}

    mUniqueId = sUniqueIdSeed++;

	// sanity check that we didn't roll-over our id seed
    assert(mUniqueId < sUniqueIdSeed);
}

QyTask::~QyTask()
{
    if (mParent != NULL) {
        mParent->removeChild(this);
	}
    delete mChildren;
}

int64 QyTask::currentTime() 
{
    return mRuuner->currentTime();
}

void QyTask::start()
{
    if (mState != STATE_INIT)
		return;
	// Set the start time before starting the task.  Otherwise if the task
    // finishes quickly and deletes the QyTask object, setting mStartTime
	// will crash.
    mStartTime = currentTime();
	runner()->startTask(this);
}

void QyTask::run() 
{
    if (mDone) {
		if (runAllChildren()) {
			return ;
		}
        mBlocked = true;
		return;
	}

	// Async Error() was called
    if (mError) {
        mDone  = true;
        mState = STATE_ERROR;
        mBlocked = true;
		stop();
		return;
	}

    mBusy = true;
    int new_state = process(mState);
    mBusy = false;

    if (mAborted) {
        mDone = true;
        mBlocked = true;
		stop();
		return;
	}

	if (new_state == STATE_BLOCKED) {
        mBlocked = true;
		// Let the timeout continue
	}
	else {
        mState = new_state;
        mBlocked = false;
		resetTimeout();

		if (new_state == STATE_DONE) {
            mDone = true;
		}
		else if (new_state == STATE_ERROR) {
            mDone = true;
            mError = true;
		}
	}

    if (mDone) {
		// check run child
		if (!needRunAllChildren()) {
			stop();
            mBlocked = true;
		}
		runner()->wakeTasks();
	}
}

void QyTask::abort() 
{
    if (mAborted || mDone) {
		return;
	}
    mAborted = true;
    if (!mBusy) {
        mDone = true;
        mBlocked = true;
        mError = true;
		stop();
	}
}

void QyTask::wake(int cmsDelay)
{
    if (mDone) {
		return;
	}
    if (mBlocked) {
        mBlocked = false;
		runner()->wakeTasks(cmsDelay);
	}
}

void QyTask::error() 
{
    if (mError || mDone) {
		return;
	}
    mError = true;
	wake();
}

std::string QyTask::getStateName(int state) const
{
	static const std::string STR_BLOCKED("BLOCKED");
	static const std::string STR_INIT("INIT");
	static const std::string STR_START("START");
	static const std::string STR_DONE("DONE");
	static const std::string STR_ERROR("ERROR");
	static const std::string STR_RESPONSE("RESPONSE");
	static const std::string STR_HUH("");
	switch (state) {
	case STATE_BLOCKED: return STR_BLOCKED;
	case STATE_INIT: return STR_INIT;
	case STATE_START: return STR_START;
	case STATE_DONE: return STR_DONE;
	case STATE_ERROR: return STR_ERROR;
	case STATE_RESPONSE: return STR_RESPONSE;
	}
	return STR_HUH;
}

int QyTask::process(int state) 
{
	int newstate = STATE_ERROR;
	if (timedOut()) {
		clearTimeout();
		newstate = onTimeout();
	} 
	else {
		switch (state) {
		case STATE_INIT:
			newstate = processInit();
			break;
		case STATE_START:
			newstate = processStart();
			break;
		case STATE_RESPONSE:
			newstate = processResponse();
			break;
		case STATE_DONE:
		case STATE_ERROR:
			newstate = STATE_BLOCKED;
			break;
		}
	}
	return newstate;
}

void QyTask::addChild(QyTask *child) 
{
    QyAutoLocker cs(&mChildCrit);
    mChildren->insert(child);
}

void QyTask::removeChild(QyTask *child)
{
    QyAutoLocker cs(&mChildCrit);
    mChildren->erase(child);
}

bool QyTask::allChildrenDone()
{
    QyAutoLocker cs(&mChildCrit);
    for (TaskChildSet::const_iterator it = mChildren->begin();it != mChildren->end();	++it) {
		if (!(*it)->isDone()) 
			return false;
	}
	return true;
}

bool QyTask::anyChildError() const
{
    return mChildError;
}

void QyTask::abortAllChildren()
{
    QyAutoLocker cs(&mChildCrit);
    if (mChildren->size() > 0) {
        TaskChildSet copy = *mChildren;
		for (TaskChildSet::iterator it = copy.begin(); it != copy.end(); ++it) {
			(*it)->abort();  // Note we do not wake
		}
	}
}

bool QyTask::needRunAllChildren()
{
	if (isError()) {
		return false;
	}
	if (isAbort()){
		return false;
	}
	if (allChildrenDone()) {
		return false;
	}
	return true;
}

bool QyTask::runAllChildren()
{
	if (!needRunAllChildren()) {
		return false;
	}

    QyAutoLocker cs(&mChildCrit);
	bool did = true;
	while(did) {
		did = false;
        for (TaskChildSet::iterator it = mChildren->begin(); it != mChildren->end(); ++it) {
			if (!(*it)->blocked()) {
				(*it)->run();
				did = true;
			}
		}
	}
	return false;
}

void QyTask::stop()
{
	// No need to wake because we're either awake or in abort
	abortAllChildren();

    if (mParent) {
		if (hasError()) {
            mParent->mChildError = true;
		}
	}
	if (this->isChild2()) {
		runner()->readyDelete(this);
	}
}

bool QyTask::isChild2() const
{
    return (mParent && mParent != static_cast<QyTask*>(mRuuner));
}

void QyTask::set_timeout_seconds(const int timeout_seconds)
{
    mTimeoutSeconds = timeout_seconds;
	resetTimeout();
}

bool QyTask::timedOut() 
{
    return mTimeoutSeconds &&	mTimeout && currentTime() > mTimeout;
}

void QyTask::resetTimeout() 
{
    bool timeout_allowed = (mState != STATE_INIT)
        && (mState != STATE_DONE)
        && (mState != STATE_ERROR);
    if (mTimeoutSeconds && timeout_allowed && !mTimeoutSuspended) {
        mTimeout = currentTime() +
            (mTimeoutSeconds * kSecToMsec * kMsecTo100ns);
	}
	else {
        mTimeout = 0;
	}
	runner()->updateTaskTimeout(this);
}

void QyTask::clearTimeout() 
{
    mTimeout = 0;
	runner()->updateTaskTimeout(this);
}

void QyTask::suspendTimeout()
{
    if (!mTimeoutSuspended) {
        mTimeoutSuspended = true;
		resetTimeout();
	}
}

void QyTask::resumeTimeout() 
{
    if (mTimeoutSuspended) {
        mTimeoutSuspended = false;
		resetTimeout();
	}
}

END_NAMESPACE(qy)
