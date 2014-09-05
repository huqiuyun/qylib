#pragma once

#include <string>
#include "ipc-config.h"

#ifndef kCONNECTID_UNK
#define kCONNECTID_UNK                0xFFFFFFFF 

typedef unsigned int                  CONNECTID;
typedef unsigned int                  CONNECTID_PTR;
#define CONNECTID_TO_PTR(connId)      (const CONNECTID_PTR)(connId)
#define WSTR_TO_CONNECT_ID(connId)    wcstol((connId),NULL,10)
#define CONNECTID_OK(connId)          ((connId) >0 && (connId) != 0xFFFFFFFF)

#endif //CONNECTID_Unk

const unsigned int kPACKET_BUFFER_MAX = 64*1024;
const unsigned int kIPC_LOADPLUGIN_TIMOUT = 310000;//30秒
enum eIPCResult
{
	kIPC_RES_OK = 0,
	kIPC_RES_NOTFIND_CONNECTED = 4000,
	kIPC_RES_NOTFIND_ENTRY = 4001,
	kIPC_RES_CALLENTRY_FAIL = 4002,
    kIPC_RES_HOOK = 4003,
    kIPC_RES_NOTFIND_PLUGIN = 4004,
	kIPC_RES_MSGWND_CREATEFAIL = 4005,
	kIPC_RES_TIMEOUT = 4006,
    kIPC_RES_APPINSTALL_FAIL = 4007,
	kIPC_RES_APP_EXISTED = 4008,
	kIPC_RES_QUIT = 4009,
    kIPC_RES_PIPE_EXISTED = 4010,
    kIPC_RES_CREATE_IO_FAIL = 4011,
	kIPC_RES_LOSSDATA = 4012,
    kIPC_RES_FAIL = 4013,
    kIPC_RES_NOINTERFACE = 4014,
};

/** 内部消息id */
enum eIPCMsgId
{
	kIPC_MSGID_UNK = 0,
	kIPC_MSGID_UNITTEST = 1,
	kIPC_MSGID_QUIT = 2,
	kIPC_MSGID_CREATE_CLIENTCONNECT = 3, ///< create client pipe ,go to connect server
	kIPC_MSGID_CREATE_SERVERCONNECT = 4, ///< create server pip , wait for client connect, AND block current thread 5 second.
	kIPC_MSGID_DESTROY_CONNECT = 5,
	kIPC_MSGID_DESTROYALL_CONNECT = 6,
	kIPC_MSGID_CONNECTSTATUS = 7,
	kIPC_MSGID_APP = 8,
	kIPC_MSGID_PLUGINLISTEN = 9,
	kIPC_MSGID_PLUGINSTATUS = 10,
	kIPC_MSGID_PLUGINDATA = 11,
	kIPC_MSGID_CHECKDLL = 12,
    kIPC_MSGID_SYNWAIT = 13,
};

enum eIPCUnitTestId
{
	kIPC_TEST_MSG = 0,
	kIPC_TEST_LOAD_PLUGIN = 1,
	kIPC_TEST_UNLOAD_PLUGIN = 2,
};

enum eIPCExeStatus
{
	kIPC_EXES_UNK=-1,
	kIPC_EXES_OK = 0,
	kIPC_EXES_NOFIND = 1,
    kIPC_EXES_EXISTED = 2,
	kIPC_EXES_INSTALL_FAIL = 3,
	kIPC_EXES_QUIT = 4,
    kIPC_EXES_CRASH = 5,
    kIPC_EXES_TASK_END
};

enum eIPCPluginStatus
{
	kIPC_PS_LOAD_OK = 0,
	kIPC_PS_START_LOAD = 1, ///< begin to start plugin ,for IPCPluginWaitTask
	kIPC_PS_LOAD_FAIL = 2,

	kIPC_PS_UNLOAD_OK = 3,
	kIPC_PS_UNLOAD_FAIL = 4,	
};

enum eIPCConnectStatus
{
	kIPC_CS_UNK = 0,
	kIPC_CS_NOFIND = 1,
    kIPC_CS_SERVER_CONNECTING = 2, ///< Wait for the client to connect. 
	kIPC_CS_SERVER_CONNECTED = 3, ///< connected from client
	
	kIPC_CS_CLIENT_CONNECTING = 4,
	kIPC_CS_CLIENT_CONNECTED = 5,

	kIPC_CS_SERVER_ERROR = 6,
	kIPC_CS_SERVER_LOSTDATA = 7,

	kIPC_CS_CLIENT_ERROR = 8,
	kIPC_CS_CLIENT_LOSTDATA = 9,

	kIPC_CS_PEER_CLOSE = 10,
	kIPC_CS_CLOSE = 11,
	kIPC_CS_ERROR = 12,
	kIPC_CS_LISTENTASK_END
};

enum eIPCPacketType
{
	kIPC_PT_DATA = 0, ///<程序内部定义类型
	kIPC_PT_PLUGIN = 1, ///<插件间的透传
	kIPC_PT_SERVER = 2,///< 服务器间类型
	kIPC_PT_APP = 3, ///< 应用程序间类型（级别上，是最高）
	kIPC_PT_VIDEO = 4,
};

enum eIPCPacketUri
{
	kIPC_URI_UNK = 0,
	kIPC_URI_LOAD_PLUGIN = 1, ///< load dll plugin
    kIPC_URI_LOAD_PLUGIN_RES = 2,

	kIPC_URI_UNLOAD_PLUGIN = 3, ///< unload dll plugin
	kIPC_URI_UNLOAD_PLUGIN_RES = 4, ///< unload dll plugin
	
    kIPC_URI_CREATE_CHILDCONNECT = 5, ///< 创建一个子链路
	kIPC_URI_QUIT = 6, ///< 客户端正常退出
    
    kIPC_URI_QUERY = 7, ///< 查询接口
    kIPC_URI_QUERY_RES = 8,
};

enum eIPCQuitFlag
{
	kIPC_QUIT_FLAG_UNK  = 0,
	kIPC_QUIT_FLAG_CRASH = 1,
	kIPC_QUIT_FLAG_NORMAL = 2,
};

