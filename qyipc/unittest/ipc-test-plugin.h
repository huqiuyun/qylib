#pragma once

#include "ipc-struct.h"
#include "ipc-tasks.h"
#include "ipc-taskrunner.h"
#include "ipc-object.h"

class IPCLoadPluginTest
{
public:
	IPCLoadPluginTest(IPCObject* ipc)
	{
		IPCPlugin_t plugin;
		plugin.connId = 1;
		plugin.appid = 10051;
		plugin.file = ipc->applicationDir();
		plugin.file += L"10051\\whiteboard.dll";
		ipc->pluginMgr()->start(plugin);
	}
};

class IPCUnLoadPluginTest
{
public:
	IPCUnLoadPluginTest(IPCObject* ipc)
	{
		ipc->pluginMgr()->stop(10051);
	}
};