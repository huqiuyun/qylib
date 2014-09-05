
#include "ipc-taskrunner.h"

IPCTaskRunner::IPCTaskRunner(qy::QyMessageThread* thread)
:thread_(thread)
{
}

void IPCTaskRunner::wakeTasks(int cmsDelay)
{
	if (thread_)
	{
		if (cmsDelay > 0)
		{
			thread_->postDelayed(cmsDelay,this);
		}
		else
		{
			thread_->post(this);
		}
	}
}

void IPCTaskRunner::onMessage(qy::QyMessage*)
{
	runTasks();
}
