#pragma once

#include "ipc-object.h"
#include "ipc-taskrunner.h"
#include "ipc-tasks.h"
#include "ipc-pipe-base.h"
#include "ipc-engine.h"
#include "ipc-plugin-mgr.h"
#include "ipc-dispatch-mgr.h"
#include "ipc-engine-handler.h"
#include "ipc-msg-wnd.h"

#include "qyutil/qyenvironment.h"
#include "qyutil/qymessagethread.h"
#include "qyutil/qytimemanager.h"
#include "qyutil/qywndmsg.h"
#include "qyutil/qysigslot.h"
#include "qyutil/qyprocesscheck.h"

//-----------------------------------------------------------------------------------------//
// IPCObjectPrivate
class IPCObjectPrivate : public sigslot::has_slots<> , public qy::QyMessageHandler	
{
public:
	IPCObjectPrivate(IPCObject* q);
	~IPCObjectPrivate();
public:
    void addEngineHandler(const CONNECTID_PTR connId,IPCEngineHandler* handler,eEngineHandlerLevel level);
    void removeEngineHandler(const CONNECTID_PTR connId,IPCEngineHandler* handler);

	qy::QyMessageThread* msg() const;
	qy::QyTimeManager* timer() const;
	IPCTaskRunner* taskRunner() const;
	IPCPluginMgr* pluginMgr() const;
	//slot
private:
	void onExeInstalled(const IPCExe_t*,const char*, const IPCExeStatus_t* status);
	void onConnectStatus(const CONNECTID_PTR connId,int status, unsigned int err);
	void onConnectChild(const CONNECTID_PTR connId,const IPCConnectConfig_t*);
private:
	void onMessage(qy::QyMessage *msg);

	void destroyConnect(const CONNECTID_PTR connId);
	void destroyAllConnect();
	void addListen(const CONNECTID& connId,bool client);

	// push wait task, if connected ,then start the plugin
    void startPlugin(const IPCPlugin_t& plug);

    void removeConnectKey(const CONNECTID_PTR connId);
	void startConnectKeyWithListen(const CONNECTID_PTR connId);
	bool isConnectKeyWithListen(const CONNECTID_PTR connId);

    void setConnectKeyWithStatus(const CONNECTID_PTR connId, const IPCConnectStatus_t& status);
    IPCConnectStatus_t getConnectKeyWithStatus(const CONNECTID_PTR connId);

    CONNECTID mainConnected();

    void removeExeKey(const std::wstring& uuid);
	void startExeKeyWithTask(const std::wstring& uuid);
	bool isExeKeyWithTask(const std::wstring& uuid);

    void setExeKeyWithStatus(const std::wstring& uuid,const IPCExeStatus_t& status);
    IPCExeStatus_t getExeKeyWithStatus(const std::wstring& uuid);

private:
	IPCObject* q_ptr_;
	qy::QyMessageQueueManager* msgmgr_;
	qy::QyMessageThread* thread_;
	qy::QyTimeManager* timermagr_;
	qy::QyWndMessage*  wndmsg_;
	IPCTaskRunner* taskrunner_;
	IPCEngine*  engine_;
	IPCPluginMgr* pluginmgr_;
	IPCDispatchMgr* dispatchmgr_;
	IPCMessageWnd*  msgWnd_;
    IPCQueryMethod* query_method_;
    CONNECTID mainConnectId_;
    qy::QyEnvironment env_;
    struct ConnectKey
    {
        bool listen;
        IPCConnectStatus_t status;

        ConnectKey()
        {
            listen = false;
        }

        ConnectKey(bool li,const IPCConnectStatus_t& st)
        {
            listen = li;
            status = st;
        }
    }; 

    struct ExeKey
    {
        bool task;
        IPCExeStatus_t status;
        
        ExeKey()
        {
            task = false;
        }
		ExeKey(bool tk,const IPCExeStatus_t& st)
		{
			task = tk;
			status = st;
		}
    };
    typedef std::map<CONNECTID,ConnectKey> ConnectKeyMap;
	typedef std::map<std::wstring,ExeKey> ExeKeyMap;
	ConnectKeyMap connHash_;
	ExeKeyMap  exeHash_;
	qy::QySLCS key_crit_;

	bool client_;
	std::wstring app_dir_;	
	IPCUsrConfig_t usrCfg_;
	qy::QyProcessCheck client_exe_exist_;

	friend class IPCObject;
};
