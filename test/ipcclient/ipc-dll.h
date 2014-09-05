#pragma once

#include <windows.h>
#include "ipc-api.h"

class IPCDll
{
	typedef IIPCApi* (*_FnCreateIPCObject)();
	typedef void (*_FnDestroyIPCObject)(void*);

	typedef IIPCApplication* (*_FnCreateIPCApplication)(IIPCApi* obj);
	typedef void (*_FnDestroyIPCApplication)(IIPCApplication*);

	_FnCreateIPCObject fnCreateIPCObject_;
	_FnDestroyIPCObject  fnDestroyIPCObject_;

	_FnCreateIPCApplication fnCreateIPCApplication_;
	_FnDestroyIPCApplication fnDestroyIPCApplication_;

	HMODULE handle_;
	IIPCApi* api_;
	IIPCApplication* app_;

	int err_;
public:

	IPCDll()		
	{        
		fnCreateIPCObject_ = NULL;
		fnDestroyIPCObject_ = NULL;
		fnCreateIPCApplication_ = NULL;
		fnDestroyIPCApplication_ = NULL;
        handle_ = NULL;
		api_ = NULL;
		app_ = NULL;
		err_ = 0;
	}

	~IPCDll()
	{
		unLoad();
	}

	int load(const wchar_t* file)
	{
		if (!handle_)
		{
			handle_ = LoadLibraryW(file);
			if (!handle_)
			{
				err_ = ::GetLastError();
				return kIPC_PS_LOAD_FAIL;
			}
			fnCreateIPCObject_ = (_FnCreateIPCObject)::GetProcAddress(handle_, "createIPCObject");
			fnDestroyIPCObject_  = (_FnDestroyIPCObject)::GetProcAddress(handle_, "destroyIPCObject");

			fnCreateIPCApplication_ = (_FnCreateIPCApplication)::GetProcAddress(handle_, "createIPCApplication");
			fnDestroyIPCApplication_  = (_FnDestroyIPCApplication)::GetProcAddress(handle_, "destroyIPCApplication");
			if (!fnCreateIPCObject_ || !fnDestroyIPCObject_ || !fnCreateIPCApplication_ || !fnDestroyIPCApplication_ )
			{
				FreeLibrary(handle_);
			    handle_ = NULL;

				err_ = kIPC_RES_NOTFIND_ENTRY;
				return kIPC_PS_LOAD_FAIL;
			}
		}
		return kIPC_PS_LOAD_OK;
	}

	void unLoad()
	{
		if (!handle_)
		{
			return;
		}
		fnFree();
		FreeLibrary(handle_);
		handle_ = NULL;

		fnCreateIPCObject_ = NULL;
		fnDestroyIPCObject_ = NULL;

		fnCreateIPCApplication_ = NULL;
		fnDestroyIPCApplication_ = NULL;
	}

	HMODULE handle() const
	{
		return handle_;
	}

	IIPCApi* fnObject()
	{
        if (!fnCreateIPCObject_)
        {
            return NULL;
        }
		if (api_)
		{
			return api_;
		}
		api_ = fnCreateIPCObject_();
		return api_;
	}

	IIPCApplication* fnApplication()
	{
        if (!fnCreateIPCApplication_)
        {
            return NULL;
        }
		if (app_)
		{
			return app_;
		}

		if (NULL == fnObject())
		{
			return NULL;
		}
		app_ = fnCreateIPCApplication_(api_);
		return app_;
	}

	void fnFree()
	{
		if (app_)
		{
			fnDestroyIPCApplication_(app_);
			app_ = NULL;
		}

		if (api_)
		{
			fnDestroyIPCObject_(api_);
			api_ = NULL;
		}
	}
};

