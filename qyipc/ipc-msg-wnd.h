#pragma once

#include "qyutil/qywndmsg.h"
#include "ipc-engine-protocol.h"

class IPCMessageWnd;
class IPCObject;
class IPCMsgDelegate;

class IPCMessageWnd : public qy::QyMessageWnd
{
public:
	IPCMessageWnd(IPCObject* ipc,qy::QyWndMessage* wndMessage);
	~IPCMessageWnd();

	bool post(unsigned int msgid, WPARAM wParam, LPARAM lParam, qy::FnMsgPush fnPush, qy::FnMsgRemove fnRemove);
	bool send(unsigned int msgid, WPARAM wParam, LPARAM lParam, qy::FnMsgPush fnPush, qy::FnMsgRemove fnRemove);

	static void fnRemove(unsigned int msgid,WPARAM wParam , LPARAM& lParam);
private:

	void onConnectStatus(const IPCConnectResult_t* res);
	void onExeStatus(const IPCAppResult_t* res);

	void onPluginListen(const IPCPluginPack_t* plug);
	void onPluginStatus(const IPCPluginResult_t* res);
	void onPluginData(const IPCEnginePack_t* res);	
private:
	friend class IPCMsgDelegate;
	IPCObject* ipc_;
	IPCMsgDelegate* delegate_;
};
