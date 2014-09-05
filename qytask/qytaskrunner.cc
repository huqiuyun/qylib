
#include "qytaskrunner.h"
#include "qytask.h"
#include "qylock.h"
#include "qytime.h"
#include <list>
#include <algorithm>

DEFINE_NAMESPACE(qy)

typedef std::set<QyTask *> TaskSet;
typedef std::list<QyTask*> TaskList;
class QyTaskRunnerPrivate
{
public:
    QyTaskRunnerPrivate() :
        mNextTimeoutTask(NULL),
        mTaskRunning(false)
	{
	}
    TaskSet  mDelTasks;
    QySLCS   mDelCrit;
    QySLCS   mTaskCrit;
    TaskList mTasks;
    QyTask  *mNextTimeoutTask;
    bool     mTaskRunning;
};

QyTaskRunner::QyTaskRunner()
: QyTask(NULL)
, d_ptr(new QyTaskRunnerPrivate())
{
}

QyTaskRunner::~QyTaskRunner()
{
	// this kills and deletes children silently!
	abortAllChildren();
	runTasks();

    delete d_ptr;
}

int64 QyTaskRunner::currentTime()
{
	return qytime_get();
}

int QyTaskRunner::startTask(QyTask * task)
{
	QY_D(QyTaskRunner);
	{
        QyAutoLocker cs(&d->mTaskCrit);
        d->mTasks.push_back(task);
	}
	// the task we just started could be about to timeout --
	// make sure our "next timeout task" is correct
	updateTaskTimeout(task);
	
	wakeTasks();

	return 0;
}

void QyTaskRunner::runTasks()
{
	QY_D(QyTaskRunner);

	// Running continues until all tasks are Blocked (ok for a small # of tasks)
    if (d->mTaskRunning)
		return;  // don't reenter

    d->mTaskRunning = true;
	// push temp task list,to do prevent lockdead
	TaskList runtask;
	{		
        QyAutoLocker cs(&d->mTaskCrit);
        runtask = d->mTasks;
	}
	
	bool did = true;
    while (did) {
		did = false;
        // use indexing instead of iterators because mTasks may grow
		TaskList::iterator it = runtask.begin();
        while (it != runtask.end()) {
			QyTask* task = *it;
            while (!task->blocked()) {
				task->run();
				did = true;
			}
			++it;
		}
	}

	endTasks();

    d->mTaskRunning = false;

    delTasks();
}

void QyTaskRunner::endTasks()
{// Tasks are deleted when running has paused

	QY_D(QyTaskRunner);
	bool need_timeout_recalc = false;
	{// for lockdead
        QyAutoLocker cs(&d->mTaskCrit);
        TaskList::iterator it = d->mTasks.begin();
        while (it != d->mTasks.end()) {
			QyTask* task = *it;
            if (task->isDone() && task->allChildrenDone()) {
                if (d->mNextTimeoutTask &&
                    d->mNextTimeoutTask->get_unique_id() == task->get_unique_id()) {
                    d->mNextTimeoutTask = NULL;
					need_timeout_recalc = true;
				}
				delete task;
                it = d->mTasks.erase(it);
			}
            else {
				++it;
			}
		}
	}

    if (need_timeout_recalc) {
		recalcNextTimeout(NULL);
    }
}

void QyTaskRunner::readyDelete(QyTask* task)
{
	QY_D(QyTaskRunner);

    QyAutoLocker cs(&d->mDelCrit);
    d->mDelTasks.insert(task);
}

void QyTaskRunner::delTasks()
{
	QY_D(QyTaskRunner);

    QyAutoLocker cs(&d->mDelCrit);
    if (d->mDelTasks.size() > 0) {
        for (TaskSet::iterator it = d->mDelTasks.begin(); it != d->mDelTasks.end(); ++it){
            delete (*it);
		}
        d->mDelTasks.clear();
    }
}

void QyTaskRunner::pollTasks()
{
	QY_D(QyTaskRunner);

	// see if our "next potentially timed-out task" has indeed timed out.
	// If it has, wake it up, then queue up the next task in line
    if (d->mNextTimeoutTask && d->mNextTimeoutTask->timedOut()) {
        d->mNextTimeoutTask->wake();
		wakeTasks();
	}
}

// this function gets called frequently -- when each task changes
// state to something other than DONE, ERROR or BLOCKED, it calls
// ResetTimeout(), which will call this function to make sure that
// the next timeout-able task hasn't changed.  The logic in this function
// prevents RecalcNextTimeout() from getting called in most cases,
// effectively making the task scheduler O-1 instead of O-N

void QyTaskRunner::updateTaskTimeout(QyTask *task) 
{
	QY_D(QyTaskRunner);

	// if the relevant task has a timeout, then
	// check to see if it's closer than the current
	// "about to timeout" task
	if (task->get_timeout_time()) {
        if (d->mNextTimeoutTask == NULL ||
            (task->get_timeout_time() <= d->mNextTimeoutTask->get_timeout_time())) {
            d->mNextTimeoutTask = task;
		}
	}
    else if (d->mNextTimeoutTask != NULL &&
        task->get_unique_id() == d->mNextTimeoutTask->get_unique_id()) {
		// otherwise, if the task doesn't have a timeout,
		// and it used to be our "about to timeout" task,
		// walk through all the tasks looking for the real
		// "about to timeout" task
		recalcNextTimeout(task);
	}
}

void QyTaskRunner::recalcNextTimeout(QyTask *exclude_task) 
{
	// walk through all the tasks looking for the one
	// which satisfies the following:
	//   it's not finished already
	//   we're not excluding it
	//   it has the closest timeout time
	QY_D(QyTaskRunner);

	int64 next_timeout_time = 0;
    d->mNextTimeoutTask = NULL;

//	QyAutoLocker cs(&d->mTaskCrit);
    TaskList::iterator it = d->mTasks.begin();
    while (it != d->mTasks.end()) {
		QyTask *task = *it;
		// if the task isn't complete, and it actually has a timeout time
        if (!task->isDone() && (task->get_timeout_time() > 0)) {
			// if it doesn't match our "exclude" task
			if (exclude_task == NULL ||	
                exclude_task->get_unique_id() != task->get_unique_id()) {
				// if its timeout time is sooner than our current timeout time
				if (next_timeout_time == 0 ||
					task->get_timeout_time() <= next_timeout_time) {
					// set this task as our next-to-timeout
					next_timeout_time  = task->get_timeout_time();
                    d->mNextTimeoutTask = task;
				}
			}
		}
		++it;
	}
}

END_NAMESPACE(qy)
