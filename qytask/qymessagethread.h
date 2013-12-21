#ifndef QY_MESSAGETHREAD_H__
#define QY_MESSAGETHREAD_H__

#include "qytask-config.h"
#include "qymessagequeue.h"
#include "qythread.h"

DEFINE_NAMESPACE(qy)

class QyWake;
class QyMessageThread;
class QyMessageThreadManagerPrivate;

class QYTASK_API QyMessageThreadManager
{
public:
	QyMessageThreadManager();
	~QyMessageThreadManager();

public:
	void add(QyMessageThread *thread);
	void remove(QyMessageThread *thread);

	static QyMessageThread *currentThread();
	static void setCurrent(QyMessageThread *thread);
private:
    QyMessageThreadManagerPrivate* d_ptr;
};

class QyMessageThreadPrivate;
class QYTASK_API QyMessageThread : public QyMessageQueue
{
public:
    QyMessageThread(QyWake* wake,QyMsgQueueManager* p);
	virtual ~QyMessageThread();
public:
	static inline QyMessageThread* current(){
		return QyMessageThreadManager::currentThread();
	}
	// QyThreadRunFunction
    static int preRun(QyThreadObj obj,ThreadEvent e);

	inline bool isCurrent() const{
		return (QyMessageThreadManager::currentThread() == this);
	}
	virtual void start();
	virtual void stop();
    virtual void send(QyMessageHandler *phandler, uint32 id = 0,QyMessageData *pdata = NULL);
	virtual void clear(QyMessageHandler *phandler, uint32 id = MQID_ANY);
protected:
    virtual int  run(ThreadEvent e);

	virtual void onSendMessage();
    virtual void wake();
	virtual void wait(int cms, bool threadrun);
private:
    QyMessageThreadPrivate* d_ptr;
	friend class QyMessageThreadManager;
};

END_NAMESPACE(qy)

#endif // QY_MESSAGETHREAD_H__
