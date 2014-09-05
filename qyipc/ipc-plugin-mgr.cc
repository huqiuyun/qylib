#include "ipc-plugin-mgr.h"
#include "ipc-object.h"
#include "ipc-object-p.h"
#include "ipc-plugin-mgr-p.h"
#include "ipc-tasks.h"
#include "ipc-taskrunner.h"
#include "ipc-msg-wnd.h"

#include "qyutil/qywndmsg.h"
#include "qyutil/qylog.h"
#include <cctype>
#include <algorithm>
#include <assert.h>

//-----------------------------------------------------------------------------------------//
// IPCPluginMgrPrivate
void IPCPluginMgrPrivate::onMessage(qy::QyMessage *msg)
{
	QY_UNUSED(msg);
	switch (msg->message_id)
	{
	case kIPC_MSGID_CHECKDLL:
		{
			qy::TypedMessageData<CONNECTID>* msgdata = static_cast<qy::TypedMessageData<CONNECTID>*>(msg->data);
			checkDll(msgdata->data());
			delete msgdata;		
		}
		break;
	}
}

void IPCPluginMgrPrivate::onPluginResult(const IPCPlugin_t* plug, const char* taskid, int status,unsigned int err)
{// client to request load OR unload dll,respone result	
	IPCPluginResult_t res;
	res.taskId= taskid;
	res.plug = *plug;
	res.result.status = status;
	res.result.error = err;
	ipc_->ipcPost(kIPC_MSGID_PLUGINSTATUS, 0, (LPARAM)new qy::TypedMessageData<IPCPluginResult_t>(res), NULL, IPCMessageWnd::fnRemove);
}

void IPCPluginMgrPrivate::onPluginListen(const IPCEnginePack_t* pack)
{// server listen from client request,AND need to load OR unload dll
	IPCPluginPack_t plug;
	sox::Unpack un((const void *)pack->raw.data(), pack->raw.size());
	plug.unmarshal(un);

	if (!ipc_->ipcPost(kIPC_MSGID_PLUGINLISTEN, 0, (LPARAM)new qy::TypedMessageData<IPCPluginPack_t>(plug), NULL, IPCMessageWnd::fnRemove))
	{
		QY_LOG(qy::kQyLogFinal, qy::kQyLogErr_LL, L"[IPC]Recv Fail: Plugin listen appid:%d", plug.plugin.appid);
	}
}

void IPCPluginMgrPrivate::onPluginData(unsigned int appid,const IPCEnginePack_t* data)
{// recv plugin data
	ipc_->ipcPost(kIPC_MSGID_PLUGINDATA, 0, (LPARAM)new qy::TypedMessageData<IPCEnginePack_t>(*data), NULL, IPCMessageWnd::fnRemove);
}

void IPCPluginMgrPrivate::dealPluginListen(IPCPluginPack_t& plug)
{
	std::wstring &file = plug.plugin.file;
	std::transform(file.begin(),file.end(),file.begin(),tolower);

	switch(plug.head.uri)
	{
	case kIPC_URI_UNLOAD_PLUGIN:
		QY_LOG(qy::kQyLogDev, qy::kQyLogInfo_LL, L"[IPC]Recv: [Begin]unload plugin request appid:%d", plug.plugin.appid);
		unLoadPlugin(plug);
		QY_LOG(qy::kQyLogDev, qy::kQyLogInfo_LL, L"[IPC]Recv: [End]unload plugin,err:%d\n\n",plug.result.error);
		break;

	case kIPC_URI_LOAD_PLUGIN:
		QY_LOG(qy::kQyLogDev, qy::kQyLogInfo_LL, L"[IPC]Recv: [Begin]load plugin request appid:%d", plug.plugin.appid);
    	loadPlugin(plug);
		QY_LOG(qy::kQyLogDev, qy::kQyLogInfo_LL, L"[IPC]Recv: [End]load plugin err:%d\n\n",plug.result.error);
		break;
	}
	// send reponse packet
	{
		IPCResponsePluginTask* task = new IPCResponsePluginTask(ipc_->q_func()->taskRunner(), ipc_, kEHL_APP, plug.plugin.connId, plug.plugin);
		task->setHead(plug.head);
		task->setResult(plug.result);
		task->start();
	}

	if (kIPC_URI_LOAD_PLUGIN_RES==plug.head.uri)
	{
		if (kIPC_PS_LOAD_OK == plug.result.status)
		{
			if(kIPC_RES_OK == plug.result.error)
			{
				initDll(plug.plugin.file);
			}
		}
		else
		{//加载失败，是否需要退出exe
			delayCheckDll(plug.plugin.connId);
		}
	}
	else if(kIPC_URI_UNLOAD_PLUGIN_RES==plug.head.uri)
	{
		if (kIPC_PS_UNLOAD_OK == plug.result.status &&
			kIPC_RES_OK == plug.result.error)
		{//是否退出exe 
			delayCheckDll(plug.plugin.connId);
		}
	}
}

bool IPCPluginMgrPrivate::unLoadPlugin(IPCPluginPack_t& plug)
{	
    plug.head.uri = kIPC_URI_UNLOAD_PLUGIN_RES;
	{// for lock
		qy::QyAutoLocker cs(&plug_crit_);
		PluginMap_t::iterator iter = plugins_.find(plug.plugin.appid);
		if (iter != plugins_.end())
		{
			if (iter->second.task)
			{
				static_cast<IPCListenPluginServiceTask*>(iter->second.task)->destroy();
				iter->second.task = NULL;
			}
			plugins_.erase(iter);
		}
	}
	return unLoadDll(plug.plugin.file, plug.result);
}

bool IPCPluginMgrPrivate::loadPlugin(IPCPluginPack_t& plug)
{	
    plug.head.uri = kIPC_URI_LOAD_PLUGIN_RES;
	//插件ID只能存在一个，但不同的ID可以使用同一个DLL
	{
		qy::QyAutoLocker cs(&plug_crit_);
		PluginMap_t::iterator iter = plugins_.find(plug.plugin.appid);
		if (iter != plugins_.end())
		{
			plug.result.status = kIPC_PS_LOAD_OK;
			plug.result.error = kIPC_RES_APP_EXISTED;
			return true;
		}
	}

	if (loadDll(plug.plugin.file, plug.plugin.cmdline, plug.result))
	{
		qy::QyAutoLocker cs(&plug_crit_);
		PluginItem_t item;
		item.plug = plug.plugin;		
		std::pair<IPCPluginMgrPrivate::PluginMap_t::iterator, bool> in;
		in = plugins_.insert(std::make_pair(plug.plugin.appid,item));
	
		IPCListenPluginServiceTask* task = new IPCListenPluginServiceTask(ipc_->q_func()->taskRunner(), ipc_, kEHL_HIGH, plug.plugin.connId, plug.plugin.appid);
		task->signalRecv.connect(this, &IPCPluginMgrPrivate::onPluginData);		
		task->start();
		in.first->second.task = task;
		
		return true;
	}
	return false;
}

bool IPCPluginMgrPrivate::unLoadDll(const std::wstring& file,IPCResult_t& result)
{
	qy::QyAutoLocker cs(&dlls_crit_);
	DllMap_t::iterator it = dlls_.find(file);
	if (it != dlls_.end())
	{
        void* obj = it->second->object();
		if (0 == it->second->Release())
		{			
			dlls_.erase(it);
            ipc_->dispatchMgr()->removeDispatchWithObject(obj);		
		}
		else
		{
			result.status = kIPC_PS_UNLOAD_FAIL;
		    result.error = kIPC_RES_HOOK;
			return false;
		}
	}
	result.status = kIPC_PS_UNLOAD_OK;
	result.error = 0;
	return true;
}

bool IPCPluginMgrPrivate::loadDll(const std::wstring& file,const std::wstring& cmdline,IPCResult_t& result)
{
	qy::QyAutoLocker cs(&dlls_crit_);
	DllMap_t::iterator it = dlls_.find(file);
	if (it == dlls_.end())
	{
		IPCPluginDll* dll = new IPCPluginDll();		

		result.status = dll->load(file.c_str());
		result.error = dll->error();

		if (kIPC_PS_LOAD_OK != result.status) 
		{
			delete dll;
			return false;
		}
		else if(!dll->fnEntry(ipc_,cmdline.c_str(),&result.status))
		{
			result.error = dll->error();
			delete dll;
			return false;
		}
		dll->AddRef();
		dlls_.insert(std::make_pair(file,dll));
	}
	else
	{
		it->second->AddRef();
	}
	return true;
}

void IPCPluginMgrPrivate::initDll(const std::wstring& file)
{
	qy::QyAutoLocker cs(&dlls_crit_);
	DllMap_t::iterator it = dlls_.find(file);
	if (it != dlls_.end())
	{
		it->second->fnInit();
	}
}

void IPCPluginMgrPrivate::checkDll(const CONNECTID& connId)
{
	bool isNil = false;
	{
		qy::QyAutoLocker cs(&dlls_crit_);
		isNil = ( dlls_.size() == 0);
	}

	if (isNil)
	{//quit exe
		// send
		IPCPacket packet;
		packet.sendQuitExe(ipc_,connId);
		//
		QY_LOG(qy::kQyLogDev, qy::kQyLogInfo_LL, L"[IPC]Post quit application message.");
		ipc_->ipcPost(kIPC_MSGID_QUIT, 0, 0, NULL, 0);
	}
}

void IPCPluginMgrPrivate::delayCheckDll(const CONNECTID& connId)
{
	bool isNil = false;
	{
		qy::QyAutoLocker cs(&dlls_crit_);
		isNil = ( dlls_.size() == 0);
	}

	if (isNil)
	{//try 
		QY_LOG(qy::kQyLogDev, qy::kQyLogInfo_LL, L"[IPC]Dll's queue is empty.");
		//
		//
		ipc_->q_func()->msg()->post(this,kIPC_MSGID_CHECKDLL,new qy::TypedMessageData<CONNECTID>(connId));
	}
}

void IPCPluginMgrPrivate::start(const IPCPlugin_t& plug,const char* taskid)
{
	{// for lock
		qy::QyAutoLocker cs(&plug_crit_);
		PluginItem_t* item_ptr = NULL;
		IPCPluginMgrPrivate::PluginMap_t::iterator iter = plugins_.find(plug.appid);
		if (iter != plugins_.end())
		{
			iter->second.plug = plug;
			item_ptr = &iter->second;
		}
		else
		{
			PluginItem_t item;
			item.plug = plug;
			std::pair<IPCPluginMgrPrivate::PluginMap_t::iterator, bool> in;
			in = plugins_.insert(std::make_pair(plug.appid,item));
			if (!in.second)
			{
				return;
			}
			item_ptr = &in.first->second;
		}

		if (!item_ptr->task)
		{
			IPCListenPluginServiceTask* task = new IPCListenPluginServiceTask(ipc_->q_func()->taskRunner(), ipc_, kEHL_HIGH, plug.connId, plug.appid);
			task->signalRecv.connect(this, &IPCPluginMgrPrivate::onPluginData);		
			task->start();
			item_ptr->task = task;
		}
	}

	{//todo load dll
		IPCLoadPluginTask* task = new IPCLoadPluginTask(ipc_->q_func()->taskRunner(), ipc_, kEHL_APP, plug.connId, plug, kIPC_URI_LOAD_PLUGIN);
		task->signalResult.connect(this, &IPCPluginMgrPrivate::onPluginResult);
		task->setTaskId(taskid);
		task->start();
	}
}

//-----------------------------------------------------------------------------------------//
// IPCPluginMgr

IPCPluginMgr::IPCPluginMgr(IPCObject* ipc)
:d_ptr_(new IPCPluginMgrPrivate(this,ipc))
{

}

IPCPluginMgr::~IPCPluginMgr()
{
    destroy();
#if defined(_DEBUG)
	QY_D(IPCPluginMgr);
    assert(d->dlls_.size()==0);
#endif
	delete d_ptr_;
}


void IPCPluginMgr::start(const IPCPlugin_t& plug, const char* taskid, unsigned int timesec)
{
	QY_D(IPCPluginMgr);
	IPCPluginWaitTask* task = new IPCPluginWaitTask(d->ipc_->q_func()->taskRunner(), d->ipc_, kEHL_LOW, plug.connId, plug);
	task->signalResult.connect(d,&IPCPluginMgrPrivate::onPluginResult);
	task->set_timeout_seconds(timesec);
	task->setTaskId(taskid);
	task->start();
}

void IPCPluginMgr::stop(unsigned int appid, const char* taskid)
{
	QY_D(IPCPluginMgr);
	IPCPlugin_t plug;
	{
		qy::QyAutoLocker cs(&d->plug_crit_);
		IPCPluginMgrPrivate::PluginMap_t::iterator iter = d->plugins_.find(appid);
		if (iter == d->plugins_.end())
		{
			return;
		}
		plug = iter->second.plug;		
		if (iter->second.task)
		{
			static_cast<IPCListenPluginServiceTask*>(iter->second.task)->destroy();
			iter->second.task = NULL;
		}
		d->plugins_.erase(iter);
	}
	{//send unload dll
		IPCLoadPluginTask* task = new IPCLoadPluginTask(d->ipc_->q_func()->taskRunner(), d->ipc_, kEHL_APP, plug.connId, plug, kIPC_URI_UNLOAD_PLUGIN);
		task->signalResult.connect(d, &IPCPluginMgrPrivate::onPluginResult);
		task->setTaskId(taskid);
		task->start();
	}
}

void IPCPluginMgr::addListen(const CONNECTID& connId,bool client)
{
    QY_UNUSED(client);

	QY_D(IPCPluginMgr);
	{ 
		IPCListenPluginTask* task = new IPCListenPluginTask(d->ipc_->q_func()->taskRunner(), d->ipc_, kEHL_TOPMOST, connId);
		task->signalPlguinListen.connect(d,&IPCPluginMgrPrivate::onPluginListen);
		task->start();
	}
}

void IPCPluginMgr::destroy()
{
	QY_D(IPCPluginMgr);
	std::list<std::wstring> dlls;
	{// for lock,and save to temp 
		qy::QyAutoLocker cs(&d->plug_crit_);
		IPCPluginMgrPrivate::PluginMap_t::iterator iter = d->plugins_.begin();
		while (iter != d->plugins_.end())
		{
			dlls.push_back(iter->second.plug.file);
			if (iter->second.task)
			{
				static_cast<IPCListenPluginServiceTask*>(iter->second.task)->destroy();
				iter->second.task = NULL;
			}
			++iter;
		}
		d->plugins_.clear();
	}

	{
		std::list<std::wstring>::iterator iter = dlls.begin();
		while (iter != dlls.end())
		{
			IPCResult_t result;
			d->unLoadDll(*iter,result);
			++iter;
		}
	}
}

bool IPCPluginMgr::find(unsigned int appid) const
{
	QY_D(IPCPluginMgr);
	qy::QyAutoLocker cs(&d->plug_crit_);
	return (d->plugins_.end() != d->plugins_.find(appid));
}

CONNECTID IPCPluginMgr::connectId(unsigned int appid)
{
	QY_D(IPCPluginMgr);
	qy::QyAutoLocker cs(&d->plug_crit_);
	IPCPluginMgrPrivate::PluginMap_t::iterator iter = d->plugins_.find(appid);
	if (iter != d->plugins_.end())
	{
		return iter->second.plug.connId;
	}
	return kCONNECTID_UNK;
}

int IPCPluginMgr::send(unsigned int appid,const IPCEnginePackHead_t* head,const char* raw,size_t rawlen)
{
	QY_D(IPCPluginMgr);
	CONNECTID connId = connectId(appid);
	if (kCONNECTID_UNK == connId)
	{
		return kIPC_RES_NOTFIND_PLUGIN;	
	}
	IPCPluginRawTask* task = new IPCPluginRawTask(d->ipc_->q_func()->taskRunner(), d->ipc_, kEHL_LOW, connId, appid);
	task->setHead(*head);
	task->setRaw(raw, rawlen);
	task->start();			
	return kIPC_RES_OK;	
}

void IPCPluginMgr::dealPluginListen(const IPCPluginPack_t* pack)
{
	QY_D(IPCPluginMgr);
	IPCPluginPack_t plug = *pack;
	d->dealPluginListen(plug);
}

void IPCPluginMgr::dealPluginLoad(const IPCPlugin_t& plug, const char* taskid)
{
	QY_D(IPCPluginMgr);
	d->start(plug,taskid);
}
