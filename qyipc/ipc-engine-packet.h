#pragma once

#include <queue>
#include "qyutil/qylock.h"
#include "ipc-engine-protocol.h"

class IPCEnginePacketQueue
{
public:
    IPCEnginePacketQueue();
    ~IPCEnginePacketQueue();

	/** @return -2 size too long, 0,success ,-1 faiure */
	int push(const IPCEnginePackHead_t& head, const unsigned char* data, size_t len);
    const IPCEnginePack_PTR next();

private:
    void clear();

private:
    typedef std::queue<IPCEnginePack_t*> PACKQueue;
    PACKQueue  packs_;
	qy::QySLCS crit_;
};
