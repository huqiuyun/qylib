#include "ipc-api.h"
#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif

	class TestObject
	{
	public:

	};

	__declspec(dllexport) 
		void* OnPluginEntry(IIPCApi* api, const wchar_t *params, int* result)
	{
		return (void*)new TestObject();
	}

    __declspec(dllexport) 
		void OnPluginFree(void* obj)
    {
		delete (TestObject*)obj;
	}

    __declspec(dllexport) 
		void OnPluginInit(void* obj)
    {
		(TestObject*)obj;
	}

#ifdef __cplusplus
}
#endif