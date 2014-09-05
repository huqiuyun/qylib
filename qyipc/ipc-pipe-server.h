#pragma once

#include "ipc-pipe-base.h"

class IPCPipeServer : public IPCPipeBase
{
public:
	IPCPipeServer(const IPCConnectConfig_t& cfg,IPCEngine* engine);
	~IPCPipeServer();

public:
	int  create();
	void destroy();

private:
	int inputBufferCount() const;
	int outputBufferCount() const;

private:
	int ouput_buffercounts_;
	int input_buffercounts_;
};
