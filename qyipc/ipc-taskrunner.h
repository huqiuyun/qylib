#pragma once

#include "qyutil/qymessagethread.h"
#include "qyutil/qytaskrunner.h"

class IPCTaskRunner 
	: public qy::QyTaskRunner
	, public qy::QyMessageHandler
{
public:
    IPCTaskRunner(qy::QyMessageThread* thread);

	void wakeTasks(int cmsDelay);
private:
	void onMessage(qy::QyMessage *msg);
private:
	qy::QyMessageThread* thread_;
};

