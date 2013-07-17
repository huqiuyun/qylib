#include "qyversion.h"
#include <assert.h>
#include <string>

#if defined(WIN32) || defined(_WIN32)
#include <wininet.h>
#pragma comment(lib, "version.lib")
#pragma comment(lib, "wininet.lib")
#endif
#include "qyosstring.h"
#include "qyos.h"

DEFINE_NAMESPACE(qy)

class QyVersionPrivate
{
public:
	std::wstring legalCopyright_;
	std::wstring privateBuild_;
	std::wstring specialBuild_;

	std::wstring majorVer_;
	std::wstring lowVer_;
	std::wstring ver3_;
	std::wstring ver_;

	unsigned long maxver_, lowver_;

	BOOL read(const wchar_t* sFileName)
	{
    #if defined(WIN32) || defined(_WIN32)
		DWORD dwHandle;     
		DWORD dwDataSize = ::GetFileVersionInfoSizeW(sFileName, &dwHandle); 
		if(dwDataSize == 0) 
		{
			return FALSE;
		}
		wchar_t *lpVersionData = new wchar_t[dwDataSize]; 
		if (!::GetFileVersionInfoW(sFileName, dwHandle, dwDataSize, lpVersionData))
		{
			delete [] lpVersionData;
			return FALSE;
		}

		assert(lpVersionData != NULL);

		UINT nQuerySize=0;
		VS_FIXEDFILEINFO* pVsffi=0;
		if(::VerQueryValueW((void *)lpVersionData, L"\\", (void**)&pVsffi, &nQuerySize))
		{
			maxver_ = pVsffi->dwFileVersionMS;
			lowver_ = pVsffi->dwFileVersionLS;
			//
			struct LANGANDCODEPAGE
			{
				WORD wLanguage;
				WORD wCodePage;
			} *lpTranslate;

			UINT cbTranslate=0;
			VerQueryValueW(lpVersionData, L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &cbTranslate);

			// Read the file description for each language and code page.
			std::wstring codePage;
			wchar_t buffer[512]= {0};
			swprintf_s( buffer ,512, L"%04x%04x",lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
			codePage = buffer;

			LPVOID pVoid=0;		
			swprintf_s( buffer , 512, L"\\StringFileInfo\\%s\\PrivateBuild", codePage.c_str());
			if(::VerQueryValueW((void *)lpVersionData, (LPWSTR)buffer, &pVoid, &nQuerySize))
			{
				privateBuild_ = (const wchar_t*)pVoid;
			}
			swprintf_s( buffer , 512, L"\\StringFileInfo\\%s\\SpecialBuild", codePage.c_str());
			if(::VerQueryValueW((void *)lpVersionData, (LPWSTR)buffer, &pVoid, &nQuerySize))
			{
				privateBuild_ = (const wchar_t*)pVoid;
			}

			swprintf_s( buffer , 512, L"\\StringFileInfo\\%s\\LegalCopyright", codePage.c_str());
			if(::VerQueryValueW((void *)lpVersionData, (LPWSTR)buffer, &pVoid, &nQuerySize))
			{
				legalCopyright_ = (const wchar_t*)(pVoid);
			}
			delete [] lpVersionData;
			return TRUE;
		}
		delete [] lpVersionData;
#else
#endif 
		return FALSE;
	}	
};

QyVersion::QyVersion()
{
	d_ptr_ = new QyVersionPrivate();
}

QyVersion::~QyVersion()
{
	delete d_ptr_;
}

BOOL QyVersion::read(HINSTANCE hResource)
{
	wchar_t szModuleFileName[1024]={0};
#if defined(WIN32) || defined(_WIN32)
	if(::GetModuleFileNameW(hResource, szModuleFileName, sizeof(szModuleFileName)) == 0)
	{
		return FALSE;
	}
#endif
	return d_ptr_->read(szModuleFileName);
}

BOOL QyVersion::read(const wchar_t* sFile)
{
	if (!sFile || *sFile == '\0')
	{
		wchar_t szModuleFileName[1024]={0};
#if defined(WIN32) || defined(_WIN32)
		if (GetModuleFileName( NULL , szModuleFileName, sizeof(szModuleFileName)) == 0)
		{
			return FALSE;
		}
#else
        
#endif
		return d_ptr_->read(szModuleFileName);
	}
	else
	{
		return d_ptr_->read( sFile);
	}
}

BOOL QyVersion::string2Word(const wchar_t* Ver , DWORD& dwMS,DWORD& dwLS)
{
	if (!Ver)
	{
		return false;
	}

	wchar_t verTemp[64] = {0};
	wcsncpy(verTemp , Ver , 64);

	// 1.1.1.0
	int i = 0;
	wchar_t* szPosition[5] = {0};		
	szPosition[i++] = verTemp;
	wchar_t* sz = verTemp;
	while( i < 5)
	{
		if (*sz == L'\0')
		{
			szPosition[i] = sz;
			break;
		}
		else if (*sz == L'.')
		{
			*sz = L'\0';
			sz++;
			szPosition[i++] = sz;
		}
		else sz++;
	}

	DWORD dwMS_hi = 0,dwMS_lo = 0;
	if (szPosition[1])
	{
		dwMS_hi = (unsigned long)qyos_atoi(szPosition[0]);
	}
	if (szPosition[2])
	{
		dwMS_lo = (unsigned long)qyos_atoi(szPosition[1]);
	}
	dwMS =(dwMS_hi << 16) + dwMS_lo;

	DWORD dwLS_hi = 0,dwLS_lo = 0;
	if (szPosition[3])
	{
		dwLS_hi = (unsigned long)qyos_atoi(szPosition[2]);
	}
	if (szPosition[4])
	{
		dwLS_lo = (unsigned long)qyos_atoi(szPosition[3]);
	}
	dwLS =(dwLS_hi << 16) + dwLS_lo;

	return true;
}

const wchar_t* QyVersion::getVersionString(int nVer)
{
	const wchar_t* str = L"";

	int ver1 = qyos_hiword(d_ptr_->maxver_);
	int ver2 = qyos_loword(d_ptr_->maxver_);
	int ver3 = qyos_hiword(d_ptr_->lowver_);
	int ver4 = qyos_loword(d_ptr_->lowver_);
			
	if (nVer == 1)
	{
		if (d_ptr_->majorVer_.length() <= 0)
		{
			wchar_t buffer[64]= {0};
			swprintf( buffer ,64, L"%d.%d", ver1, ver2);
			d_ptr_->majorVer_ = buffer;
		}
		str = d_ptr_->majorVer_.c_str();
	}
	else if (nVer == 2 )
	{
		if (d_ptr_->lowVer_.length() <= 0)
		{
			wchar_t buffer[64]= {0};
			swprintf( buffer ,64, L"%d.%d", ver3, ver4);	
			d_ptr_->lowVer_ = buffer;
		}
		str = d_ptr_->lowVer_.c_str();
	}
	else if (3 == nVer)
	{
		if (d_ptr_->ver3_.length() <= 0)
		{
			wchar_t buffer[64]= {0};
			swprintf( buffer ,64, L"%d.%d.%d", ver1, ver2, ver3);
			d_ptr_->ver3_ = buffer;
		}
		str = d_ptr_->ver3_.c_str();
	}

	else if (nVer == -1)
	{
		if (d_ptr_->ver_.length() <= 0)
		{
		wchar_t buffer[64]= {0};
			swprintf( buffer ,64, L"%d.%d.%d.%d", ver1, ver2, ver3, ver4);
			d_ptr_->ver_ = buffer;
		}
		str = d_ptr_->ver_.c_str();
	}
	return str;
}

const wchar_t* QyVersion::getLegalCopyright(void) const
{
	return d_ptr_->legalCopyright_.c_str();
}

const wchar_t* QyVersion::getPrivateBuild(void) const
{
	return d_ptr_->privateBuild_.c_str();
}

const wchar_t* QyVersion::getSpecialBuild(void) const
{
	return d_ptr_->specialBuild_.c_str();
}

unsigned long QyVersion::getMS(void) const
{
	return d_ptr_->maxver_;
}

unsigned long QyVersion::getLS(void) const
{
	return d_ptr_->lowver_;
}

QyVersion::Win32SysType  QyVersion::getShellType()  
{  
	static QyVersion::Win32SysType  ShellType = WindowsNo;  
	if (ShellType != WindowsNo)
	{
		return ShellType;
	}
#if defined(WIN32) || defined(_WIN32)
	BOOL bOsVersionInfoEx = FALSE;
	OSVERSIONINFOEXW osvi;  
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEXW));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

	DWORD winVer = ::GetVersion();  
	if (winVer < 0x80000000)
	{
		// NT
		ShellType = WindowsNT3;  
		if (!(bOsVersionInfoEx = GetVersionExW ((OSVERSIONINFOW *) &osvi)))
		{
			osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFOW);
			if (!GetVersionExW((OSVERSIONINFOW *) &osvi)) 
			{
				return ShellType;
			}
		}
		if (osvi.dwMajorVersion==4L) ShellType = WindowsNT4;  
		else if (osvi.dwMajorVersion == 5L && osvi.dwMinorVersion==0L) ShellType = Windows2000;  
		else if (osvi.dwMajorVersion == 5L && osvi.dwMinorVersion==1L) ShellType = WindowsXP;
		else if (osvi.dwMajorVersion == 6L && osvi.dwMinorVersion==0L) ShellType = WindowsVista;
	}  
	else if (LOBYTE(LOWORD(winVer)) < 4)  
	{
		ShellType = Windows32s;  
	}
	else
	{  
		ShellType = Windows95;  
		if (!(bOsVersionInfoEx = GetVersionExW ((OSVERSIONINFOW *) &osvi)))
		{
			osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFOW);
			if (!GetVersionExW((OSVERSIONINFOW *) &osvi))
			{
				return ShellType;
			}
		} 
		if (osvi.dwMajorVersion == 4L && osvi.dwMinorVersion == 10L) ShellType = Windows98;  
		else if (osvi.dwMajorVersion == 4L && osvi.dwMinorVersion == 90L) ShellType = WindowsME;
	}
#endif
	return   ShellType;
}

END_NAMESPACE(qy)