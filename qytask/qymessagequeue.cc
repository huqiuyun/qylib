#include "qymessagequeue.h"
#include <algorithm>
#include <vector>

DEFINE_NAMESPACE(qy)

class QyMsgQueueManagerPrivate
{
public:
    // This list contains 'active' MessageQueues.
    std::vector<QyMessageQueue*> mMsgQueues;
    QySLCS mCrit;
};

QyMsgQueueManager::QyMsgQueueManager() :
    d_ptr(new QyMsgQueueManagerPrivate())
{
}

QyMsgQueueManager::~QyMsgQueueManager()
{
    delete d_ptr;
}

void QyMsgQueueManager::add(QyMessageQueue *message_queue)
{
    QyAutoLocker cs(&d_ptr->mCrit);
    d_ptr->mMsgQueues.push_back(message_queue);
}

void QyMsgQueueManager::remove(QyMessageQueue *message_queue)
{
    QyAutoLocker cs(&d_ptr->mCrit);
    std::vector<QyMessageQueue *>::iterator iter =
            std::find(d_ptr->mMsgQueues.begin(), d_ptr->mMsgQueues.end(), message_queue);
    if (iter != d_ptr->mMsgQueues.end())
        d_ptr->mMsgQueues.erase(iter);
}

void QyMsgQueueManager::clear(QyMessageHandler *handler) {
    QyAutoLocker cs(&d_ptr->mCrit);
    std::vector<QyMessageQueue *>::iterator iter = d_ptr->mMsgQueues.begin();
    for (; iter != d_ptr->mMsgQueues.end(); iter++)
		(*iter)->clear(handler);
}

//------------------------------------------------------------------//
// QyMessageQueue

QyMessageQueue::QyMessageQueue(QyMsgQueueManager* p) :
    mMsgQueueMgr(p),
    mStop(false),
    mPeekKeep(false),
    mActive(false)
{
}

QyMessageQueue::~QyMessageQueue()
{
    if (mActive && mMsgQueueMgr) {
        mMsgQueueMgr->remove(this);
	}
	clear(NULL);
}

void QyMessageQueue::stop()
{
    mStop = true;
    wake();
}

bool QyMessageQueue::isStopping() {
    return mStop;
}

void QyMessageQueue::start() {
    mStop = false;
}

bool QyMessageQueue::peek(QyMessage *msg, unsigned long cmsWait)
{
    if (mPeekKeep) {
        *msg = mMsgPeek;
		return true;
	}
	if (!get(msg, cmsWait)){
		return false;
    }
    mMsgPeek = *msg;
    mPeekKeep = true;
	return true;
}

bool QyMessageQueue::get(QyMessage *msg, unsigned long cmsWait)
{
	// Return and clear peek if present
	// Always return the peek if it exists so there is Peek/Get symmetry

    if (mPeekKeep) {
        *msg = mMsgPeek;
        mPeekKeep = false;
		return true;
	}

	uint32 cmsTotal = cmsWait;
	uint32 cmsElapsed = 0;
	uint32 msStart = qytime_get();
	uint32 msCurrent = msStart;
	while (true) {
		// Check for sent messages
		onSendMessage();

		// Check queues
        uint32 cmsDelayNext = kForever;
		{
            QyAutoLocker cs(&mCrit);

			// Check for delayed messages that have been triggered
			// Calc the next trigger too
            while (!mDelayMsgQueue.empty()) {
                if (msCurrent < mDelayMsgQueue.top().mMsTrigger) {
                    cmsDelayNext = mDelayMsgQueue.top().mMsTrigger - msCurrent;
					break;
				}
                mMsgQueue.push(mDelayMsgQueue.top().mMsg);
                mDelayMsgQueue.pop();
			}

			// Check for posted events
            while (!mMsgQueue.empty()) {
                *msg = mMsgQueue.front();
                mMsgQueue.pop();
                if (MQID_DISPOSE == msg->msgid) {
					qyDelete(msg->data);
					continue;
				}
				return true;
			}
        }// for mCrit lock

        if (mStop){
			break;
        }

		// Which is shorter, the delay wait or the asked wait?
        int cmsNext;
		if (cmsWait == kForever) {
			cmsNext = cmsDelayNext;
		}
		else {
			cmsNext = cmsTotal - cmsElapsed;
			if (cmsNext < 0)
				cmsNext = 0;
			if (cmsDelayNext != kForever &&
                cmsDelayNext < (uint32)cmsNext){
				cmsNext = cmsDelayNext;
            }
		}

		// Wait and multiplex in the meantime(等待 : 直到有信号或超时为止)
		wait(cmsNext, true);

		// If the specified timeout expired, return
		msCurrent = qytime_get();
		cmsElapsed = msCurrent - msStart;
		if (cmsWait != kForever) {
			if (cmsElapsed >= cmsWait)
				return false;
		}
	}
	return false;
}

void QyMessageQueue::post(QyMessageHandler *handler, uint32 id,QyMessageData *data)
{
    if (mStop)
		return;

	// Keep thread safe
	// Add the message to the end of the queue
	// Signal for the multiplexer to return

    QyAutoLocker cs(&mCrit);
	ensureActive();
	QyMessage msg;
	msg.handler = handler;
    msg.msgid = id;
	msg.data = data;
    mMsgQueue.push(msg);
    wake();
}

void QyMessageQueue::postDelayed(int cmsDelay, QyMessageHandler *handler, uint32 id, QyMessageData *data)
{
    if (mStop)
		return;

	// Keep thread safe
	// Add to the priority queue. Gets sorted soonest first.
	// Signal for the multiplexer to return.

    QyAutoLocker cs(&mCrit);
	ensureActive();
	QyMessage msg;
	msg.handler = handler;
    msg.msgid = id;
	msg.data = data;
    mDelayMsgQueue.push(QyDelayedMessage(cmsDelay, &msg));
    wake();
}

int QyMessageQueue::delay()
{
    QyAutoLocker cs(&mCrit);

    if (!mMsgQueue.empty())
		return 0;

    if (!mDelayMsgQueue.empty())
	{
        int delay = mDelayMsgQueue.top().mMsTrigger - qytime_get();
		if (delay < 0)
			delay = 0;
		return delay;
	}
	return kForever;
}

void QyMessageQueue::clear(QyMessageHandler *handler, uint32 id)
{
    QyAutoLocker cs(&mCrit);

	// Remove messages with handler
    if (mPeekKeep) {
		if (handler == NULL ||
            mMsgPeek.handler == handler) {
			if (id == MQID_ANY ||
                mMsgPeek.msgid == id) {
                qyDelete(mMsgPeek.data);
                mPeekKeep = false;
			}
		}
	}

	// Remove from ordered message queue
    size_t c = mMsgQueue.size();
	while (c-- != 0) 
	{
        QyMessage msg = mMsgQueue.front();
        mMsgQueue.pop();
		if (handler != NULL &&
            msg.handler != handler) {
            mMsgQueue.push(msg);
		} 
		else{
			if (id == MQID_ANY ||
                msg.msgid == id) {
				qyDelete(msg.data);
			}
			else {
                mMsgQueue.push(msg);
			}
		}
	}

	// Remove from priority queue. Not directly iterable, so use this approach

	std::queue<QyDelayedMessage> dmsgs;
    while (!mDelayMsgQueue.empty())
	{
        QyDelayedMessage dmsg = mDelayMsgQueue.top();
        mDelayMsgQueue.pop();
		if (handler != NULL &&
            dmsg.mMsg.handler != handler) {
			dmsgs.push(dmsg);
		} 
		else {
			if (id == MQID_ANY ||
                dmsg.mMsg.msgid == id) {
                qyDelete(dmsg.mMsg.data);
			} 
			else {
				dmsgs.push(dmsg);
			}
		}
	}
	while (!dmsgs.empty())
	{
        mDelayMsgQueue.push(dmsgs.front());
		dmsgs.pop();
	}
}

void QyMessageQueue::dispatch(QyMessage *pmsg) 
{
	pmsg->handler->onMessage(pmsg);
}

void QyMessageQueue::ensureActive()
{		
    if (!mActive && mMsgQueueMgr) {
        mActive = true;
        mMsgQueueMgr->add(this);
	}
}

END_NAMESPACE(qy)
