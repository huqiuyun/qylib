#ifndef __QY_DLLLOADER_H__
#define __QY_DLLLOADER_H__

#include "qyutil/qyconfig.h"
#include <assert.h>

DEFINE_NAMESPACE(qy)

class QYUTIL_API QyDLLLoader
{
private:

public:
	QyDLLLoader()
		:m_handle(NULL)
	{	
	}

	QyDLLLoader(const wchar_t* szDll)
		:m_handle(NULL)
	{
		init(szDll);
	}

	~QyDLLLoader(void)
	{
		unInit();
	}

	int init(const wchar_t* szDll)
	{
		SET_PATH(szDll);
		if (!m_handle)
		{
#if defined(WIN32) || defined(WINCE)
			m_handle = LoadLibraryW(szDll);
#else

#endif
			assert(m_handle);
		}
		return m_handle?0:-1;
	}

	int unInit()
	{
		if (m_handle)
		{
#if defined(WIN32) || defined(WINCE)
			::FreeLibrary(m_handle);
#else
#endif
			m_handle = NULL;
		}
		return 0;
	}

	QyHMODULE handle() const
	{
		return m_handle;
	}
private:
	QyHMODULE m_handle;
};

END_NAMESPACE(qy)

#endif //__qy_DLLLOADER_H__