#include "ipc-dll.h"
#include <qwidget>

class IPCServerDispatch : public IIPCDispatch
{
public:
	IPCServerDispatch(IIPCApplication* app,IIPCApi* ipc)
		:app_(app)
		,ipc_(ipc)
	{
		ipc_->dispatchMgr()->addDispatch(this,this);
	}
	
	void onExeStatus(const IPCExe_t*, int, int)
	{//
	}

	void onConnectStatus(const CONNECTID_PTR connId,int status ,int err)
	{
		switch (status)
		{
		case kIPC_CS_SERVER_CONNECTING:
			OutputDebugStringW(L"Wait for connected from client...server\n\n");
			break;

		case kIPC_CS_SERVER_CONNECTED:
			OutputDebugStringW(L"Connected from client ok,server\n\n");
			break;

			// error
		case kIPC_CS_SERVER_ERROR:
			OutputDebugStringW(L"Create connect failure.,sever\n\n");//need to quit application ?
			app_->quit();
			break;

		case kIPC_CS_PEER_CLOSE:
			OutputDebugStringW(L"connect close,peer\n\n");
			app_->quit();
			break;

		case kIPC_CS_CLOSE:
			OutputDebugStringW(L"connect close,own\n\n");
			app_->quit();
			break;

		case kIPC_CS_ERROR:
			OutputDebugStringW(L"connect error\n\n");
			app_->quit();
			break;
		}
	}
	//
	void onPluginStatus(unsigned int, int, int)
	{
	}

	void onPluginRecv(unsigned int appid, const IPCEnginePackHead_t* head, const char* raw, size_t rawlen)
	{
	}
private:
	IIPCApplication* app_;
	IIPCApi* ipc_;
};

int main(int argc, char *argv[])
{
	IPCDll ipcdll;
	if (kIPC_PS_LOAD_OK != ipcdll.load(L"eduipc.dll"))
	{
		return -1;
	}
	IPCServerDispatch dispatch(ipcdll.fnApplication(),ipcdll.fnObject());
    
	IPCUsrConfig_t usr_t;
	IPCExe_t exe_t;
	IPCConnectConfig_t connCfg_t;
	IPCEnvironment_t env_t;
	if (!ipcdll.fnApplication()->cmdLine(GetCommandLineW(),exe_t,connCfg_t, env_t,usr_t))
	{
		ipcdll.fnApplication()->uinit();
		return -2;
	}

	if (kIPC_RES_OK != ipcdll.fnObject()->startClient(exe_t))
	{
		ipcdll.fnApplication()->uinit();
		return -3;
	}

	if (kIPC_RES_OK != ipcdll.fnObject()->createServer(connCfg_t,env_t))
	{
		ipcdll.fnApplication()->uinit();
		return -4;
	}

    QWidget window;   
    window.resize(300, 200);
	window.setWindowTitle("server");
	window.show();

	ipcdll.fnApplication()->exec();
	ipcdll.fnApplication()->uinit();

	return 0;
}
