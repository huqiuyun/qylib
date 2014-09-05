#pragma once

#include "ipc-define.h"
#include "sox/packet.h"

typedef struct tagIPCUserConfig
{
	unsigned long uid;
	unsigned long yyid; // = im id
	unsigned long ch_role;
	unsigned long ch_subsid;
	unsigned long ch_asid;
	unsigned long ch_topsid;

	tagIPCUserConfig()
	{
		uid = 0;
		yyid = 0;
		ch_role = 0;
		ch_subsid = 0;
		ch_asid = 0;
		ch_topsid = 0;
	}
}IPCUsrConfig_t,*IPCUsrConfig_PTR;

/** 链路配置 */
typedef struct tagIPCConnectConfig
{
	CONNECTID    connId; ///<  connect id,唯一标识  
	int          counts; ///< 一个管道名字，可以建多少个实例
    bool         overlapped;
	std::wstring name; ///< 名字，PIPE NAME
	
    tagIPCConnectConfig()
	{
		connId = kCONNECTID_UNK;
        counts = 1;
	}

	void marshal(sox::Pack &pk) const
	{		
		pk << connId;
		pk << counts;
        pk << overlapped;
		pk << name;
	}

	void unmarshal(const sox::Unpack & up)
	{
		up >> connId;
		up >> counts;
        up >> overlapped;
		up >> name;
	}
}IPCConnectConfig_t, *IPCConnectConfig_PTR;

typedef struct tagIPCConnectStatus
{
    int status;
    unsigned int err;

    tagIPCConnectStatus()
    {
        status = kIPC_CS_UNK;
        err = 0;
    }

    tagIPCConnectStatus(int st,unsigned int e)
    {
        status = st;
        err = e;
    }

    bool isConnecting() const
    {
        return (kIPC_CS_CLIENT_CONNECTING == status ||
                kIPC_CS_SERVER_CONNECTING == status);
    }

    bool isConnected() const
    {
        return (kIPC_CS_CLIENT_CONNECTED == status ||
                kIPC_CS_SERVER_CONNECTED == status);
    }

    bool isClosed() const
    {
        return (kIPC_CS_ERROR == status ||
                kIPC_CS_PEER_CLOSE == status ||
                kIPC_CS_CLOSE == status ||
                kIPC_CS_CLIENT_ERROR == status ||
                kIPC_CS_SERVER_ERROR == status);

    }
    bool isPeerClosed() const
    {
        return (kIPC_CS_PEER_CLOSE == status);
    }
   
    bool isLoss() const
    {
        return (kIPC_CS_CLIENT_LOSTDATA == status ||
                kIPC_CS_SERVER_LOSTDATA == status);
    }

    bool isNoFind() const
    {
        return (kIPC_CS_NOFIND == status);
    }
    bool isTaskEnd() const
	{
		return (kIPC_CS_LISTENTASK_END == status);
	}
    unsigned int error() const
    {
        return err;
    }
}IPCConnectStatus_t,*IPCConnectStatus_PTR;

/** 插件dll,exe */
typedef struct tagIPCPlugin
{
	//如 10051,10039,10085...
	unsigned int appid;
    std::wstring uuid;
	std::wstring file; ///< dll filename
    std::wstring curdir;
    std::wstring cmdline;

	CONNECTID    connId; ///< 链路ID，插件通过哪条链路来通信，最由它指定

    tagIPCPlugin()
    {
        appid = 0;
        connId = kCONNECTID_UNK;
    }

	void marshal(sox::Pack &pk) const
	{		
		pk << appid;
		pk << file;
        pk << curdir;
		pk << cmdline;
		pk << connId;
	}

	void unmarshal(const sox::Unpack & up)
	{
		up >> appid;
		up >> file;
        up >> curdir;
		up >> cmdline;
		up >> connId;
	}
}IPCPlugin_t,*IPCPlugin_PTR, IPCExe_t, *IPCEXE_PTR;

typedef struct tagIPCEnvironment
{
	HWND mainWnd;
	std::wstring envdir;
	
	tagIPCEnvironment()
	{
		mainWnd = NULL;
	}
}IPCEnvironment_t,*IPCEnvironment_PTR;

typedef struct tagIPCExeStatus
{
    int status;
    unsigned int error;
	unsigned int processId;

    tagIPCExeStatus()
    {
		processId = 0;
		status = kIPC_EXES_UNK;
		error = 0;
    }

    tagIPCExeStatus(int st,unsigned int e,unsigned int proid)
    {
        status = st;
        error = e;
		processId = proid;
    }

	bool isCrash() const 
	{
		return (kIPC_QUIT_FLAG_CRASH == status);
	}

    bool isRuned() const
    {
        return (kIPC_EXES_OK == status);
    }

    bool isFailed() const
    {
        return (kIPC_EXES_INSTALL_FAIL == status ||
                kIPC_EXES_QUIT == status ||
                kIPC_EXES_CRASH == status);
	}

    bool isNoFind() const
    {
        return (kIPC_EXES_NOFIND == status);
    }

    bool isTaskEnd() const
    {
        return (kIPC_EXES_TASK_END == status);
    }

}IPCExeStatus_t,*IPCExeStatus_PTR;

/** 协议包头*/
typedef struct tagIPCEnginePackHead
{
	int type; ///< server, data(js,xml,bytes...) @see eIPCPacketType
	int uri; ///<  @see eIPCPacketUri
	unsigned int fromid; ///< plugin id,10051,10039,10085
	unsigned int toid;   ///< = fromid, 如果没有特别的需求是相同的
	unsigned int reserve; ///< 保留字段
	std::string  taskid;
	
	tagIPCEnginePackHead()
	{
		fromid = 0;
		toid = 0;
		type = kIPC_PT_DATA;
		uri = kIPC_PURI_UNK;
		reserve = 0;
	}

	void marshal(sox::Pack &pk) const
	{		
		pk << type;
		pk << uri;
		pk << fromid;
		pk << toid;
		pk << reserve;
		pk << taskid;
	}

	void unmarshal(const sox::Unpack & up)
	{
		up >> type;
		up >> uri;
		up >> fromid;
		up >> toid;
		up >> reserve;
		up >> taskid;
	}
}IPCEnginePackHead_t, *IPCEnginePackHead_PTR;

typedef struct tagIPCResult
{
	int status; ///< result
	unsigned int error;  ///< error
    unsigned int reserve;
	tagIPCResult()
	{
		status = 0;
		error = 0;
        reserve = 0;
	}
	void marshal(sox::Pack &pk) const
	{		
		pk << status;
		pk << error;
        pk << reserve;
	}

	void unmarshal(const sox::Unpack & up)
	{
		up >> status;
		up >> error;
        up >> reserve;
	}
}IPCResult_t,*IPCResult_PTR;

typedef struct tagIPCEnginePack
{
	IPCEnginePackHead_t head;
	std::string  raw; //不能当普通字符串处理,bytes

	void marshal(sox::Pack &pk) const
	{		
		head.marshal(pk);
		pk.push_varstr32(raw);
	}

	void unmarshal(const sox::Unpack & up)
	{
		head.unmarshal(up);
		raw = up.pop_varstr32();
	}
}IPCEnginePack_t , *IPCEnginePack_PTR;

typedef struct tagIPCPluginPack
{
	IPCEnginePackHead_t head;
	IPCPlugin_t plugin;
	IPCResult_t result;
	void marshal(sox::Pack &pk) const
	{		
		head.marshal(pk);
		plugin.marshal(pk);
		result.marshal(pk);
	}

	void unmarshal(const sox::Unpack & up)
	{
		head.unmarshal(up);
		plugin.unmarshal(up);
		result.unmarshal(up);
	}
}IPCPluginPack_t,*IPCPluginPack_PTR;

typedef struct tagIPCQuery
{
    std::string cmd;
    std::string usrdata;///< 自定义数据

    tagIPCQuery
    {
    }

    void marshal(sox::Pack &pk) const
    {
        pk << cmd;
        pk << usrdata;
    }

    void unmarshal(sox::Unpack &up)
    {
        up >> cmd;
        up >> usrdata;
    }
}IPCQuery_t,*IPCQuery_PTR;

typedef struct tagIPCQureyReslut
{
    std::string result_data;
    IPCResult_t result;

    void marshal(sox::Pack &pk) const
    {
        pk << result_data; 
        result.marsha(pk);
    }

    void unmarshal(sox::Unpack &up)
    {
        up >> result_data;
        result.unmarshal(up);
    }

}IPCQueryResult_t,*IPCQueryResult_PTR;

typedef struct tagIPCConnectResult
{
    CONNECTID connId;
    IPCResult_t  result;
}IPCConnectResult_t,*IPCConnectResult_PTR;


typedef struct tagIPCAppResult
{
	std::string  taskId;
    IPCExe_t     exe;
    IPCResult_t  result;
}IPCAppResult_t,*IPCAppResult_PTR;


typedef struct tagIPCPluginResult
{   
	std::string  taskId;
	IPCPlugin_t  plug;
    IPCResult_t  result;
}IPCPluginResult_t,*IPCPluginResult_PTR;


