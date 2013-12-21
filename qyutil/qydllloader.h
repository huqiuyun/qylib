#ifndef __QY_DLLLOADER_H__
#define __QY_DLLLOADER_H__

#include "qyutil/qyconfig.h"
#include <assert.h>

DEFINE_NAMESPACE(qy)

class QYUTIL_API QyDLLLoader
{
public:
	QyDLLLoader()
		:handle_(NULL)
	{	
	}

	QyDLLLoader(const wchar_t* szDll)
		:handle_(NULL)
	{
		init(szDll);
	}

	~QyDLLLoader(void)
	{
		unInit();
	}

	int init(const wchar_t* szDll)
	{
		if (!handle_)
		{
#if defined(WIN32) || defined(WINCE)
			handle_ = LoadLibraryW(szDll);
#else

#endif
			assert(handle_);
		}
		return handle_?0:-1;
	}

	int unInit()
	{
		if (handle_)
		{
#if defined(WIN32) || defined(WINCE)
			::FreeLibrary(m_handle);
#else
#endif
			handle_ = NULL;
		}
		return 0;
	}

	QyHMODULE handle() const
	{
		return handle_;
	}
private:
	QyHMODULE handle_;
};

END_NAMESPACE(qy)

#endif //__QY_DLLLOADER_H__