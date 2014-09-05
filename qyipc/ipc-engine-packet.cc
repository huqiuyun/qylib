#include "ipc-engine-packet.h"

IPCEnginePacketQueue::IPCEnginePacketQueue()
{
}

IPCEnginePacketQueue::~IPCEnginePacketQueue()
{
    clear();
}

int IPCEnginePacketQueue::push(const IPCEnginePackHead_t& head, const unsigned char* data, size_t len)
{
	if (len==0 || !data) 
	{
		return -1;
	}

	if (len > kPACKET_BUFFER_MAX)
	{
		return -2;
	}

	qy::QyAutoLocker cs(&crit_);
	IPCEnginePack_t* p = new IPCEnginePack_t;
	p->head = head;
	p->raw.append((const char*)data, len);
	packs_.push(p);
	return 0;
}

const IPCEnginePack_PTR IPCEnginePacketQueue::next()
{
    qy::QyAutoLocker cs(&crit_);
	if (packs_.empty())
	{
		return NULL;
	}
	IPCEnginePack_t* p = packs_.front();
	packs_.pop();
    return p;
}

void IPCEnginePacketQueue::clear()
{
	qy::QyAutoLocker cs(&crit_);
	while (!packs_.empty()) 
	{
		IPCEnginePack_t* p = packs_.front();
		packs_.pop();
		delete p;
	}
}