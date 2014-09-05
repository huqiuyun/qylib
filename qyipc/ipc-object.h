#pragma once

#include "qyutil/qyconfig.h"
#include "ipc-api.h"

class IPCObjectPrivate;
class QYUTIL_API IPCObject : public IIPCApi
{
public:
    IPCObject();
	~IPCObject();

    bool ipcPost(unsigned int msgid, WPARAM wParam, LPARAM lParam, qy::FnMsgPush fnPush, qy::FnMsgRemove fnRemove);

    //IIPCApi
public:
	IIPCPluginMgr* pluginMgr();
	IIPCDispatchMgr* dispatchMgr();

	/** Step 1: server call */
	int  initServer(const IPCEnvironment_t& env);
    int  createServer(const IPCConnectConfig_t& connCfg,const CONNECTID_PTR parentConnId = kCONNECTID_UNK);
	/** Step 2: server call
	 *
	 * @return eIPCResult
	*/
	int  startupClient(const IPCExe_t& exe,const char* taskid = NULL);
   
	/** Step 1: client call */
	int  initClient(const wchar_t* exeuuid,const IPCEnvironment_t& env);
	/** Step 2: client call */
    int  connectToServer(const IPCConnectConfig_t& connCfg);

	/** 断开链路 */
    void close(const CONNECTID_PTR connId);
	void closeAll();

	/** 结束所有事务 */
	void uninit();

	/** 这是直接给链路发原始数据包 */
    int  send(const CONNECTID_PTR connId,const unsigned char* data, size_t len);
	bool isConnected(const CONNECTID_PTR connId) const;
	IPCConnectStatus_t connectStatus(const CONNECTID_PTR connId) const;

	void setApplicationDir(const wchar_t* dir);
	const wchar_t* applicationDir() const;

	void setUserConfig(const IPCUsrConfig_t& cfg);
	IPCUsrConfig_t usrConfig() const;

	bool cmdLine(const wchar_t* cmdLine, IPCExe_t& exe, IPCConnectConfig_t& connCfg, IPCEnvironment_t& env);

    void setQueryMethod(IIPCQueryMethod* method); 
    int  query(const char* cmd, const char* usrdata, IPCQueryResult_t* result);
private:
	IPCObjectPrivate* q_func() const;
    int init(const IPCEnvironment_t& env,bool client);

private:
	IPCObjectPrivate* d_ptr_;
	friend class IPCMessageWnd;
	friend class IPCPluginMgr;
	friend class IPCPluginMgrPrivate;
	friend class IPCTask;
	friend class IPCObjectPrivate;
};
