#pragma once

#include "ipc-pipe-base.h"

class IPCPipeClient : public IPCPipeBase
{
public:
	IPCPipeClient(const IPCConnectConfig_t& cfg,IPCEngine* engine);
	~IPCPipeClient();

public:
	int  create();
	void destroy();
};
