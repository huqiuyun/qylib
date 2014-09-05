#pragma once

#include "ipc-plugin-mgr.h"
#include "ipc-object.h"
#include "ipc-engine-packet.h"
#include "ipc-plugin-dll.h"
#include "qyutil/qylock.h"
#include "qyutil/qysigslot.h"
#include "qyutil/qymessagethread.h"

class IPCTask;
//-----------------------------------------------------------------------------------------//
// IPCPluginMgrPrivate
class IPCPluginMgrPrivate : public sigslot::has_slots<>	, public qy::QyMessageHandler	
{
	typedef struct tagPluginItem
	{
		IPCTask* task;
		IPCPlugin_t plug;

		tagPluginItem()
		{
			task = NULL;
		}
	}PluginItem_t;
	typedef std::map<unsigned int, PluginItem_t> PluginMap_t;
	typedef std::map<std::wstring, IPCPluginDll*> DllMap_t;
public:
	IPCPluginMgrPrivate(IPCPluginMgr* q,IPCObject* ipc)
		:q_ptr_(q)
		,ipc_(ipc)
	{
	}

	IPCPluginMgrPrivate::~IPCPluginMgrPrivate()
	{
	}

	// slot
private:
	void onPluginResult(const IPCPlugin_t*, const char* taskId, int status,unsigned int err);
	void onPluginListen(const IPCEnginePack_t*);
	void onPluginData(unsigned int appid,const IPCEnginePack_t*);   
private:
	void onMessage(qy::QyMessage *msg);

private:
    void start(const IPCPlugin_t& plug,const char* taskid);

	void dealPluginListen(IPCPluginPack_t& plug);
    bool loadPlugin(IPCPluginPack_t& plug);
	bool unLoadPlugin(IPCPluginPack_t& plug);
	
	bool loadDll(const std::wstring& file,const std::wstring& cmdline,IPCResult_t& result);
	bool unLoadDll(const std::wstring& file,IPCResult_t& result);

	void initDll(const std::wstring& file);
	void checkDll(const CONNECTID& connId);
	void delayCheckDll(const CONNECTID& connId);
private:
	IPCPluginMgr* q_ptr_;
	IPCObject* ipc_;

	PluginMap_t plugins_;
	qy::QySLCS  plug_crit_;

	DllMap_t    dlls_;
	qy::QySLCS  dlls_crit_;

	friend class IPCPluginMgr;
};

