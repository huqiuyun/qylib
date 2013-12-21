#include <assert.h>
#include <algorithm>
#include <list>
#include "qymessagethread.h"
#include "qytime.h"
#include "qywake.h"

DEFINE_NAMESPACE(qy)

static QyThreadKey gThreadKey = 0;
static QyMessageThreadManager gThreadMgr;

struct qy_sendmessage_t
{
    QyMessageThread *thread;
    QyMessage msg;
    bool *ready;
};
typedef std::list<qy_sendmessage_t> SENDMSGS;

const unsigned int MSDEV_SET_mThreadNAME = 0x406D1388;

class QyMessageThreadManagerPrivate
{
public:
    QyMessageThreadManagerPrivate()
    {
    }
    std::list<QyMessageThread *> mThreads;
    QySLCS mCrit;
};

#ifndef H_OS_WIN
QyMessageThreadManager::QyMessageThreadManager()
    :d_ptr(new QyMessageThreadManagerPrivate())
{
    pthread_key_create(&gThreadKey, NULL);
}

QyMessageThreadManager::~QyMessageThreadManager() {	
    pthread_key_delete(gThreadKey);
    delete d_ptr;
}

QyMessageThread *QyMessageThreadManager::currentThread() {
    return (QyMessageThread*)pthread_getspecific(gThreadKey);
}

void QyMessageThreadManager::setCurrent(QyMessageThread *thread) {
    pthread_setspecific(gThreadKey, thread);
}
#else
QyMessageThreadManager::QyMessageThreadManager()
    :d_ptr(new QyMessageThreadManagerPrivate())
{
    gThreadKey = TlsAlloc();
}

QyMessageThreadManager::~QyMessageThreadManager() {
    TlsFree(gThreadKey);
    delete d_ptr;
}

QyMessageThread *QyMessageThreadManager::currentThread() {
    return (QyMessageThread *)TlsGetValue(gThreadKey);
}

void QyMessageThreadManager::setCurrent(QyMessageThread *thread) {
    TlsSetValue(gThreadKey, thread);
}
#endif //H_OS_WIN

void QyMessageThreadManager::add(QyMessageThread *thread) {
    QY_D(QyMessageThreadManager);
    QyAutoLocker cs(&d->mCrit);
    d->mThreads.push_back(thread);
}

void QyMessageThreadManager::remove(QyMessageThread *thread) {
    QY_D(QyMessageThreadManager);
    QyAutoLocker cs(&d->mCrit);
    d->mThreads.erase(std::remove(d->mThreads.begin(), d->mThreads.end(), thread), d->mThreads.end());
}

class QyThreadWake : public QyWake
{
public:
    QyThreadWake(){}
    ~QyThreadWake(){}

public:
    void wake(void)
    {
        mEvent->wake();
    }

    bool wait(unsigned long cms,bool threadrun)
    {
        QY_UNUSED(threadrun);
        mEvent->wait(cms);
        mEvent->reset();
        return true;
    }
private:
    QySmartLock<HQyEvent,QyManualEvent> mEvent;
};


class QyAutoThread : public QyMessageThread
{
public:
    QyAutoThread(QyWake* wake,QyMsgQueueManager* p)
        : QyMessageThread(wake,p){
        QyMessageThreadManager::setCurrent(this);
    }
    ~QyAutoThread(){
        if (QyMessageThreadManager::currentThread() == this){
            QyMessageThreadManager::setCurrent(NULL);
        }
    }
};

class QyMessageThreadPrivate
{
public:
    QyMessageThreadPrivate(QyMessageThread* q, QyWake* wake) :
        q_ptr(q),
        mThread(NULL),
        mThreadWake(wake),
        mNewThreadWake(false),
        mHasSends(false)
    {
        mThread = QyThread::createThread((QyThreadRunFunction)&QyMessageThread::preRun,q_ptr);
        if (!mThreadWake)
        {
            mThreadWake = new QyThreadWake();
            mNewThreadWake = true;
        }
    }

    ~QyMessageThreadPrivate()
    {
        delete mThread;
        if (mNewThreadWake && mThreadWake) {
            delete mThreadWake;
            mThreadWake = NULL;
        }
    }
    QyMessageThread* q_ptr;
    QyThread* mThread;
    QyWake* mThreadWake;
    bool mNewThreadWake;
    bool mHasSends;
    SENDMSGS mMsgSends;
};

QyMessageThread::QyMessageThread(QyWake* wake,QyMsgQueueManager* p)
    : QyMessageQueue(p)
    , d_ptr(new QyMessageThreadPrivate(this,wake))
{
    gThreadMgr.add(this);

    if (!QyMessageThreadManager::currentThread()) {
        QyMessageThreadManager::setCurrent(this);
    }
}

QyMessageThread::~QyMessageThread()
{
    stop();
    if (mActive) {
        clear(NULL);
    }
    if (QyMessageThreadManager::currentThread()==this){
        QyMessageThreadManager::setCurrent(NULL);
    }
    gThreadMgr.remove(this);

    delete d_ptr;
}

void QyMessageThread::wake()
{ 
    QY_D(QyMessageThread);
    if (d->mThreadWake){
        d->mThreadWake->wake();
    }
}

void QyMessageThread::wait(int cms, bool threadrun) 
{
    QY_D(QyMessageThread);
    if (d->mThreadWake) {
        d->mThreadWake->wait(cms , threadrun);
    }
}

int QyMessageThread::preRun(QyThreadObj obj,ThreadEvent e)
{
    QyMessageThread *thread = (QyMessageThread *)obj;
    QyMessageThreadManager::setCurrent(thread);
    return thread->run(e);
}

int QyMessageThread::run(ThreadEvent e)
{
    if (kTStoped == e) {
        return kTResNone;
    }
    QyMessage msg;
    if (!get(&msg, kForever)){
        return kTResExit;
    }
    dispatch(&msg);

    return kTResNone;
}

void QyMessageThread::start()
{
    QY_D(QyMessageThread);
    QyMessageQueue::start();
    d->mThread->start();
}

void QyMessageThread::stop()
{
    QY_D(QyMessageThread);
    QyMessageQueue::stop();
    d->mThread->stop(kForever);
}

void QyMessageThread::send(QyMessageHandler *handler, uint32 id, QyMessageData *data)
{
    QY_D(QyMessageThread);

    if (mStop){
        return;
    }
    // Sent messages are sent to the QyMessageHandler directly, in the context
    // of "thread", like Win32 SendMessage. If in the right context,
    // call the handler directly.

    QyMessage msg;
    msg.handler = handler;
    msg.msgid = id;
    msg.data = data;
    if (isCurrent()) {
        handler->onMessage(&msg);
    }
    else{
        QyAutoThread thread(NULL,NULL);
        bool ready = false;
        {
            QyAutoLocker cs(&mCrit);
            ensureActive();
            qy_sendmessage_t smsg;
            smsg.thread = &thread;
            smsg.msg    = msg;
            smsg.ready  = &ready;
            d->mMsgSends.push_back(smsg);
            d->mHasSends  = true;
        }
        wake();

        // Wait for a reply
        while (!ready) {
            thread.wait(kForever, false);
        }
    }
}

void QyMessageThread::onSendMessage() 
{
    // Before entering critical section, check boolean.
    QY_D(QyMessageThread);
    mCrit.lock();
    if (!d->mHasSends){
        mCrit.unlock();
        return;
    }

    while(!d->mMsgSends.empty()) {
        qy_sendmessage_t msg = d->mMsgSends.front();
        d->mMsgSends.pop_front();
        mCrit.unlock();

        msg.msg.handler->onMessage(&msg.msg);

        mCrit.lock();
        *msg.ready = true;
        msg.thread->wake();
    }
    d->mHasSends = false;
    mCrit.unlock();
}

void QyMessageThread::clear(QyMessageHandler *handler, uint32 id)
{
    QY_D(QyMessageThread);
    QyAutoLocker cs(&mCrit);

    SENDMSGS::iterator iter = d->mMsgSends.begin();
    while (iter != d->mMsgSends.end()) {
        qy_sendmessage_t msg = *iter;
        if (handler == NULL ||
                msg.msg.handler == handler) {
            if (id == MQID_ANY || msg.msg.msgid == id) {
                iter = d->mMsgSends.erase(iter);
                *msg.ready = true;
                msg.thread->wake();
                continue;
            }
        }
        ++iter;
    }
    QyMessageQueue::clear(handler, id);
}

END_NAMESPACE(qy)
