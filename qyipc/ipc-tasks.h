#pragma once

#include "qyutil/qytask.h"
#include "qyutil/qysigslot.h"

#include "ipc-object.h"
#include "ipc-struct.h"
#include "ipc-engine-handler.h"
#include "ipc-engine-packet.h"

#define TASK_CONSTRUCT_PARAM  \
	qy::QyTask* parent,IPCObject* ipc,eEngineHandlerLevel level,const CONNECTID& id

class IPCSynWait;
class IPCObject;
class IPCTask 
	: public qy::QyTask 
	, public IPCEngineHandler
{
public:
    IPCTask(TASK_CONSTRUCT_PARAM);
    ~IPCTask();

	void setHead(const IPCEnginePackHead_t& head);
	void setResult(const IPCResult_t& result);
	void setTaskId(const char* taskid);
	
	static const wchar_t* uriName(int uri);
	static const wchar_t* typeName(int type);
	//IPCEngineHandler
protected:
    bool onEngineHandler(const unsigned char* data,size_t len);
	void onEngineStatus(int status,int err);

protected:
    IPCObject* ipc() const;
	const char* taskId() const;

    int processInit();
protected:
    IPCObject* ipc_;
	CONNECTID  connId_;
    IPCConnectStatus_t connStatus_;
    IPCEnginePacketQueue queue_;
	IPCEnginePackHead_t  head_;
	IPCResult_t result_;
};


/** 应用程序exe任务 */
class IPCExeTask : public IPCTask
{
public:
	IPCExeTask(TASK_CONSTRUCT_PARAM, const IPCExe_t& app);
	~IPCExeTask();

	/**
	  @param 0 = 
	  @param 1 = taskid
	  @param 2 = status,  @IPCExeStatus_t 
	 */
	sigslot::signal3<const IPCExe_t*,const char*, const IPCExeStatus_t*> signalResult;

private:
	int processStart();
    int crashStatus(int st);
protected:
    bool onEngineHandler(const unsigned char* data,size_t len);
private:
	IPCExe_t app_;
	int recount_;
	bool check_;
	unsigned long processId_;
	int quit_flag_;
};

class IPCQueryTask : public IPCTask
{
public:
    IPCQueryTask(TASK_CONSTRUCT_PARAM,const IPCQuery_t& query);

private:
    int  processStart();
    int  processResponse();
    int  onTimeout();
    bool onEngineHandler(const unsigned char* data,size_t len);

    void finished();
    IPCQueryResult_t* queryResult();
private:
    IPCQuery_t query_;
    IPCSynWait* synwait_;
};
//
class IPCListenQueryTask : public IPCTask
{
public:
    IPCListenQueryTask(TASK_CONSTRUCT_PARAM);

private:
    int  processStart();
    bool onEngineHandler(const unsigned char* data,size_t len);
};

/** 监听task
 * APP通路状态
 */
class IPCListenConnectTask : public IPCTask
{
public:
	IPCListenConnectTask(TASK_CONSTRUCT_PARAM);
	~IPCListenConnectTask();

	/** param1= status,param2 = error */
	sigslot::signal3<const CONNECTID_PTR,int,unsigned int> signalConnectStatus;
	sigslot::signal2<const CONNECTID_PTR,const IPCConnectConfig_t*> signalConnectChild;
protected:
	void onEngineStatus(int status,int err);
    bool onEngineHandler(const unsigned char* data,size_t len);
private:
	int processStart();
private:
	int statuschanged_;
};

/** 创建一个子链路
 */
class IPCCreateChildConnectTask : public IPCTask
{
public:
	IPCCreateChildConnectTask(TASK_CONSTRUCT_PARAM,const IPCConnectConfig_t& connCfg);

private:
	int processStart();
private:
	IPCConnectConfig_t connCfg_;
};

/** plugin(DLL),server task, 等待链路OK,就自动运行加载,超时 10秒 */
class IPCPluginWaitTask : public IPCTask
{
public:
	IPCPluginWaitTask(TASK_CONSTRUCT_PARAM, const IPCPlugin_t& plugin);

	/**
	  @param 0 = 
	  @param 1 = taskid
	  @param 2 = status,  @eIPCPluginStatus 
	  @param 3 = error
	 */
	sigslot::signal4<const IPCPlugin_t*,const char*,int,unsigned int> signalResult;
    
private:
	int processStart();
	int onTimeout();
private:
	IPCPlugin_t plugin_;
};

/** (LOAD,UNLOAD) response client task */
class IPCResponsePluginTask : public IPCTask
{
public:
	IPCResponsePluginTask(TASK_CONSTRUCT_PARAM, const IPCPlugin_t& plugin);

private:
	int processStart();

private:
	IPCPlugin_t plugin_;
};

/** plugin(DLL(load,unload) server  任务 */
class IPCLoadPluginTask : public IPCTask
{
public:
	IPCLoadPluginTask(TASK_CONSTRUCT_PARAM, const IPCPlugin_t& plugin, int uri);

	/** param0 = status = eIPCPluginStatus , param1= error */
	sigslot::signal4<const IPCPlugin_t*, const char* , int,unsigned int> signalResult;
    
protected:
	bool onEngineHandler(const unsigned char* data,size_t len);

private:
	int processStart();
	int processResponse();
private:
	IPCPlugin_t plugin_;
};

/** listen client任务 */
class IPCListenPluginTask : public IPCTask
{
public:
	IPCListenPluginTask(TASK_CONSTRUCT_PARAM);

    sigslot::signal1<const IPCEnginePack_t*> signalPlguinListen;
	
protected:
	bool onEngineHandler(const unsigned char* data,size_t len);

private:
	int processStart();
};

/**
 * @brief  监听task,插件服务处理任务
 */
class IPCListenPluginServiceTask : public IPCTask
{
public:
    IPCListenPluginServiceTask(TASK_CONSTRUCT_PARAM, unsigned int appid);
    ~IPCListenPluginServiceTask();

	void destroy();
	/** @param 0 appid
	*/
    sigslot::signal2<unsigned int,const IPCEnginePack_t*> signalRecv;

protected:
    bool onEngineHandler(const unsigned char* data,size_t len);

private:
    int processStart();

private:
	unsigned int appid_;
	bool destroy_;
};

/**
 * @brief  插件数据包
 */
class IPCPluginRawTask : public IPCTask
{
public:
    IPCPluginRawTask(TASK_CONSTRUCT_PARAM, unsigned int appid);

	void setRaw(const char* data,size_t rawlen);

private:
	int processStart();
private:
	unsigned int appid_;
	IPCEnginePack_t pack_;
};

/** 直接发包 */
class IPCPacket
{
public:
	int sendQuitExe(IPCObject* ipc,const CONNECTID& id);
};

/** for synchronous wait ,@see IPCQueryTask */
class IPCSynWait
{
public:
    IPCSynWait(IPCObject* ipc, void* obj);

    int   exec();
    void  quit(int code);
    void* object() const;

private:
    IPCObject* ipc_;
    void* object_;
    int code_;
};

