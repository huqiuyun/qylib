#pragma once

#include "ipc-engine-handler.h"
#include "ipc-pipe-base.h"
#include "iocp.h"
#include "qyutil/qyscoped_ptr.h"
#include <map>

class IPCEngineHandlerList;
class IPCEngine
{
public:
    IPCEngine();
    ~IPCEngine();

public:
    int  start();
    void stop();

    int createClient(const IPCConnectConfig_t& cfg);
    int createServer(const IPCConnectConfig_t& cfg);
    void destroy(const CONNECTID_PTR connId);

    bool isConnected(const CONNECTID_PTR connId);
    //
    void addEngineHandler(const CONNECTID_PTR connId,IPCEngineHandler* handler,eEngineHandlerLevel level);
    void removeEngineHandler(const CONNECTID_PTR connId,IPCEngineHandler* handler);

    //
    void onRecv(const CONNECTID_PTR connId,const unsigned char* data,size_t len);
    void onStatus(const CONNECTID_PTR connId,int status,int err);
    void clearHandler();

    //
    int  send(const CONNECTID_PTR connId,const unsigned char* data,size_t len);
private:
    typedef std::map<CONNECTID,IPCEngineHandlerList*> HANDLERMap_t;
    HANDLERMap_t     handlers_;
	qy::QySLCS       crit_;
    scoped_ptr<IOCP> iocp_;
};
