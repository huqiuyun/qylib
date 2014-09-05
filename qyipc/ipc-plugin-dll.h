#pragma once

//-----------------------------------------------------------------------------------------//
// IPCPluginDll
#include "ipc-define.h"
#include "ipc-api.h"

#include "qyutil/qyunknwn.h"
#include "qyutil/qylog.h"

class IPCPluginDll : public qy::QyUnknown
{
	typedef void* (*_FnPluginEntry)(IIPCApi*, const wchar_t* cmdline,int* result);
	typedef void (*_FnPluginFree)(void*);
	typedef void (*_FnPluginInit)(void*);
	_FnPluginEntry fnEntry_;
	_FnPluginFree  fnFree_;
	_FnPluginInit  fnInit_;
	HMODULE handle_;
	void* object_;
    int  err_;
	bool entry_;
	bool init_;
	std::wstring filename_;
public:

	IPCPluginDll()
		:qy::QyUnknown(NULL)
	{
        object_ = NULL;
        handle_ = NULL;
		fnFree_ = NULL;
		fnEntry_= NULL;

		init_ = false;
		err_ = 0;
		entry_ = false;
	}

	~IPCPluginDll()
	{
		unLoad();
	}

	QY_DECLARE_IUNKNOWN()

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
			fnEntry_ = (_FnPluginEntry)::GetProcAddress(handle_, "OnPluginEntry");
			fnFree_  = (_FnPluginFree)::GetProcAddress(handle_, "OnPluginFree");
			fnInit_ = (_FnPluginInit)::GetProcAddress(handle_, "OnPluginInit");
			if (!fnEntry_ || !fnFree_)
			{
				err_ = kIPC_RES_NOTFIND_ENTRY;
				return kIPC_PS_LOAD_FAIL;
			}
			filename_ = file;
		}
		return kIPC_PS_LOAD_OK;
	}

	void unLoad()
	{
		QY_LOG(qy::kQyLogDev, qy::kQyLogInfo_LL, L"[IPC]Dll: [Begin] unLoad %s",filename_.c_str());
		if (handle_)
		{
			fnFree();
            FreeLibrary(handle_);
			handle_ = NULL;
			fnEntry_ = NULL;
			fnFree_ = NULL;
			entry_ = false;
			init_ = false;
		}
		QY_LOG(qy::kQyLogDev, qy::kQyLogInfo_LL, L"[IPC]Dll: [End] unLoad");
	}

	HMODULE handle() const
	{
		return handle_;
	}

	int error() const
	{
		return err_;
	}

    void* object() const
    {
        return object_;
    }

	bool fnEntry(IIPCApi* api, const wchar_t* cmdline,int* status)
	{
		int result = 0;
        if (!fnEntry_)
        {
            return false;
        }
        if (!entry_ || NULL == object_)
		{
            entry_ = false;
			if (NULL != (object_=fnEntry_(api,cmdline,&result)))
			{
				entry_ = true;
			}
            else
            {
				err_ = kIPC_RES_CALLENTRY_FAIL;
				*status = kIPC_PS_LOAD_FAIL;
            }
		}
		return entry_;
	}

    void fnFree()
    {
        if (fnFree_&&object_)
        {
            fnFree_(object_);
            object_ = NULL;
        }
    }

	void fnInit()
	{
		if (fnInit_ &&object_&& !init_)
		{
			fnInit_(object_);
			init_ = true;
		}
	}
};

