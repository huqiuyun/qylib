#ifndef QY_THREADQUEUE_H__
#define QY_THREADQUEUE_H__

#include <queue>
#include "qylock.h"
#include "qythread.h"

DEFINE_NAMESPACE(qy)

template<class T>
class IQyThreadHandlerT
{
public:
	virtual int onTQHandler(const T& value, eThreadEvent e){ return kTResNone; }
};

template <class T>
class QyThreadQueue
{
public:
	/// Constructor
	QyThreadQueue(IQyThreadHandlerT<T>* handler)
    :handler_(handler)
	{
        thread_ = QyThread::createThread(&QyThreadQueue<T>::runMsg,this);
	}
	/// Destructor
	~QyThreadQueue(void){
        stop();
        delete thread_;
	}
public:
	bool start(){
		QyAutoLocker cs(&msg_cs_);
        return thread_->start();
	}
    
	void stop(int msec = 2000){
        thread_->stop(msec);
	}
    
	void push(const T& msg,bool bactive = true)
	{
		QyAutoLocker cs(&msg_cs_);
		msgs_.push( msg);
        
		if (bactive)
        {
			thread_->start();
		}
        thread_->waitUp();
	}
    
	bool empty(){
		QyAutoLocker cs(&msg_cs_);
		return msgs_.empty();
	}
    
	size_t size(){
		QyAutoLocker cs(&msg_cs_);
		return msgs_.size();
	}
    
	void  clear()
	{
		QyAutoLocker cs(&msg_cs_);
		while (!msgs_.empty()){
			msgs_.pop();
		}
	}
private:
	bool pop(T& m){
		QyAutoLocker cs(&msg_cs_);
		if (msgs_.empty()){
            return false;
        }
        m = msgs_.front();
        msgs_.pop();
        return true;
    }
    
	int execute(eThreadEvent e){
		T msg;
        
        if (!handler_){
            return kTResExit;
        }
        
        if (e==kTRunning)
        {
            if (!pop(msg)){
                return kTResHook;
            }
            if (kTResExit==
                handler_->onTQHandler(msg, e)){
                return kTResExit;
            }
            return kTResNone;
        }
        else if (e==kTStoped)
        {
            while (pop(msg))
            {
                if (kTResExit==
                    handler_->onTQHandler(msg, e)){
                    break;
                }
            }
        }
        return kTResExit;
	}
    
	static int runMsg(QyThreadObj param,eThreadEvent e){
		QyThreadQueue<T>* th = (QyThreadQueue<T>*)param;
		return th->execute(e);
	}
private:
    std::queue<T>           msgs_;
    IQyThreadHandlerT<T>*   handler_;
    QySmartLock<QY_CS>      msg_cs_;
    QyThread*               thread_;
};

END_NAMESPACE(qy)

#endif // QY_THREADQUEUE_H__
