#ifndef QY_TASKRUNNER_H__
#define QY_TASKRUNNER_H__

#include "qytask-config.h"
#include "qytask.h"

DEFINE_NAMESPACE(qy)

const int64 kSecToMsec = 1000;
const int64 kMsecTo100ns = 10000;
const int64 kSecTo100ns = kSecToMsec * kMsecTo100ns;

class QyTaskRunnerPrivate;
class QYTASK_API QyTaskRunner : public QyTask
{
public:
	QyTaskRunner();
	virtual ~QyTaskRunner();

	int64 currentTime();
    int  startTask(QyTask *task);
	void runTasks();
    void pollTasks();
	void updateTaskTimeout(QyTask *task);
    void readyDelete(QyTask *task);
public:
	virtual void wakeTasks(int cmsDelay = 0) = 0;

	// dummy state machine - never run.
	virtual int processStart() { return STATE_DONE; }
private:
	void endTasks();
    void delTasks();
	void recalcNextTimeout(QyTask *task);
    void run(){}
private:
    QyTaskRunnerPrivate* d_ptr;
};

END_NAMESPACE(qy)

#endif  // QY_TASKRUNNER_H__
