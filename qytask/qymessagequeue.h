#ifndef QY_MESSAGEQUEUE_H__
#define QY_MESSAGEQUEUE_H__

#include "qytask-config.h"
#include "qymessagedata.h"
#include "qylock.h"
#include "qytime.h"

#include <queue>

DEFINE_NAMESPACE(qy)

const uint32 MQID_ANY = static_cast<uint32>(-1);
const uint32 MQID_DISPOSE = static_cast<uint32>(-2);

class  QyMessage;
class  QyMessageQueue;
class  QyMessageHandler;

// QyMsgQueueManager does cleanup of of message queues
class QyMsgQueueManagerPrivate;
class QYTASK_API QyMsgQueueManager
{
public:
    QyMsgQueueManager();
    ~QyMsgQueueManager();
public:
	void add(QyMessageQueue *message_queue);
    void remove(QyMessageQueue *message_queue);
	void clear(QyMessageHandler *handler);

private:
    QyMsgQueueManagerPrivate* d_ptr;
};

// Messages get dispatched to a QyMessageHandler
class QYTASK_API QyMessageHandler
{
public:
    QyMessageHandler(QyMsgQueueManager* p = NULL)
        :mMsgQueueMgr(p)
	{
	}
	virtual ~QyMessageHandler()
	{
        if( mMsgQueueMgr )
            mMsgQueueMgr->clear(this);
    }
	virtual void onMessage(QyMessage *pmsg) = 0;

protected:
    QyMsgQueueManager* mMsgQueueMgr;
};

class QYTASK_API QyMessage
{
public:
	QyMessage()
	{
        msgid = 0;
		data = NULL;
		handler = NULL;
	}		
    uint32 msgid;
    QyMessageData *data;
	QyMessageHandler *handler;
};

// QyDelayedMessage goes into a priority queue, sorted by trigger time
class QYTASK_API QyDelayedMessage
{
public:
	QyDelayedMessage(int cmsDelay, QyMessage *pmsg) 
	{
        mCmsDelay = cmsDelay;
        mMsTrigger = qy::GetMillisecondCount() + cmsDelay;
        mMsg = *pmsg;
	}

	bool operator< (const QyDelayedMessage& dmsg) const
	{
        return dmsg.mMsTrigger < mMsTrigger;
	}

    int mCmsDelay; // for debugging
    uint32 mMsTrigger;
    QyMessage mMsg;
};

class QYTASK_API QyMessageQueue
{
public:
    QyMessageQueue(QyMsgQueueManager* p);
	virtual ~QyMessageQueue();

	// Note: The behavior of QyMessageQueue has changed.  When a MQ is stopped,
	// futher Posts and Sends will fail.  However, any pending Sends and *ready*
	// Posts (as opposed to unexpired delayed Posts) will be delivered before
	// Get (or Peek) returns false.  By guaranteeing delivery of those messages,
	// we eliminate the race condition when an QyMessageHandler and QyMessageQueue
	// may be destroyed independently of each other.

	virtual void stop();
	virtual bool isStopping();
	virtual void start();

	virtual bool get(QyMessage *msg, unsigned long cmsWait = kForever);
	virtual bool peek(QyMessage *msg, unsigned long cmsWait = 0);

    virtual void send(QyMessageHandler *handler, uint32 id = 0,QyMessageData *data = NULL) {
        QY_UNUSED(handler);
        QY_UNUSED(id);
        QY_UNUSED(data);
    }
    virtual void post(QyMessageHandler *handler, uint32 id = 0,QyMessageData *data = NULL);
    virtual void postDelayed(int cmsDelay, QyMessageHandler *handler,uint32 id = 0, QyMessageData *data = NULL);
	virtual void clear(QyMessageHandler *handler, uint32 id = MQID_ANY);
	virtual void dispatch(QyMessage *msg);
	virtual int  delay();

protected:
    virtual void wake() {}
    virtual void wait(int cms,bool threadrun) {
        QY_UNUSED(cms);
        QY_UNUSED(threadrun);
    }
    virtual void onSendMessage() {}

protected:
    void ensureActive();

protected:
    QyMsgQueueManager* mMsgQueueMgr;
    bool mStop;
    bool mPeekKeep;
    bool mActive;
    QyMessage mMsgPeek;
    std::queue<QyMessage> mMsgQueue;
    std::priority_queue<QyDelayedMessage> mDelayMsgQueue;
    QySLCS mCrit;
};

END_NAMESPACE(qy)

#endif // QY_MESSAGEQUEUE_H__
