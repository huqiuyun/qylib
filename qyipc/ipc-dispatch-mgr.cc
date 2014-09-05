#include "ipc-dispatch-mgr.h"
#include "ipc-object.h"
#include "qyutil/qylock.h"
//-----------------------------------------------------------------------------------------//
// IPCDispatchMgrPrivate
class IPCDispatchMgrPrivate
{
    typedef std::map<IIPCDispatch*,void*> DispacthMap_t;
public:
	IPCDispatchMgrPrivate(IPCDispatchMgr* q,IPCObject* ipc)
		:q_ptr_(q)
		,ipc_(ipc)
	{
	}

	IPCDispatchMgrPrivate::~IPCDispatchMgrPrivate()
	{
	}
private:
	IPCDispatchMgr* q_ptr_;
	IPCObject* ipc_;
	
    DispacthMap_t dispatchs_;
    qy::QySLCS  crit_;
	friend class IPCDispatchMgr;
};

//-----------------------------------------------------------------------------------------//
// IPCDispatchMgr

IPCDispatchMgr::IPCDispatchMgr(IPCObject* ipc)
:d_ptr_(new IPCDispatchMgrPrivate(this,ipc))
{

}

IPCDispatchMgr::~IPCDispatchMgr()
{
	delete d_ptr_;
}

IIPCDispatch* IPCDispatchMgr::dispatch()
{
	return this;
}

void IPCDispatchMgr::addDispatch(IIPCDispatch* dis, void* object)
{
	QY_D(IPCDispatchMgr);
	qy::QyAutoLocker cs(&d->crit_);
	IPCDispatchMgrPrivate::DispacthMap_t::iterator iter = d->dispatchs_.find(dis);
	if (iter != d->dispatchs_.end())
	{
        iter->second = object;
	}
    else
    {
        d->dispatchs_.insert(std::make_pair(dis,object));
    }
}
 
void IPCDispatchMgr::removeDispatch(IIPCDispatch* dis)
{
	QY_D(IPCDispatchMgr);
	qy::QyAutoLocker cs(&d->crit_);
	IPCDispatchMgrPrivate::DispacthMap_t::iterator iter = d->dispatchs_.find(dis);
	if (iter != d->dispatchs_.end())
	{
        d->dispatchs_.erase(iter);
	}
}

void IPCDispatchMgr::removeDispatchWithObject(void* object)
{
	QY_D(IPCDispatchMgr);
	qy::QyAutoLocker cs(&d->crit_);
	IPCDispatchMgrPrivate::DispacthMap_t::iterator iter = d->dispatchs_.begin();
	while (iter != d->dispatchs_.end())
	{
        if (iter->second == object)
        {
            d->dispatchs_.erase(iter++);
            continue;
        }
        ++iter;
	}
}

void IPCDispatchMgr::onExeStatus(const IPCExe_t* exe, int status, int err)
{
	QY_D(IPCDispatchMgr);
	qy::QyAutoLocker cs(&d->crit_);
	IPCDispatchMgrPrivate::DispacthMap_t::iterator iter = d->dispatchs_.begin();
	while (iter != d->dispatchs_.end())
	{
        iter->first->onExeStatus(exe,status,err);
        ++iter;
	}
}

void IPCDispatchMgr::onConnectStatus(const CONNECTID_PTR connId,int status ,int err)
{
	QY_D(IPCDispatchMgr);
	qy::QyAutoLocker cs(&d->crit_);
	IPCDispatchMgrPrivate::DispacthMap_t::iterator iter = d->dispatchs_.begin();
	while (iter != d->dispatchs_.end())
	{
        iter->first->onConnectStatus(connId,status,err);
        ++iter;
	}
}

void IPCDispatchMgr::onPluginStatus(const IPCPluginResult_t* result)
{
	QY_D(IPCDispatchMgr);
	qy::QyAutoLocker cs(&d->crit_);
	IPCDispatchMgrPrivate::DispacthMap_t::iterator iter = d->dispatchs_.begin();
	while (iter != d->dispatchs_.end())
	{
        iter->first->onPluginStatus(result);
        ++iter;
	}
}

void IPCDispatchMgr::onPluginRecv(unsigned int appid, const IPCEnginePackHead_t* head, const char* raw, size_t rawlen)
{
	QY_D(IPCDispatchMgr);
	qy::QyAutoLocker cs(&d->crit_);
	IPCDispatchMgrPrivate::DispacthMap_t::iterator iter = d->dispatchs_.begin();
	while (iter != d->dispatchs_.end())
	{
        iter->first->onPluginRecv(appid,head,raw,rawlen);
        ++iter;
	}
}	
