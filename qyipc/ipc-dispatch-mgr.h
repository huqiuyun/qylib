#pragma once

#include "ipc-api.h"

class IPCDispatchMgrPrivate;
class IPCObject;
class IPCDispatchMgr : public IIPCDispatchMgr, public IIPCDispatch
{
public:
    IPCDispatchMgr(IPCObject* ipc);
	~IPCDispatchMgr();
	//IIPCDispatchMgr
public:
	IIPCDispatch* dispatch();
    void addDispatch(IIPCDispatch* handler,void* object);
    void removeDispatch(IIPCDispatch* handler);
    void removeDispatchWithObject(void* object); 

	//IIPCDispatch
public:
	void onExeStatus(const IPCExe_t* exe, int status, int err);
	void onConnectStatus(const CONNECTID_PTR connId,int status ,int err);

	void onPluginStatus(const IPCPluginResult_t* result);
	void onPluginRecv(unsigned int appid, const IPCEnginePackHead_t* head, const char* raw, size_t rawlen);
	//
private:
	IPCDispatchMgrPrivate* d_ptr_;
	friend class IPCDispatchMgrPrivate;
};
