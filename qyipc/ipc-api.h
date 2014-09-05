#pragma once

#include "qyutil/qyconfig.h"
#include "qyutil/qylog.h"
#include "ipc-engine-protocol.h"

struct QYNovtableM IIPCDispatch
{
	// for client
	virtual void onExeStatus(const IPCExe_t* exe, int status, int err) = 0;
	virtual void onConnectStatus(const CONNECTID_PTR connId,int status ,int err) =0;
	// for client
	virtual void onPluginStatus(const IPCPluginResult_t* result) = 0;
	virtual void onPluginRecv(unsigned int appid, const IPCEnginePackHead_t* head, const char* raw, size_t rawlen) = 0;
};

struct QYNovtableM IIPCDispatchMgr
{
	virtual IIPCDispatch* dispatch() = 0;
    virtual void addDispatch(IIPCDispatch* dis,void* object) = 0;
    virtual void removeDispatch(IIPCDispatch* dis) = 0;
    virtual void removeDispatchWithObject(void* object) = 0;  	
};

struct QYNovtableM IIPCPluginMgr
{
    /** push plug to tasks, if connect is connected,then it start,otherwise wait for connected */
	virtual void start(const IPCPlugin_t& plug, const char* taskid = NULL, unsigned int timesec = kIPC_LOADPLUGIN_TIMOUT) = 0;
	virtual void stop(unsigned int appid,const char* taskid = NULL) = 0;
	virtual void destroy() = 0;

	virtual bool find(unsigned int appid) const = 0;	
    virtual int  send(unsigned int appid, const IPCEnginePackHead_t* head, const char* raw, size_t rawlen) = 0;
	virtual CONNECTID connectId(unsigned int appid) = 0;
};


struct QYNovtableM IIPCQueryMethod
{
    virtual int query(const char* cmd,const char* usrdata,IPCQueryResult_t* result) = 0;
};

struct QYNovtableM IIPCApi
{
	virtual IIPCPluginMgr* pluginMgr() = 0;
	virtual IIPCDispatchMgr* dispatchMgr() = 0;

    /** Step 1: server call,create server, wait for connect from client
     *
     * */
	virtual int  initServer(const IPCEnvironment_t& env) = 0;
	/**
	 @param parentConnId,如果父链路存在，可以依托父链路建一个子链路
	*/
    virtual int  createServer(const IPCConnectConfig_t& connCfg,const CONNECTID_PTR parentConnId = kCONNECTID_UNK) = 0;

	/** Step 2: server call 第二步：启动一个客户端进程，以便与服务器建立链路*/
	virtual int  startupClient(const IPCExe_t& exe,const char* taskid = NULL) = 0;
   
	/** Step 1: client call */
	virtual int initClient(const wchar_t* exeuuid,const IPCEnvironment_t& env) = 0;

    /** Step 2: client call ,go to connect server
     * */
    virtual int  connectToServer(const IPCConnectConfig_t& connCfg) = 0;

	virtual void close(const CONNECTID_PTR connId)= 0;
	virtual void closeAll() = 0;
    virtual void uninit() = 0;
	
    /** 直接给链路发原始数据包 */
    virtual int  send(const CONNECTID_PTR connId,const unsigned char* data, size_t len) = 0;
	virtual bool isConnected(const CONNECTID_PTR connId) const = 0;
    virtual IPCConnectStatus_t connectStatus(const CONNECTID_PTR connId) const = 0;

	virtual void setApplicationDir(const wchar_t* dir) = 0;
	virtual const wchar_t* applicationDir() const = 0;

	virtual void setUserConfig(const IPCUsrConfig_t& cfg) = 0;
	virtual IPCUsrConfig_t usrConfig() const = 0;

	virtual bool cmdLine(const wchar_t* cmdLine, IPCExe_t& exe, IPCConnectConfig_t& connCfg, IPCEnvironment_t& env) = 0;
   
    virtual void setQueryMethod(IIPCQueryMethod* method) = 0; 
    virtual int  query(const char* cmd, const char* usrdata, IPCQueryResult_t* result) = 0;
};
