#include "ipc-engine.h"
#include "ipc-pipe-client.h"
#include "ipc-pipe-server.h"
#include "ipc-engine-packet.h"

class IPCEngineHandlerList
{
public:
    IPCEngineHandlerList()
    {
    }

    void add(IPCEngineHandler* handler,eEngineHandlerLevel level)
    {
        qy::QyAutoLocker cs(&crit_);
        HANDLERList_t& h = handlers_[level];
        h.push_back(handler);
    }

    void remove(IPCEngineHandler* handler)
    {
        qy::QyAutoLocker cs(&crit_);
        for (int i= 0; i < kEHL_END; i++){
            HANDLERList_t& h = handlers_[i];
			h.erase(std::remove(h.begin(), h.end(), handler), h.end());     
        }
    }

    void onHandler(const unsigned char* data,size_t len)
    {
		qy::QyAutoLocker cs(&crit_);
        for (int i = kEHL_END-1; i>= 0 ; i--){
            HANDLERList_t& h = handlers_[i];

            HANDLERList_t::iterator iter = h.begin();
            while (iter != h.end()) {
				if ((*iter)->onEngineHandler(data,len)) {
                    return;
                }
                ++iter;
            }
        }
		return;
    }

    void onStatus(int status,int err)
    {
        qy::QyAutoLocker cs(&crit_);
        for (int i = kEHL_END-1; i>= 0 ; i--){
            HANDLERList_t& h = handlers_[i];

            HANDLERList_t::iterator iter = h.begin();
            while (iter != h.end()){
                (*iter)->onEngineStatus(status,err);
                ++iter;
            }
        }
    }

    typedef std::list<IPCEngineHandler*> HANDLERList_t;
    HANDLERList_t handlers_[kEHL_END];
	qy::QySLCS    crit_;
};


IPCEngine::IPCEngine()
:iocp_(new IOCP())
{
}

IPCEngine::~IPCEngine()
{
    clearHandler();
}

int IPCEngine::start()
{
    return iocp_->open();
}

void IPCEngine::stop()
{
    iocp_->close();
}

int IPCEngine::createClient(const IPCConnectConfig_t& cfg)
{	
	IPCPipeClient* client = new IPCPipeClient(cfg,this);
	int ret = iocp_->attach(client);
    if (kIPC_RES_OK != ret){
	    delete client;
    }
    return ret;
}

int IPCEngine::createServer(const IPCConnectConfig_t& cfg)
{
	IPCPipeServer* server = new IPCPipeServer(cfg,this);
	int ret = iocp_->attach(server);
    if (kIPC_RES_OK != ret){
        delete server;
	}
	return ret;
}

void IPCEngine::destroy(const CONNECTID_PTR connId)
{
	iocp_->deatch(connId);
}

bool IPCEngine::isConnected(const CONNECTID_PTR connId)
{
	return iocp_->check(connId);
}

void IPCEngine::addEngineHandler(const CONNECTID_PTR connId,IPCEngineHandler* handler,eEngineHandlerLevel level)
{
    qy::QyAutoLocker cs(&crit_);

	IPCEngineHandlerList* li = 0;
    HANDLERMap_t::iterator iter = handlers_.find(connId);
    if (iter != handlers_.end()){
        li = iter->second;
    }
    else {
        li = new IPCEngineHandlerList();
        handlers_.insert(std::make_pair(connId,li));
    }

    li->add(handler,level);
}

void IPCEngine::removeEngineHandler(const CONNECTID_PTR connId,IPCEngineHandler* handler)
{
    qy::QyAutoLocker cs(&crit_);
    HANDLERMap_t::iterator iter = handlers_.find(connId);
    if (iter != handlers_.end()){
        iter->second->remove(handler);
    }
}

void IPCEngine::onRecv(const CONNECTID_PTR connId,const unsigned char* data,size_t len)
{
    qy::QyAutoLocker cs(&crit_);
    HANDLERMap_t::iterator iter = handlers_.find(connId);
    if (iter != handlers_.end()){
        iter->second->onHandler(data,len);
    }
}

void IPCEngine::onStatus(const CONNECTID_PTR connId,int status,int err)
{
    qy::QyAutoLocker cs(&crit_);
    HANDLERMap_t::iterator iter = handlers_.find(connId);
    if (iter != handlers_.end()) {
        iter->second->onStatus(status,err);
    }
}

void IPCEngine::clearHandler()
{
	qy::QyAutoLocker cs(&crit_);
    HANDLERMap_t::iterator iter = handlers_.begin();
    while (iter != handlers_.end())
    {
        delete iter->second;
        ++iter;
    }
    handlers_.clear();
}

int IPCEngine::send(const CONNECTID_PTR connId,const unsigned char* data,size_t len)
{
	return iocp_->send(connId,data,len);
}
