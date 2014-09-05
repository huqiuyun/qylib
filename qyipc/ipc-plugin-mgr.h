#pragma once

#include "ipc-api.h"

class  IPCPluginMgrPrivate;
class  IPCObject;

class IPCPluginMgr : public IIPCPluginMgr
{
public:
    IPCPluginMgr(IPCObject* ipc);
	~IPCPluginMgr();

	void addListen(const CONNECTID& connId,bool client);
	void dealPluginListen(const IPCPluginPack_t* pack);
	void dealPluginLoad(const IPCPlugin_t& plug, const char* taskid = NULL);
	//IIPCPluginMgr
public:
	void start(const IPCPlugin_t& plug, const char* taskid = NULL, unsigned int timesec = kIPC_LOADPLUGIN_TIMOUT);
	void stop(unsigned int appid,const char* taskid = NULL);
	void destroy();

	bool find(unsigned int appid) const;	
    int  send(unsigned int appid,const IPCEnginePackHead_t* head,const char* raw,size_t rawlen);
	CONNECTID connectId(unsigned int appid);
private:
	IPCPluginMgrPrivate* d_ptr_;
	friend class IPCPluginMgrPrivate;
};
