#pragma once

#include "ipc-config.h"

template<typename T>
class IPCNullTest
{
public:
	IPCNullTest(void* r,void* p){}
	IPCNullTest(void* r){}
};

#if defined(_USE_IPC_TEST)

#if defined(_USE_TEST_PLUGIN)
#include "ipc-test-plugin.h"
#else
typedef IPCNullTest<int> IPCLoadPluginTest;
typedef IPCNullTest<int> IPCUnLoadPluginTest;
#endif//_USE_TEST_PLUGIN

#if defined(_USE_TEST_MSG)
#include "ipc-test-msg.h"
#else
typedef IPCNullTest<int> IPCMsgTest;
#endif //_USE_TEST_MSG 

#else //!_USE_TEST_MSG

typedef IPCNullTest<int> IPCLoadPluginTest;
typedef IPCNullTest<int> IPCUnLoadPluginTest;
typedef IPCNullTest<int> IPCMsgTest;

#endif// _USE_TEST_MSG
