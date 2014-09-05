#pragma once

#include "ipc-struct.h"
#include "qyutil/qymessagethread.h"
#include "ipc-object.h"

class IPCMsgHandlerTest : public qy::QyMessageHandler
{
	IPCMsgHandlerTest(IPCObject* ipcobj)
		:ipcobj_(ipcobj)
	{
	}
public:
	static IPCMsgHandlerTest* inst(IPCObject* ipcobj) {
		static IPCMsgHandlerTest in(ipcobj);
		return &in;
	}
private:
	void onMessage(qy::QyMessage *msg)
	{
		char buffer[16]={0};
		static int i = 1;
		sprintf(buffer,"-%d",i++);

		std::string data("test mssage");
		data += buffer;
		ipcobj_->send(1, (const unsigned char*)data.c_str(),data.length());
	}
private:
	IPCObject* ipcobj_;
};

class IPCMsgTest 
{
public:
	IPCMsgTest(IPCObject* ipcobj)
	{
		for(int i= 1; i <= 10000; i++) {
			ipcobj->msg()->postDelayed(i*5,IPCMsgHandlerTest::inst(ipcobj));
		}
	}
};