#pragma once

#include "ipc-engine-protocol.h"

enum eEngineHandlerLevel
{
    kEHL_NONE = 0,
    kEHL_LOW,
    kEHL_HIGH,
	kEHL_APP, ///< application(exe)
	kEHL_TOPMOST,
    kEHL_END,
};

class IPCEngineHandler
{
public:
    virtual ~IPCEngineHandler(){}

    virtual bool onEngineHandler(const unsigned char* data,size_t len) = 0;
	virtual void onEngineStatus(int status,int err) =0;
};

