#include "ipc-dll.h"
#include <qwidget>
#include <string>

class IPCClientDispatch : public IIPCDispatch
{
public:
	IPCClientDispatch(IIPCApplication* app,IIPCApi* ipc)
		:app_(app)
		,ipc_(ipc)
	{
		ipc_->dispatchMgr()->addDispatch(this,this);
	}

	void onExeStatus(const IPCExe_t* exe, int status, int err)
	{// for client
		switch(status)
		{
		case kIPC_EXES_EXISTED:// to do check app pipe is existed
			OutputDebugStringA("Application process existed\n\n");
			break;
		case kIPC_EXES_OK: //create pipe,and go to connect
			OutputDebugStringA("Application process startup\n\n");
			break;
			break;
		case kIPC_EXES_INSTALL_FAIL:
			OutputDebugStringA("Application process intall fail\n\n");
			break;
		default:// fail
			break;
		}
	}

	void onConnectStatus(const CONNECTID_PTR connId,int status ,int err)
	{
		switch (status)
		{
		case kIPC_CS_CLIENT_CONNECTING:
			OutputDebugStringW(L"Connecting...,client\n\n");
			break;

		case kIPC_CS_CLIENT_CONNECTED:
			OutputDebugStringW(L"Connected to server.,client\n\n");
			break;

		case kIPC_CS_CLIENT_ERROR:
			OutputDebugStringW(L"Create connect failure,client\n\n");
			break;

		case kIPC_CS_PEER_CLOSE:
			OutputDebugStringW(L"connect close,peer\n\n");
			break;

		case kIPC_CS_CLOSE:
			OutputDebugStringW(L"connect close,own\n\n");
			break;

		case kIPC_CS_ERROR:
			OutputDebugStringW(L"connect error\n\n");
			break;

		case kIPC_CS_CLIENT_LOSTDATA:
			OutputDebugStringW(L"Client data lost\n\n");
			break;
		}
	}

	void onPluginStatus(unsigned int appid, int status, int err)
	{// for client
		switch(status)
		{
		case kIPC_PS_LOAD_OK: // for dll
			OutputDebugStringA("Respone:Load plugin ok\n\n");
			break;

		case kIPC_PS_LOAD_FAIL://for dll
			OutputDebugStringA("Respone:Load plugin fail\n\n");
			break;

		case kIPC_PS_UNLOAD_OK://for dll
			OutputDebugStringA("Respone:UnLoad plugin ok\n\n");
			break;

		case kIPC_PS_UNLOAD_FAIL://for dll
			OutputDebugStringA("Respone:UnLoad plugin fail\n\n");
			break;
		}
	}

	void onPluginRecv(unsigned int appid, const IPCEnginePackHead_t* head, const char* raw, size_t rawlen)
	{
		OutputDebugStringA("Plugin data recv\n\n");
	}
private:
	IIPCApplication* app_;
	IIPCApi* ipc_;
};

int main(int argc, char *argv[])
{
	IPCDll dll;
	if( kIPC_PS_LOAD_OK != dll.load(L"eduipc.dll"))
	{
		return -1;
	}
	IPCClientDispatch dispatch(dll.fnApplication(),dll.fnObject());

	IPCUsrConfig_t usr_t;
    IPCExe_t exe_t;
	IPCConnectConfig_t conncfg_t;
	IPCEnvironment_t env_t;
	if (!dll.fnApplication()->cmdLine(GetCommandLineW(),exe_t,conncfg_t,env_t,usr_t))
	{		
		dll.fnApplication()->uinit();		
		return -2;
	}
	dll.fnObject()->setUserConfig(usr_t);

	if (!dll.fnApplication()->initClient(exe_t.uuid.c_str()))
	{	
		dll.fnApplication()->uinit();		
		return -3;
	}

	//to do connect to server
	if (kIPC_RES_OK !=dll.fnObject()->connect(conncfg_t,env_t))
	{
		dll.fnApplication()->uinit();
		return -4;
	}

	{
		QWidget window;   
		window.resize(300, 200); 
		window.setWindowTitle("client");
		window.show();
	}

	dll.fnApplication()->exec();
	dll.fnApplication()->uinit();
	return 0;
}
