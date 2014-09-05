#include "ipc-msg-wnd.h"
#include "ipc-object.h"
#include "ipc-object-p.h"
#include "qyutil/qymessagedata.h"

class IPCMsgDelegate : public qy::IQyMsgCallback
{
public:
	IPCMsgDelegate(IPCMessageWnd* q);
private:
	LRESULT onMsg(unsigned int msgid, WPARAM , LPARAM p);
	IPCMessageWnd* q_ptr_;
};

IPCMsgDelegate::IPCMsgDelegate(IPCMessageWnd* q)
:q_ptr_(q)
{
}

LRESULT IPCMsgDelegate::onMsg(unsigned int msgid, WPARAM , LPARAM p)
{
	switch (msgid)
	{
	case kIPC_MSGID_CONNECTSTATUS:
		{
			qy::TypedMessageData<IPCConnectResult_t>* msg = reinterpret_cast<qy::TypedMessageData<IPCConnectResult_t>*>(p);
			q_ptr_->onConnectStatus(&msg->data());
		}
		break;

	case kIPC_MSGID_APP:
		{
			qy::TypedMessageData<IPCAppResult_t>* msg = reinterpret_cast<qy::TypedMessageData<IPCAppResult_t>*>(p);
			q_ptr_->onExeStatus(&msg->data());
		}
		break;

	case kIPC_MSGID_PLUGINLISTEN:
		{
			qy::TypedMessageData<IPCPluginPack_t>* msg = reinterpret_cast<qy::TypedMessageData<IPCPluginPack_t>*>(p);
			q_ptr_->onPluginListen(&msg->data());
		}
		break;

	case kIPC_MSGID_PLUGINSTATUS:
		{// notify to plugin status
			qy::TypedMessageData<IPCPluginResult_t>* msg = reinterpret_cast<qy::TypedMessageData<IPCPluginResult_t>*>(p);
			q_ptr_->onPluginStatus(&msg->data());
		}
		break;

	case kIPC_MSGID_PLUGINDATA:
		{
			qy::TypedMessageData<IPCEnginePack_t>* msg = reinterpret_cast<qy::TypedMessageData<IPCEnginePack_t>*>(p);
			q_ptr_->onPluginData(&msg->data());			
		}
		break;

	case kIPC_MSGID_QUIT:
		{
			IPCConnectResult_t status;
			status.result.status = kIPC_CS_ERROR;
			status.result.error = kIPC_RES_QUIT;
			q_ptr_->onConnectStatus(&status);
		}
		break;
	}
	return 0;
}

IPCMessageWnd::IPCMessageWnd(IPCObject* ipc,qy::QyWndMessage* wndMsg)
:qy::QyMessageWnd(wndMsg)
,ipc_(ipc)
{
	delegate_ = new IPCMsgDelegate(this);
	addDelegate(delegate_);
}

IPCMessageWnd::~IPCMessageWnd()
{
	removeDelegate(delegate_);
	delete delegate_;
}

void IPCMessageWnd::fnRemove(unsigned int msgid,WPARAM wParam , LPARAM& lParam)
{
	switch(msgid)
	{
	case kIPC_MSGID_APP:
		{
			qy::TypedMessageData<IPCAppResult_t>* msg = reinterpret_cast<qy::TypedMessageData<IPCAppResult_t>*>(lParam);

			delete msg;
		}
		break;

	case kIPC_MSGID_CONNECTSTATUS:
		{
			qy::TypedMessageData<IPCConnectResult_t>* msg = reinterpret_cast<qy::TypedMessageData<IPCConnectResult_t>*>(lParam);

			delete msg;
		}
		break;

	case kIPC_MSGID_PLUGINSTATUS:
		{
			qy::TypedMessageData<IPCEnginePack_t>* msg = reinterpret_cast<qy::TypedMessageData<IPCEnginePack_t>*>(lParam);

			delete msg;
		}
		break;

	case kIPC_MSGID_PLUGINLISTEN:
		{
			qy::TypedMessageData<IPCPluginPack_t>* msg = reinterpret_cast<qy::TypedMessageData<IPCPluginPack_t>*>(lParam);

			delete msg;
		}
		break;
	}
}

bool IPCMessageWnd::post(unsigned int msgid, WPARAM wParam, LPARAM lParam, qy::FnMsgPush fnPush, qy::FnMsgRemove fnRemove)
{
	return wndmsg_->post(msgid,wParam,lParam,fnPush,fnRemove);
}

bool IPCMessageWnd::send(unsigned int msgid, WPARAM wParam, LPARAM lParam, qy::FnMsgPush fnPush, qy::FnMsgRemove fnRemove)
{
	if (fnPush)
	{
		fnPush(msgid,wParam,lParam);
	}

	delegateProc(msgid,wParam,lParam);

	if (fnRemove)
	{
		fnRemove(msgid,wParam,lParam);
	}
	return true;
}

void IPCMessageWnd::onConnectStatus(const IPCConnectResult_t* res)
{			
	ipc_->dispatchMgr()->dispatch()->onConnectStatus(CONNECTID_TO_PTR(res->connId), res->result.status, res->result.error);
}

void IPCMessageWnd::onExeStatus(const IPCAppResult_t* res)
{// only server,notify all owner plugin 
	ipc_->dispatchMgr()->dispatch()->onExeStatus(&res->exe,res->result.status,res->result.error);
}

void IPCMessageWnd::onPluginStatus(const IPCPluginResult_t* res)
{// only server 
	if (kIPC_PS_START_LOAD == res->result.status)
	{
		ipc_->q_func()->pluginMgr()->dealPluginLoad(res->plug,res->taskId.c_str());	
	}
	ipc_->dispatchMgr()->dispatch()->onPluginStatus(res);
}

void IPCMessageWnd::onPluginData(const IPCEnginePack_t* res)
{
	if (ipc_->q_func()->pluginMgr()->find(res->head.fromid))
	{
		ipc_->dispatchMgr()->dispatch()->onPluginRecv(res->head.toid, &res->head,res->raw.data(),res->raw.size());
	}
}

void IPCMessageWnd::onPluginListen(const IPCPluginPack_t* plug)
{
	ipc_->q_func()->pluginMgr()->dealPluginListen(plug);
}
