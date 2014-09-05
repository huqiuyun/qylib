#include "ipc-object.h"

#ifdef __cplusplus
extern "C"
{
#endif

	__declspec(dllexport) 
		IIPCApi* createIPCObject()
	{
		return new IPCObject();
	}

	__declspec(dllexport) 
		void destroyIPCObject(IIPCApi* api)
	{
		delete reinterpret_cast<IPCObject*>(api);
	}

#ifdef __cplusplus
}
#endif
