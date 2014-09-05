#include "ipc-object.h"
#include "ipc-object-p.h"
#include "ipc-msg-wnd.h"

#include "qyutil/qycmdline.h"
#include "qyutil/qylog.h"
// test code
#include "unittest/ipc-unittest.h"
//-----------------------------------------------------------------------------------------//
// IPCObjectPrivate
IPCObjectPrivate::IPCObjectPrivate(IPCObject* q)
:q_ptr_(q)
{
    mainConnectId_ = kCONNECTID_UNK; 
    query_method_ = NULL;
	client_ = true;
	msgmgr_ = new qy::QyMessageQueueManager();
	thread_ = new qy::QyMessageThread(NULL,msgmgr_);
	timermagr_ = new qy::QyTimeManager();
	wndmsg_ = new qy::QyWndMessage();

	taskrunner_ = new IPCTaskRunner(thread_);
	engine_ = new IPCEngine();
	pluginmgr_ = new IPCPluginMgr(q_ptr_);
	dispatchmgr_ = new IPCDispatchMgr(q_ptr_);
	msgWnd_ = new IPCMessageWnd(q_ptr_,wndmsg_);
}

IPCObjectPrivate::~IPCObjectPrivate()
{	
	delete timermagr_;
	delete taskrunner_;
	delete engine_;
	delete thread_;
	delete msgmgr_;
	delete pluginmgr_;
	delete msgWnd_;
	delete wndmsg_;
	delete dispatchmgr_;	
}

void IPCObjectPrivate::onMessage(qy::QyMessage *msg)
{
	switch(msg->message_id)
	{

	case kIPC_MSGID_CREATE_CLIENTCONNECT:
		{
			qy::TypedMessageData<IPCConnectConfig_t>* msgdata = static_cast<qy::TypedMessageData<IPCConnectConfig_t>*>(msg->data);
			int ret = engine_->createClient(msgdata->data()); //block...

			QY_LOG(qy::kQyLogDev, qy::kQyLogInfo_LL, L"[IPC]Create client connect...(ret:%d)",ret);
			delete msgdata;
		}
		break;

	case kIPC_MSGID_CREATE_SERVERCONNECT:
		{
			qy::TypedMessageData<IPCConnectConfig_t>* msgdata = static_cast<qy::TypedMessageData<IPCConnectConfig_t>*>(msg->data);
			int ret = engine_->createServer(msgdata->data());//block...

			QY_LOG(qy::kQyLogDev, qy::kQyLogInfo_LL,L"[IPC]Create server connect...(ret:%d)", ret);

			delete msgdata;
		}
		break;

	case kIPC_MSGID_DESTROY_CONNECT:
		{
			qy::TypedMessageData<CONNECTID>* msgdata = static_cast<qy::TypedMessageData<CONNECTID>*>(msg->data);
			engine_->destroy(CONNECTID_TO_PTR(msgdata->data()));

			delete msgdata;
		}
		break;

	case kIPC_MSGID_DESTROYALL_CONNECT:
		{
			engine_->stop();
		}
		break;

	case kIPC_MSGID_UNITTEST: //单元测试
		{
			qy::TypedMessageData<int>* msgdata = static_cast<qy::TypedMessageData<int>*>(msg->data);
			switch(msgdata->data())
			{
			case kIPC_TEST_MSG:
				{
					IPCMsgTest  test(q_ptr_);
				}
				break;

			case kIPC_TEST_LOAD_PLUGIN:
				{
					IPCLoadPluginTest test(q_ptr_);
				}
				break;

			case kIPC_TEST_UNLOAD_PLUGIN:
				{
					IPCUnLoadPluginTest test(q_ptr_);
				}
				break;
			}
			delete msgdata;
		}
		break;
	}
}

void IPCObjectPrivate::onConnectStatus(const CONNECTID_PTR connId,int st,unsigned int err)
{
	IPCConnectStatus_t status(st,err);
    if (status.isClosed())
    {
        destroyConnect(connId);
		setConnectKeyWithStatus(connId,status);
    }
    else if(status.isTaskEnd())
    {
        removeConnectKey(connId);
    }
    else if (status.isConnected() || status.isConnecting())
	{
		setConnectKeyWithStatus(connId,status);
	}

	IPCConnectResult_t res;
	res.connId = connId;
	res.result.status = st;
	res.result.error = err;
	q_ptr_->ipcPost(kIPC_MSGID_CONNECTSTATUS, 0, (LPARAM)new qy::TypedMessageData<IPCConnectResult_t>(res), NULL, IPCMessageWnd::fnRemove);
}

void IPCObjectPrivate::onConnectChild(const CONNECTID_PTR connId,const IPCConnectConfig_t* connCfg)
{
	q_ptr_->connectToServer(*connCfg);
}

void IPCObjectPrivate::onExeInstalled(const IPCExe_t* exe, const char* taskid, const IPCExeStatus_t* status)
{
	IPCAppResult_t res;
	res.exe = *exe;
	res.result.status = status->status;
	res.result.error = status->err;
	res.taskId = taskid;

	if (status->isRuned())
	{
		setExeKeyWithStatus(exe->uuid,*status);
	}
	else if (status->isFailed())
	{
		setExeKeyWithStatus(exe->uuid,*status);
	}
	else if (status->isTaskEnd())
	{
		removeExeKey(exe->uuid);
	}
	q_ptr_->ipcPost(kIPC_MSGID_APP, 0, (LPARAM)new qy::TypedMessageData<IPCAppResult_t>(res), NULL, IPCMessageWnd::fnRemove);
}

void IPCObjectPrivate::destroyConnect(const CONNECTID_PTR connId)
{
	thread_->post(this,kIPC_MSGID_DESTROY_CONNECT,new qy::TypedMessageData<CONNECTID>(connId));
}

void IPCObjectPrivate::destroyAllConnect()
{
	thread_->post(this,kIPC_MSGID_DESTROYALL_CONNECT,0);
}

void IPCObjectPrivate::addEngineHandler(const CONNECTID_PTR connId,IPCEngineHandler* handler,eEngineHandlerLevel level)
{
	engine_->addEngineHandler(connId,handler,level);
}

void IPCObjectPrivate::removeEngineHandler(const CONNECTID_PTR connId,IPCEngineHandler* handler)
{
	engine_->removeEngineHandler(connId,handler);
}

qy::QyTimeManager* IPCObjectPrivate::timer() const
{
	return timermagr_;
}

IPCTaskRunner* IPCObjectPrivate::taskRunner() const
{
	return taskrunner_;
}

IPCPluginMgr* IPCObjectPrivate::pluginMgr() const
{
	return pluginmgr_;
}

qy::QyMessageThread* IPCObjectPrivate::msg() const
{
	return thread_;
}

void IPCObjectPrivate::addListen(const CONNECTID& connId,bool client)
{
	if (isConnectKeyWithListen(CONNECTID_TO_PTR(connId)))
	{
		return ;
	}
	startConnectKeyWithListen(CONNECTID_TO_PTR(connId));
	{// public 
		IPCListenConnectTask* task = new IPCListenConnectTask(taskrunner_, q_ptr_, kEHL_APP, connId);
		task->signalConnectStatus.connect(this,&IPCObjectPrivate::onConnectStatus);
		task->signalConnectChild.connect(this,&IPCObjectPrivate::onConnectChild);
		task->start();
	}
    {
		IPCListenQueryTask* task = new IPCListenQueryTask(taskrunner_, q_ptr_, kEHL_APP, connId);
		task->start();
    }
	pluginmgr_->addListen(connId,client);
}


void IPCObjectPrivate::removeConnectKey(const CONNECTID_PTR connId)
{
	qy::QyAutoLocker cs(&key_crit_);
	ConnectKeyMap::iterator iter = connHash_.find(connId);
	if (iter != connHash_.end())
	{
		connHash_.erase(iter);
	}
}

void IPCObjectPrivate::startConnectKeyWithListen(const CONNECTID_PTR connId)
{
	qy::QyAutoLocker cs(&key_crit_);
	ConnectKeyMap::iterator iter = connHash_.find(connId);
	if (iter != connHash_.end())
	{
		iter->second.listen = true;
	}
	else
	{
		connHash_.insert(std::make_pair(connId,ConnectKey(true,IPCConnectStatus_t())));
	}
}

bool IPCObjectPrivate::isConnectKeyWithListen(const CONNECTID_PTR connId)
{
	qy::QyAutoLocker cs(&key_crit_);
	ConnectKeyMap::iterator iter = connHash_.find(connId);
	if (iter != connHash_.end())
	{
		return iter->second.listen;
	}
	return false;
}

IPCConnectStatus_t IPCObjectPrivate::getConnectKeyWithStatus(const CONNECTID_PTR connId)
{
	qy::QyAutoLocker cs(&key_crit_);
	ConnectKeyMap::iterator iter = connHash_.find(connId);
	if (iter != connHash_.end())
	{
		return iter->second.status;
	}
	return IPCConnectStatus_t(kIPC_CS_NOFIND,0);
}

void IPCObjectPrivate::setConnectKeyWithStatus(const CONNECTID_PTR connId,const IPCConnectStatus_t& status)
{
	qy::QyAutoLocker cs(&key_crit_);
	ConnectKeyMap::iterator iter = connHash_.find(connId);
	if (iter != connHash_.end())
	{
		iter->second.status = status;
	}
	else
	{
		connHash_.insert(std::make_pair(connId,ConnectKey(false,status)));
	}
}

CONNECTID IPCObjectPrivate::mainConnected()
{
    return mainConnectId_;
}

void IPCObjectPrivate::removeExeKey(const std::wstring& uuid)
{
	qy::QyAutoLocker cs(&key_crit_);
	ExeKeyMap::iterator iter = exeHash_.find(uuid);
	if (iter != exeHash_.end())
	{
		exeHash_.erase(iter);
	}
}

IPCExeStatus_t IPCObjectPrivate::getExeKeyWithStatus(const std::wstring& uuid)
{
	qy::QyAutoLocker cs(&key_crit_);
	ExeKeyMap::iterator iter = exeHash_.find(uuid);
	if (iter != exeHash_.end())
	{
		return iter->second.status;
	}
	return IPCExeStatus_t(kIPC_EXES_NOFIND,0,0);
}

void IPCObjectPrivate::setExeKeyWithStatus(const std::wstring& uuid,const IPCExeStatus_t& status)
{
	qy::QyAutoLocker cs(&key_crit_);
	ExeKeyMap::iterator iter = exeHash_.find(uuid);
	if (iter != exeHash_.end())
	{
		iter->second.status = status;
	}
	else
	{
		exeHash_.insert(std::make_pair(uuid,ExeKey(false,status)));
	}
}

void IPCObjectPrivate::startExeKeyWithTask(const std::wstring& uuid)
{
	qy::QyAutoLocker cs(&key_crit_);
	ExeKeyMap::iterator iter = exeHash_.find(uuid);
	if (iter != exeHash_.end())
	{
		iter->second.task = true;
	}
	else
	{
		exeHash_.insert(std::make_pair(uuid,ExeKey(true,IPCExeStatus_t())));
	}
}

bool IPCObjectPrivate::isExeKeyWithTask(const std::wstring& uuid)
{
	qy::QyAutoLocker cs(&key_crit_);
	ExeKeyMap::iterator iter = exeHash_.find(uuid);
	if (iter != exeHash_.end())
	{
		return iter->second.task;
	}
	return false;
}

//-----------------------------------------------------------------------------------------//
// IPCObject

IPCObject::IPCObject()
:d_ptr_(new IPCObjectPrivate(this))
{
}

IPCObject::~IPCObject()
{
	uninit();
	delete d_ptr_;
}

int IPCObject::startupClient(const IPCExe_t& exe,const char* taskid)
{
	QY_D(IPCObject);

	if (!d->isExeKeyWithTask(exe.uuid) &&
		!d->getExeKeyWithStatus(exe.uuid).isRuned())
	{
		d->startExeKeyWithTask(exe.uuid);
		IPCExeTask* task = new IPCExeTask(d->taskrunner_, this, kEHL_NONE, exe.connId, exe);
		task->signalResult.connect(d,&IPCObjectPrivate::onExeInstalled);
		task->setTaskId(taskid);
		task->start();		
	}	
	return kIPC_RES_OK;
}

int IPCObject::init(const IPCEnvironment_t& env, bool client)
{
	QY_D(IPCObject);

	if (!d->msgWnd_->createWindow(env.mainWnd))
	{
		return kIPC_RES_MSGWND_CREATEFAIL;
	}

	// 设置环境变量
	if (env.envdir.length()>0)
	{
		d->env_.add(env.envdir.c_str());
	}
	d->client_ = client;
	d->engine_->start();
	d->thread_->start();
    return kIPC_RES_OK;
}

int IPCObject::initServer(const IPCEnvironment_t& env)
{
    return init(env,false);
}

int IPCObject::createServer(const IPCConnectConfig_t& connCfg,const CONNECTID_PTR parentConnId)
{
	//AND create port,create server pipe
	QY_D(IPCObject);

	d->addListen(connCfg.connId,false);

	d->thread_->post(d,kIPC_MSGID_CREATE_SERVERCONNECT,new qy::TypedMessageData<IPCConnectConfig_t>(connCfg));

	if (parentConnId != kCONNECTID_UNK &&
		connectStatus(parentConnId).isConnected())
	{//
		IPCCreateChildConnectTask* task = new IPCCreateChildConnectTask(d->taskrunner_, this, kEHL_HIGH, parentConnId,connCfg);				
		task->start();		
	}
	return kIPC_RES_OK;
}

/** Step 1: client call */
int IPCObject::initClient(const wchar_t* exeuuid,const IPCEnvironment_t& env)
{
	QY_D(IPCObject);
	if (d->client_exe_exist_.checkWithHead(exeuuid))
	{
		return kIPC_RES_APP_EXISTED;
	}
	d->client_exe_exist_.initWithHead(exeuuid);

    return init(env,true);
}

int IPCObject::connectToServer(const IPCConnectConfig_t& connCfg)
{
	//AND create port,create client pipe,to connect to server
	QY_D(IPCObject);
	d->addListen(connCfg.connId,true);
	d->thread_->post(d,kIPC_MSGID_CREATE_CLIENTCONNECT,new qy::TypedMessageData<IPCConnectConfig_t>(connCfg));	
	return kIPC_RES_OK;
}

void IPCObject::close(const CONNECTID_PTR connId)
{
    QY_D(IPCObject);
    d->destroyConnect(connId);
}

void IPCObject::closeAll()
{
    QY_D(IPCObject);
    d->destroyAllConnect();
}

void IPCObject::uninit()
{
	QY_D(IPCObject);
	d->client_exe_exist_.uninit();
	d->env_.reset();
	d->timermagr_->destroy();
	d->pluginmgr_->destroy();
	d->engine_->stop();
	d->thread_->clear(NULL);
	d->thread_->stop();
	d->taskrunner_->abort();
}

int  IPCObject::send(const CONNECTID_PTR connId,const unsigned char* data,size_t len)
{
	QY_D(IPCObject);
	return d->engine_->send(connId,data,len);
}

bool IPCObject::isConnected(const CONNECTID_PTR connId) const
{
	QY_D(const IPCObject);
	return d->engine_->isConnected(connId);
}

IPCConnectStatus_t IPCObject::connectStatus(const CONNECTID_PTR connId) const
{
	QY_D(IPCObject);     
	return d->getConnectKeyWithStatus(connId);
}

IPCObjectPrivate* IPCObject::q_func() const
{
	return d_ptr_;
}

void IPCObject::setApplicationDir(const wchar_t* dir)
{
	QY_D(IPCObject);
	d->app_dir_ = dir;
}

const wchar_t* IPCObject::applicationDir() const
{
	QY_D(const IPCObject);
	return d->app_dir_.c_str();
}

void IPCObject::setUserConfig(const IPCUsrConfig_t& cfg)
{
	QY_D(IPCObject);
	d->usrCfg_ = cfg;
}

IPCUsrConfig_t IPCObject::usrConfig() const
{
	QY_D(const IPCObject);
	return d->usrCfg_;
}

IIPCPluginMgr* IPCObject::pluginMgr()
{
	QY_D(IPCObject);
	return (IIPCPluginMgr*)d->pluginmgr_;
}

IIPCDispatchMgr* IPCObject::dispatchMgr()
{
	QY_D(IPCObject);
	return (IIPCDispatchMgr*)d->dispatchmgr_;
}

bool IPCObject::cmdLine(const wchar_t* cmdLine, IPCExe_t& exe,
						IPCConnectConfig_t& connCfg, IPCEnvironment_t& env)
{
	// -uuid yyedu -cname huqy -cid 1
	QY_D(IPCObject);

	qy::QyCommandLine cmd;
	if (0 != cmd.parse(cmdLine))
	{
		return false;
	}
	connCfg.overlapped = true;
	qy::QyParameterMgr* params = cmd.parameters();
	for (int i = 0 ; i < params->count();i++) {
		if (wcscmp(params->name(i),L"uuid")==0) {
			exe.uuid = params->value(i);
		}
		else if(wcscmp(params->name(i),L"cname")==0) { // for connect Name,pipe Name
			connCfg.name = params->value(i);
		}
		else if(wcscmp(params->name(i),L"cid")==0) {// for connect id ,pipe id
			connCfg.connId = WSTR_TO_CONNECT_ID(params->value(i));
		}
		else if(wcscmp(params->name(i),L"cmd")==0) {// for server
			exe.cmdline = params->value(i);
		}
		else if(wcscmp(params->name(i),L"curdir")==0){
			exe.curdir = params->value(i);
		}
		else if(wcscmp(params->name(i),L"envdir")==0) {
			env.envdir = params->value(i);
		}
		//业务的特殊性
		else if(wcscmp(params->name(i),L"userid")==0) {
			d->usrCfg_.uid = params->toULong(i);
		}
		else if(wcscmp(params->name(i),L"yyid")==0) {
			d->usrCfg_.yyid = params->toULong(i);
		}
		else if(wcscmp(params->name(i),L"chrole")==0) {//频道角色
			d->usrCfg_.ch_role = params->toULong(i);
		}
		else if(wcscmp(params->name(i),L"chsubsid")==0) {//子频道ID
			d->usrCfg_.ch_subsid = params->toULong(i);
		}
		else if(wcscmp(params->name(i),L"chasid")==0) { //短频道id
			d->usrCfg_.ch_asid = params->toULong(i);
		}
		else if(wcscmp(params->name(i),L"chtopsid")==0) { //频道顶级id
			d->usrCfg_.ch_topsid = params->toULong(i);
		}
	}
	return true;
}

void IPCObject::setQueryMethod(IIPCQueryMethod* method) 
{
    QY_D(IPCObject);
    d->query_method_ = method;
}

int IPCObject::query(const char* cmd, const char* usrdata, IPCQueryResult_t* result)
{
    QY_D(IPCObject);

    IPCSynWait wait(this,(void*)result);
    IPCQuery_t req;
    req.cmd = cmd;
    req.usrdata = usrdata ? usrdata:"";
    IPCQueryTask* task = new IPCQueryTask(d->taskRunner(),this, kEHL_HIGH, d->mainConnectId(),req, &wait);
	task->set_timeout_seconds(4000);
    task->start();

    //wait,hook the current thread
    return wait.exec();
}

bool IPCObject::ipcPost(unsigned int msgid, WPARAM wParam, LPARAM lParam, qy::FnMsgPush fnPush, qy::FnMsgRemove fnRemove)
{//可以
    QY_D(IPCObject);
	return d->msgWnd_->post(msgid,wParam,lParam,fnPush,fnRemove);
}

