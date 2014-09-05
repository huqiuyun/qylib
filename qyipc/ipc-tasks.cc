#include "ipc-tasks.h"
#include "ipc-object.h"
#include "ipc-object-p.h"

#include "qyutil/qyprocesscheck.h"
#include "qyutil/qytime.h"
#include "qyutil/qylog.h"

class IPCAutoTaskWake
{
public:
	IPCAutoTaskWake(qy::QyTask* task,int cmsDelay=0)
		:task_(task)
		,cmsDelay_(cmsDelay)
	{
	}

	~IPCAutoTaskWake()
	{
		task_->wake(cmsDelay_);
	}
private: 
	qy::QyTask* task_;
	int cmsDelay_;
};

IPCTask::IPCTask(TASK_CONSTRUCT_PARAM)
:qy::QyTask(parent)
,ipc_(ipc)
,connId_(id)
{    
    ipc_->q_func()->addEngineHandler(CONNECTID_TO_PTR(connId_),this,level);
}

IPCTask::~IPCTask()
{
	ipc_->q_func()->removeEngineHandler(CONNECTID_TO_PTR(connId_),this);
}

IPCObject* IPCTask::ipc() const
{
	return ipc_;
}

bool IPCTask::onEngineHandler(const unsigned char* data,size_t len)
{
	QY_UNUSED(data);
	QY_UNUSED(len);
	return false;
}

void IPCTask::onEngineStatus(int st,int err)
{
    IPCConnectStatus_t status(st,err);
    if (!status.isLoss())
    {
        connStatus_ = status;
        wake();
    }
}

int IPCTask::processInit() 
{
	connStatus_ = ipc_->connectStatus(CONNECTID_TO_PTR(connId_));
	return STATE_START; 
}

void IPCTask::setHead(const IPCEnginePackHead_t& head) 
{ head_ = head; }

void IPCTask::setResult(const IPCResult_t& result)
{ result_ = result; } 

void IPCTask::setTaskId(const char* taskid)
{ if (taskid) head_.taskid = taskid;}

const char* IPCTask::taskId() const
{
	return head_.taskid.c_str();
}

const wchar_t* IPCTask::uriName(int uri)
{
	switch(uri)
	{
	case kIPC_URI_LOAD_PLUGIN:    return L"Load";
	case kIPC_URI_UNLOAD_PLUGIN:  return L"Unload";
	}
	return L"Unk";
}

const wchar_t* IPCTask::typeName(int type)
{
	switch(type)
	{
	case kIPC_PT_DATA:   return L"Data";
	case kIPC_PT_PLUGIN: return L"Plugin";
	case kIPC_PT_SERVER: return L"Server";
	case kIPC_PT_VIDEO:  return L"Video";
	}
	return L"Unk";
}
//
IPCExeTask::IPCExeTask(TASK_CONSTRUCT_PARAM,const IPCExe_t& app)
:IPCTask(parent,ipc,level,id)
,app_(app)
,recount_(0)
,check_(false)
,processId_(0)
,quit_flag_(kIPC_QUIT_FLAG_UNK)
{
}

IPCExeTask::~IPCExeTask()
{
	IPCExeStatus_t status(crashStatus(kIPC_EXES_TASK_END), 0, processId_);
	signalResult(&app_, "", &status);
}

int IPCExeTask::processStart()
{
	if (check_)
	{// installed,AND listen is process runing
		if (connStatus_.isClosed())
		{
			IPCExeStatus_t status(crashStatus(kIPC_EXES_QUIT), 0, processId_);
			signalResult(&app_, taskId(),  &status);
			return STATE_ERROR;
		}
		return STATE_BLOCKED;
	}
	else
	{// install 
		if (!connStatus_.isConnecting() &&
			!connStatus_.isConnected())
		{
			return STATE_BLOCKED;
		}
		QY_LOG(qy::kQyLogFinal, qy::kQyLogInfo_LL, L"[IPC] IPCExeTask,uuid:%s", app_.uuid.c_str());

		BOOL rs = TRUE; 
		int  st = kIPC_EXES_OK;
		unsigned int err = 0;

		STARTUPINFO si = {0};
		si.cb = sizeof(si);	

		PROCESS_INFORMATION pi;
		ZeroMemory( &pi, sizeof(pi));

		qy::QyProcessCheck proc;
		if (!proc.checkWithHead(app_.uuid.c_str()))
		{//没有这个进程
			std::wstring exec_app = app_.file;
			exec_app.append(L" ");
			exec_app.append(app_.cmdline);

			rs = CreateProcess(NULL,        // No module name (use command line)
				(wchar_t*)exec_app.c_str(),			// Command line
				NULL,						// Process handle not inheritable
				NULL,						// Thread handle not inheritable
				FALSE,						// Set handle inheritance to FALSE
				0,							// No creation flags
				NULL,						// Use parent's environment block
				app_.curdir.c_str(),      // 修改工作路径以加载正确的DLL
				&si,						// Pointer to STARTUPINFO structure
				&pi );						// Pointer to PROCESS_INFORMATION structure

			::CloseHandle(pi.hProcess);
			::CloseHandle(pi.hThread);

			processId_ = pi.dwProcessId;
		}
		else
		{
			st = kIPC_EXES_OK;
			err = kIPC_RES_APP_EXISTED;
		}
		if (rs)
		{
			quit_flag_ = kIPC_QUIT_FLAG_CRASH;
			IPCExeStatus_t status(st, err, processId_);
			signalResult(&app_, taskId(), &status);
			if (err != 0)
			{
				return STATE_DONE;
			}
			check_ = true;
			return STATE_BLOCKED;
		}
		else if (recount_++ < 3)
		{
			return STATE_START;
		}
		else
		{
			IPCExeStatus_t status(crashStatus(kIPC_EXES_INSTALL_FAIL), 0, processId_);
			signalResult(&app_, taskId(), &status);
			return STATE_ERROR;
		}
	}
}

bool IPCExeTask::onEngineHandler(const unsigned char* data,size_t len)
{
	sox::Unpack un((const void *)data, len);
	IPCEnginePackHead_t head;
	head.unmarshal(un);

	if (kIPC_PT_APP != head.type)
	{
		return false;
	}
	if (kIPC_URI_QUIT != head.uri)
	{
		return false;
	}
	if (head.reserve != processId_)
	{
		return false;
	}
	quit_flag_ = kIPC_QUIT_FLAG_NORMAL;
	QY_LOG(qy::kQyLogFinal, qy::kQyLogInfo_LL, L"[IPC]Recv: Client normal quit.");
	return true;
}

int IPCExeTask::crashStatus(int st)
{
    return (kIPC_QUIT_FLAG_CRASH == quit_flag_)?kIPC_EXES_CRASH:st; 
}
//
IPCQueryTask::IPCQueryTask(TASK_CONSTRUCT_PARAM,const IPCQuery_t& query,IPCSynWait* wait)
:IPCTask(param,ipc,level,id)
,query_(query)
,synwait_(wait)
{
}

IPCQueryTask::~IPCQueyTask()
{
    if (synwait_)
    {
        queryResult()->result.status = kIPC_RES_FAIL;
        finished();
    }
}

int IPCQueryTask::processStart()
{
    sox::PackBuffer buffer;
    sox::Pack pk(buffer);
	
    head_.type = kIPC_PT_APP;
    head_.uri = kIPC_URI_QUERY;
    head_.marshal(pk);
	query_.marshal(pk);

	int res = ipc()->send(connId_,(const unsigned char*)pk.data(), pk.size());
	if (res != 0)
    {
        IPCQueryResult_t* qres = queryResult();
        qres->result.status = res;
        finished();
        return STATE_ERROR;
    }
    return STATE_RESPONSE;
}

int  IPCQueryTask::processResponse()
{
    return done_?STATE_DONE:STATE_BLOCKED;
}

int  IPCQueryTask::onTimeout()
{
    IPCQueryResult_t* qres = queryResult();
    qres->result.status = kIPC_RES_TIMEOUT;
    finished();
    return STATE_DONE;
}

bool IPCQueryTask::onEngineHandler(const unsigned char* data,size_t len)
{
	sox::Unpack un((const void *)data, len);
	IPCEnginePackHead_t head;
	head.unmarshal(un);

	if (kIPC_PT_APP != head.type)
	{
		return false;
	}

	if (kIPC_URI_QUERY_RES != head.uri)
	{
		return false;
    }

    IPCQuery_t query;
    query.unmarshal(un);
    if (query.cmd != query_.cmd || query.usrdata != query_.usrdata)
    {
        return false;
    }
    IPCQueryResult_t* qres = queryResult();
    qres->unmarshal(un);
    finished();
    done_ = true;
    wake();
    return true;
}

IPCQueryResult_t* IPCQueryTask::queryResult()
{
    return reinterprest_cast<IPCQueryResult_t*>(synwait_->object());
}

void IPCQueryTask::finished()
{
    synwait_->quit(queryResult()->result.status);
    synwait_ = NULL;
}
//
IPCListenQueryTask::IPCListenQueryTask(TASK_CONSTRUCT_PARAM)
:IPCTask(parent,ipc,level,id)
{
}

bool IPCListenQueryTask::onEngineHandler(const unsigned char* data,size_t len)
{
	// reponse,if true,then insert pack to queue
	sox::Unpack un((const void *)data, len);
	IPCEnginePackHead_t head;
	head.unmarshal(un);

	if (kIPC_PT_APP != head.type)
	{
		return false;
	}
	if (kIPC_URI_QUERY != head.uri)
	{
		return false;
    }
	queue_.push(head, un.data(), un.size());
    wake();
    return true;
}

int IPCListenQueryTask::processStart()
{ 
	if (connStatus_.isClosed())
	{
		return STATE_ERROR;
	}

    if (connStatus_.isConnecting())
    {
        return STATE_BLOCKED;
	}

	const IPCEnginePack_PTR pack = queue_.next();
	if (NULL==pack)
	{
		return STATE_BLOCKED;
	}
    IPCEnginePackHead_t head = pack->head; 
    head.uri = kIPC_URI_QUERY_RES;

    IPCQuery_t req;
    sox::Unpack up((const char*)pack->raw.c_str(), pack->raw.size());
    req.unmarshal(up);

    IPCQureyResult_t qres;
    IIPCQueryMethod* method = ipc()->queryMethod();
    if (method)
    {
        qres.result.error = method->query(req.cmd.c_str(),req.usrdata.c_str(), &qres);
        if ( 0 != qres.result.error)
        {
            qres.result.status = kIPC_RES_FAIL;
        }
        else
        {
            qres.result.status = kIPC_RES_OK;
        }
    }
    else
    {
        qres.result.status = kIPC_RES_NOTINTERFACE;
    }

    sox::PackBuffer buffer;
    sox::Pack pk(buffer);
	head.marshal(pk);
	req.marshal(pk);
    qres.marshal(pk);

	int res = ipc()->send(connId_,(const unsigned char*)pk.data(), pk.size());
	if (res != 0)
	{	
		QY_LOG(qy::kQyLogFinal, qy::kQyLogErr_LL, L"[IPC]Send Fail: IPCListenQueryTask err:%d", res);
	}
    delete pack;
	return STATE_START;
}
//
IPCListenConnectTask::IPCListenConnectTask(TASK_CONSTRUCT_PARAM)
:IPCTask(parent,ipc,level,id)
,statuschanged_(0)
{
}

IPCListenConnectTask::~IPCListenConnectTask()
{
	signalConnectStatus(CONNECTID_TO_PTR(connId_),kIPC_CS_LISTENTASK_END,0);
}

void IPCListenConnectTask::onEngineStatus(int st,int err)
{
    IPCConnectStatus_t status(st,err);
    if (status.isLoss())
    {
		signalConnectStatus(CONNECTID_TO_PTR(connId_),st,err);
    }
	else
    {
		statuschanged_ ++;
        IPCTask::onEngineStatus(st,err); 
	}
}

bool IPCListenConnectTask::onEngineHandler(const unsigned char* data,size_t len)
{
	// reponse,if true,then insert pack to queue
	QY_UNUSED(data);
	QY_UNUSED(len);

	sox::Unpack un((const void *)data, len);
	IPCEnginePackHead_t head;
	head.unmarshal(un);

	if (kIPC_PT_APP != head.type)
	{
		return false;
	}
	if (kIPC_URI_CREATE_CHILDCONNECT != head.uri)
	{
		return false;
	}
	IPCConnectConfig_t cfg;
	cfg.unmarshal(un);
    signalConnectChild(CONNECTID_TO_PTR(connId_), &cfg);
	return true;
}

int IPCListenConnectTask::processStart()
{
    if (statuschanged_<=0)
    {
		statuschanged_ = 0;
        return STATE_BLOCKED;
    }
	statuschanged_--;

	if (connStatus_.isConnected() ||
		connStatus_.isConnecting())
	{
		IPCAutoTaskWake aw(this);//for other task quit.
		signalConnectStatus(CONNECTID_TO_PTR(connId_),connStatus_.status,connStatus_.error());
	}
	else if (connStatus_.isClosed())
	{
		IPCAutoTaskWake aw(this);//for other task quit.
		signalConnectStatus(CONNECTID_TO_PTR(connId_),connStatus_.status,connStatus_.error());
		return STATE_DONE;
	}
	return STATE_START;
}
//
IPCCreateChildConnectTask::IPCCreateChildConnectTask(TASK_CONSTRUCT_PARAM,const IPCConnectConfig_t& connCfg)
:IPCTask(parent,ipc,level,id)
,connCfg_(connCfg)
{
}

int IPCCreateChildConnectTask::processStart()
{
	if (connStatus_.isClosed())
	{
		QY_LOG(qy::kQyLogFinal, qy::kQyLogErr_LL, L"[IPC]Send Fail: because connect is closed,IPCCreateChildConnectTask ");
		return STATE_ERROR;
	}
	head_.type = kIPC_PT_APP;
	head_.uri = kIPC_URI_CREATE_CHILDCONNECT;

    sox::PackBuffer buffer;
    sox::Pack pk(buffer);
	head_.marshal(pk);
	connCfg_.marshal(pk);
		
	int res = ipc()->send(connId_,(const unsigned char*)pk.data(), pk.size());
	if (res != 0)
	{	
		QY_LOG(qy::kQyLogFinal, qy::kQyLogErr_LL, L"[IPC]Send Fail: IPCCreateChildConnectTask err:%d", res);
		return STATE_ERROR;
	}
	return STATE_DONE;
}

//
IPCPluginWaitTask::IPCPluginWaitTask(TASK_CONSTRUCT_PARAM,const IPCPlugin_t& plugin)
:IPCTask(parent,ipc,level,id)
,plugin_(plugin)
{
}

int IPCPluginWaitTask::processStart()
{
	if (connStatus_.isConnected())
	{//
		signalResult(&plugin_,taskId(),kIPC_PS_START_LOAD,0);
		return STATE_DONE;
	}
    else if (connStatus_.isClosed())
    {
        signalResult(&plugin_,taskId(),kIPC_PS_LOAD_FAIL,connStatus_.error());
        return STATE_DONE;
    }
	return STATE_BLOCKED;
}

int IPCPluginWaitTask::onTimeout()
{
	signalResult(&plugin_,taskId(), kIPC_PS_LOAD_FAIL,kIPC_RES_TIMEOUT);
	return STATE_DONE;
}

//
IPCResponsePluginTask::IPCResponsePluginTask(TASK_CONSTRUCT_PARAM,const IPCPlugin_t& plugin)
:IPCTask(parent,ipc,level,id)
,plugin_(plugin)
{
}

int IPCResponsePluginTask::processStart()
{
	if (connStatus_.isClosed())
	{
		QY_LOG(qy::kQyLogFinal, qy::kQyLogErr_LL, L"[IPC]Send Fail: because connect is closed,IPCResponsePluginTask ,appid:%d", plugin_.appid);
		return STATE_ERROR;
	}
    if (connStatus_.isConnecting())
	{
        return STATE_BLOCKED;
    }

    if (kIPC_URI_LOAD_PLUGIN_RES != head_.uri ||
        kIPC_URI_UNLOAD_PLUGIN_RES != head_.uri)
    {
		QY_LOG(qy::kQyLogFinal, qy::kQyLogErr_LL, L"[IPC]Send Fail:Uri is not match, IPCResponsePluginTask uri:%s ,appid:%d", uriName(head_.uri), plugin_.appid);
        return STATE_ERROR;
    }

    IPCPluginPack_t pack;
	pack.plugin = plugin_;
	pack.head = head_;
	pack.result = result_;

    sox::PackBuffer buffer;
    sox::Pack pk(buffer);
	pack.marshal(pk);
	
	QY_LOG(qy::kQyLogDev, qy::kQyLogInfo_LL, L"[IPC]Send: IPCResponsePluginTask uri:%s,appid:%d,err:%d", uriName(head_.uri), plugin_.appid,result_.error);
	int res = ipc()->send(plugin_.connId,(const unsigned char*)pk.data(), pk.size());
	if (res != 0)
	{	
		QY_LOG(qy::kQyLogFinal, qy::kQyLogErr_LL, L"[IPC]Send Fail: IPCResponsePluginTask appid:%d,err:%d", plugin_.appid, res);
		return STATE_ERROR;
	}
	return STATE_DONE;
}

//
IPCLoadPluginTask::IPCLoadPluginTask(TASK_CONSTRUCT_PARAM,const IPCPlugin_t& plugin,int uri)
:IPCTask(parent,ipc,level,id)
,plugin_(plugin)
{
	head_.type = kIPC_PT_APP;
	head_.uri = uri;
}

int IPCLoadPluginTask::processStart()
{
	if (connStatus_.isClosed())
	{
		QY_LOG(qy::kQyLogFinal, qy::kQyLogErr_LL, L"[IPC]Send Fail:Cnnect is closed, IPCLoadPluginTask uri:%s ,appid:%d", uriName(head_.uri), plugin_.appid);
		return STATE_ERROR;
	}
    if (connStatus_.isConnecting())
    {
        return STATE_BLOCKED;
    }

    if (kIPC_URI_LOAD_PLUGIN != head_.uri ||
        kIPC_URI_UNLOAD_PLUGIN != head_.uri)
    {
		QY_LOG(qy::kQyLogFinal, qy::kQyLogErr_LL, L"[IPC]Send Fail:Uri is not match, IPCLoadPluginTask uri:%s ,appid:%d", uriName(head_.uri), plugin_.appid);
        return STATE_ERROR;
    }
	IPCPluginPack_t pack;
	pack.plugin = plugin_;
	pack.head = head_;

    sox::PackBuffer buffer;
    sox::Pack pk(buffer);
	pack.marshal(pk);
	
	QY_LOG(qy::kQyLogDev, qy::kQyLogInfo_LL, L"[IPC]Send: IPCLoadPluginTask uri:%s ,appid:%d", uriName(head_.uri), plugin_.appid);
	int res = ipc()->send(plugin_.connId,(const unsigned char*)pk.data(), pk.size());
	if (res == 0)
	{
		return STATE_RESPONSE;
	}
	else
	{//failure
		signalResult(&plugin_, taskId(), (head_.uri==kIPC_URI_LOAD_PLUGIN)?kIPC_PS_LOAD_FAIL:kIPC_PS_UNLOAD_FAIL,res);
		return STATE_ERROR;
	}
}

int IPCLoadPluginTask::processResponse()
{
	return done_?STATE_DONE:STATE_BLOCKED;
}

bool IPCLoadPluginTask::onEngineHandler(const unsigned char* data,size_t len)
{
	// reponse,if true,then insert pack to queue
	QY_UNUSED(data);
	QY_UNUSED(len);

	sox::Unpack un((const void *)data, len);
	IPCEnginePackHead_t head;
	head.unmarshal(un);

	if (kIPC_PT_APP != head.type)
	{
		return false;
	}
    if (head_.uri+1 != head.uri)
    {
        return false;
    }

	IPCPlugin_t plugin;
	plugin.unmarshal(un);

	if (plugin_.appid != plugin.appid)
	{
		return false;
	}
	if (head_.taskid != head.taskid)
	{
		return false;
	}
	QY_LOG(qy::kQyLogDev, qy::kQyLogInfo_LL, L"[IPC]Response: IPCLoadPluginTask, uri:%s ,appid:%d", uriName(head.uri), plugin_.appid);

	IPCResult_t result;
	result.unmarshal(un);
    signalResult(&plugin_, taskId(), result.status,result.error);
	done_ = true;
    wake();

	return true;
}
//
IPCListenPluginTask::IPCListenPluginTask(TASK_CONSTRUCT_PARAM)
:IPCTask(parent,ipc,level,id)
{
	head_.type = kIPC_PT_APP;
}

int IPCListenPluginTask::processStart()
{
	if (connStatus_.isClosed())
	{
		return STATE_ERROR;
	}
    if (connStatus_.isConnecting())
    {
        return STATE_BLOCKED;
	}

	const IPCEnginePack_PTR pack = queue_.next();
	if (NULL==pack)
	{
		return STATE_BLOCKED;
	}
	// notify pack to
	signalPlguinListen(pack);

	delete pack;
	return STATE_START;
}

bool IPCListenPluginTask::onEngineHandler(const unsigned char* data,size_t len)
{
	QY_UNUSED(data);
	QY_UNUSED(len);

	sox::Unpack un((const void *)data, len);
	IPCEnginePackHead_t head;
	head.unmarshal(un);

	if (kIPC_PT_APP != head.type)
	{
		return false;
	}
	switch(head.uri)
	{
	case kIPC_URI_UNLOAD_PLUGIN:
	case kIPC_URI_LOAD_PLUGIN:
		{// insert queue
			queue_.push(head, data, len);
		}
		break;
	default:
		return false;
	}
	wake();
	return true;
}
//
IPCListenPluginServiceTask::IPCListenPluginServiceTask(TASK_CONSTRUCT_PARAM,unsigned int appid)
:IPCTask(parent,ipc,level,id)
,appid_(appid)
,destroy_(false)
{

}

IPCListenPluginServiceTask::~IPCListenPluginServiceTask()
{
}

void IPCListenPluginServiceTask::destroy()
{
	destroy_ = true;
}

bool IPCListenPluginServiceTask::onEngineHandler(const unsigned char* data,size_t len)
{
	// if true,then insert pack to queue
	QY_UNUSED(data);
	QY_UNUSED(len);

	sox::Unpack un((const void *)data, len);
	IPCEnginePackHead_t head;
	head.unmarshal(un);

	if (head.fromid != appid_) 
	{
		return false;
	}
	if (head.toid == 0)
	{
		head.toid = head.fromid;
	}
   
	const uchar* raw = 0;
	uint32 rawlen = 0;
	switch(head.type)
	{
    case kIPC_PT_VIDEO:
	case kIPC_PT_SERVER:
    case kIPC_PT_DATA:
		{
            un >> rawlen;
			raw = (const uchar*)un.data();
		    if (un.size() < rawlen)
            {
                QY_LOG(qy::kQyLogFinal, qy::kQyLogErr_LL, L"[IPC]Recv: data packet error,data size is not match,appid:%d", appid_);
                return true;
            }
        }
		break;
        	
	case kIPC_PT_PLUGIN:
		{
			raw = data;
			rawlen = len;
		}
		break;
        
	default:
		return false;
	}
	QY_LOG(qy::kQyLogDev, qy::kQyLogInfo_LL, L"[IPC]Recv: packet,size=%d, type:%s ,appid:%d",len, typeName(head.type), appid_);
	//read raw data
	queue_.push(head, raw, rawlen);
	wake();
	return true;
}

int IPCListenPluginServiceTask::processStart()
{
	if (destroy_)		
    {	
		return STATE_DONE;
	}

	const IPCEnginePack_PTR pack = queue_.next();
	if (NULL==pack)
	{
		return STATE_BLOCKED;
	}
	// notify pack to
	signalRecv(appid_,pack);

	delete pack;
	return STATE_START;
}

//
IPCPluginRawTask::IPCPluginRawTask(TASK_CONSTRUCT_PARAM,unsigned int appid)
:IPCTask(parent,ipc,level,id)
,appid_(appid)
{
}

void IPCPluginRawTask::setRaw(const char* data,size_t rawlen)
{
	pack_.raw.append(data,rawlen);
}

int IPCPluginRawTask::processStart()
{
	if (connStatus_.isClosed())
	{		
		return STATE_ERROR;
	}
    else if(connStatus_.isConnecting())
    {
        return STATE_BLOCKED;
	}
	pack_.head = head_;

    sox::PackBuffer buffer;
    sox::Pack pk(buffer);
	pack_.marshal(pk);

	QY_LOG(qy::kQyLogDev, qy::kQyLogInfo_LL, L"[IPC]Send: Raw data,size=%d, type:%s ,appid:%d", pk.size(), typeName(head_.type), appid_);
	int res = ipc()->send(connId_,(const unsigned char*)pk.data(), pk.size());
	if (res != kIPC_RES_OK)
	{
		QY_LOG(qy::kQyLogFinal, qy::kQyLogErr_LL, L"[IPC]Send Fail: Raw data ,size=%d, type:%s ,appid:%d,err:%d", pk.size(), typeName(head_.type), appid_, res);
		return STATE_ERROR;
	}
	return STATE_DONE;
}

/** 直接发包 */
int IPCPacket::sendQuitExe(IPCObject* ipc,const CONNECTID& connId)
{
	IPCEnginePackHead_t  head;

	head.type = kIPC_PT_APP;
	head.uri  = kIPC_URI_QUIT;
	head.reserve = ::GetCurrentProcessId();
	sox::PackBuffer buffer;
	sox::Pack pk(buffer);
	head.marshal(pk);
	
	int res = ipc->send(connId,(const unsigned char*)pk.data(), pk.size());
	if (res != kIPC_RES_OK)
	{
		QY_LOG(qy::kQyLogFinal, qy::kQyLogErr_LL, L"[IPC]Send Fail: quit packet,err:%d", res);
	}
	return res;
}

IPCSynWait::IPCSynWait(IPCObject* ipc, void* obj)
:object_(obj)
,ipc_(ipc)
,running_(false)
,code_(0)
{
}

int IPCSynWait::exec()
{
    running_ = true;
    MSG msg;
    while (::GetMessage(&msg, NULL, 0, 0))
    {
        if (WM_QUIT == msg.message)
        {
            return -1;
        }
        
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        
        if (!running_)
        {
            return code_;
        }
    }
    return -1;
}

void IPCSynWait::quit(int code)
{
    code_ = code;
    running_ = false;
    ipc_->ipcPost(kIPC_MSGID_SYNWAIT,0,0,NULL,NULL);
}

void* IPCSynWait::object() const
{
    return object_;
}

