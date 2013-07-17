#include "qydirectory.h"
#include "qystring.h"

#if defined(WIN32) || defined(_WIN32) || defined(WINCE)
#include <windows.h>
#include <shlobj.h>
#include <tchar.h>
#else
#include <wchar.h>
#endif //WIN32

#include <string>

DEFINE_NAMESPACE(qy)

static  std::wstring __systemAppDataPath;
static  std::wstring __systemDocumentPath;
static  std::wstring __systemTempPath;

int QyDirectory::getDirectoryA(HMODULE handle ,char* pszPath , int len)
{		
	int outsize = 0;
#if defined(WIN32) || defined(_WIN32) || defined(WINCE)
	outsize = ::GetModuleFileNameA(handle, pszPath, len);		
#else
#endif //win32
	pathA(pszPath,outsize);
	char* sz = strrchr( pszPath , QY_PATH_SEPARATOR);
	if (sz)
	{
		*(sz+1) = '\0';
	}
	return (outsize<len)?(outsize+1):len;
}

int QyDirectory::getDirectoryW(HMODULE handle ,wchar_t* pszPath , int len)
{	
	int outsize = 0;
#if defined(WIN32) || defined(_WIN32) || defined(WINCE)
	outsize = ::GetModuleFileNameW(handle, pszPath, len);	
#else
#endif//win32
	pathW(pszPath,outsize);
	wchar_t* sz = wcsrchr( pszPath , QY_PATH_SEPARATOR) ;
	if (sz)
	{
		*(sz+1) = L'\0';
	}
	return (outsize<len)?(outsize+1):len;
}

std::string QyDirectory::getDirectoryA(HMODULE handle)
{
	char szpath[MAX_PATH+1] = {0};
	getDirectoryA(handle , szpath , MAX_PATH);
	return szpath;
}

std::wstring QyDirectory::getDirectoryW(HMODULE handle)
{
	wchar_t szpath[MAX_PATH+1] = {0};
	getDirectoryW(handle , szpath , MAX_PATH);
	return szpath;
}

const wchar_t* QyDirectory::getSystemAppDataPath(void)
{
	if (__systemAppDataPath.length() <= 0)
	{
#if defined(WIN32) || defined(_WIN32) || defined(WINCE)
		wchar_t szpath[MAX_PATH] = {0};
		::SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, szpath);
		__systemAppDataPath = szpath;
#else
#endif
		QyDirectory dir;
		dir.pathW(__systemAppDataPath);
		// 加上后辍
		if (__systemAppDataPath[ __systemAppDataPath.length()-1 ] != QY_PATH_SEPARATOR)
		{
			__systemAppDataPath += QY_PATH_SEPARATOR;
		}
	}
	return __systemAppDataPath.c_str();
}

const wchar_t* QyDirectory::getSystemDocumentPath(void)
{
	if (__systemDocumentPath.length() <= 0)
	{
#if defined(WIN32) || defined(_WIN32) || defined(WINCE)
		wchar_t szpath[MAX_PATH]={0};	
		::SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, szpath);
		__systemDocumentPath = szpath;
#else
#endif 
		QyDirectory dir;
		dir.pathW(__systemDocumentPath);

		// 加上后辍
		if (__systemDocumentPath[ __systemDocumentPath.length()-1 ] != QY_PATH_SEPARATOR)
		{
			__systemDocumentPath += QY_PATH_SEPARATOR;
		}
	}
	return __systemDocumentPath.c_str();
}

void QyDirectory::removeFolderByRecursive(const std::wstring &folder)
{
#if defined(WIN32) || defined(_WIN32) || defined(WINCE)
	WIN32_FIND_DATAW data = {0};
	std::wstring str = folder + L"/*";
	HANDLE h = ::FindFirstFileW( str.c_str(), &data);
	if (h == INVALID_HANDLE_VALUE)
	{
		return ;
	}
	do
	{
		if (std::wstring(L".") != data.cFileName &&
			std::wstring(L"..") != data.cFileName)
		{
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				removeFolderByRecursive(folder + L"/" + data.cFileName);
			}
			else
			{
				std::wstring file = folder + L"/" + data.cFileName;
				if (data.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
				{
					DWORD dwAttr = ~FILE_ATTRIBUTE_READONLY;
					::SetFileAttributesW(file.c_str(), data.dwFileAttributes & dwAttr);
				}
				::DeleteFileW(file.c_str());
			}
		}

		BOOL b = ::FindNextFileW(h, &data);
		if (!b)
		{
			break;
		}
	} while (true);

	::FindClose(h);
	::RemoveDirectoryW(folder.c_str());
#else
#endif
}

bool QyDirectory::deleteFolder(const std::wstring& folder)
{
#if defined(WIN32) || defined(_WIN32) || defined(WINCE)
	wchar_t buffer[2048] = {0};
	SHFILEOPSTRUCTW sc;

	ZeroMemory(&sc, sizeof(SHFILEOPSTRUCTW));
	sc.wFunc = FO_DELETE;	
	sc.pFrom = buffer;
	sc.fFlags = FOF_NOERRORUI | FOF_SILENT | FOF_NOCONFIRMATION;

	wcsncpy( buffer ,  folder.c_str() ,folder.length());
	
	if (0 != SHFileOperationW(&sc))	
	{	////An unknown error occurred. This is typically due to an invalid path in the source or destination. This error does not occur on Windows Vista and later.
		removeFolderByRecursive(folder);
	}
	return true;
#else
    return false;
#endif
}

bool QyDirectory::createDirectory(const wchar_t* path)
{
	bool ok = 0;
	if (!path)
	{
		return false;
	}
	{		
	    wchar_t _path[ 513 ] = {0};
		wchar_t* zp = 0;

		wcsncpy(_path , path , 512);
		zp = _path;
		while( *zp != '\0')
		{
			wchar_t* zf = wcschr(zp,QY_PATH_SEPARATOR);
			if (!zf)
			{
				break;
			}
			*zf = '\0';
            
#if defined(WIN32) || defined(_WIN32) || defined(WINCE)
			if (CreateDirectoryW( path , NULL))
				ok |= 1;
#else
            
#endif
			*zf = QY_PATH_SEPARATOR;
			zp = zf + 1;
		}
	}
	return ok;
}

bool QyDirectory::createDirectoryA(const char *path)
{
	bool ok = 0;
	if (!path)
	{
		return false;
	}
	{
	    char _path[ 513 ] = {0};
		char* zp = 0;
        
		strncpy(_path , path , 512);
		zp = _path;
		while (*zp != '\0')
		{
			char* zf = strchr(zp,QY_PATH_SEPARATOR);
			if (!zf)
			{
				break;
			}
			*zf = '\0';
            
#if defined(WIN32) || defined(_WIN32) || defined(WINCE)
			if (CreateDirectoryA(path , NULL))
				ok |= 1;
#else
            
#endif
			*zf = QY_PATH_SEPARATOR;
			zp = zf + 1;
		}
	}
	return ok;
}

bool QyDirectory::createFolder(const wchar_t* folder)
{
	if (!folder || *folder == '\0')
	{
		return false;
	}
    
#if defined(WIN32) || defined(_WIN32) || defined(WINCE)
	unsigned long dwAttrib = GetFileAttributesW(szFolder);

	// already exists ?
	if (dwAttrib != 0xffffffff)
	{
		return ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
	}
	// if can't create parent
	if (::CreateDirectoryW(folder,NULL))
	{
		dwAttrib = 0;			
	}
	return (0==dwAttrib);
#else
    return false;
#endif 
}

bool QyDirectory::isAbsoluteFilePathA(const char* path)
{
#if defined(WIN32) || defined(_WIN32)
	bool in =((*path >= 'A' && *path <= 'Z') || (*path >= 'a' && *path <= 'z'));
	if (!in)
	{
		return false;
	}
	const char* fi = strchr(path , ':');
	if (!fi)
	{
		return false;
	}
	if (*(fi+1) != '/' || *(fi+1) != '\\')
	{
		return false;
	}
	return true;
#else
	return false;
#endif
}

// 绝对路径，简单检测,是否含有 :
bool QyDirectory::isAbsoluteFilePathW(const wchar_t* path)
{
#if defined(WIN32) || defined(_WIN32)
	bool in =((*path >= L'A' && *path <= L'Z') || (*path >= L'a' && *path <= L'z'));
	if (!in)
	{
		return false;
	}
	const wchar_t* fi = wcschr(path , L':');
	if (!fi)
	{
		return false;
	}
	if (*(fi+1) != L'/' || *(fi+1) != L'\\')
	{
		return false;
	}
	return true;
#else
	return false;
#endif
}

bool QyDirectory::getFilePathA(/*out*/char* szPath,size_t iPathLen/*MAX_PATH+1*/ , const char* szFile)
{
    QyStringA filename(szFile);

	filename.path();
	QyStringA::size_type idx = filename.rfind(QY_PATH_SEPARATOR);
	if (idx != QyStringA::npos)
	{
		filename.copyTo(szPath , iPathLen , 0 ,idx+1);
		return true;
	}
	return false;
}

bool QyDirectory::getFilePathW(/*out*/wchar_t* szPath,size_t iPathLen/*MAX_PATH+1*/ , const wchar_t* szFile)
{
	QyStringW filename(szFile);

	filename.path();
	QyStringW::size_type idx = filename.rfind(QY_PATH_SEPARATOR);
	if (idx != QyStringW::npos)
	{
		filename.copyTo(szPath , iPathLen , 0 ,idx+1);
		return true;
	}
	return false;
}

void QyDirectory::pathW(wchar_t* path , size_t len)
{
	qyutil_safePath(path,len);
}

void QyDirectory::pathW(std::wstring& path)
{
	qyutil_safePath(path,path.size());
}

void QyDirectory::pathA(char* path , size_t len)
{
	qyutil_safePath(path,len);
}

void QyDirectory::pathA(std::string& path)
{
	qyutil_safePath(path,path.size());
}

END_NAMESPACE(qy)