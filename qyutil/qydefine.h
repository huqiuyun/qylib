#ifndef  __QY_DEFINE_H__
#define  __QY_DEFINE_H__

#include <stddef.h>
#include <qyutil/qyctypes.h>
/**
 * @author loach
 *
 * @date 2009-06-08
 * 
*/

#ifndef qyRelease
#define qyRelease(p) if (p) { (p)->Release(); (p) = NULL; }
#endif //qyRelease

#ifndef qyAddRef
#define qyAddRef(p) if (p) (p)->AddRef();
#endif //qyAddRef

#ifndef qyDeleteM
#define qyDeleteM(h) if(h) { delete (h); (h)= NULL;}
#define qyDeleteA(h) if(h) { delete[] (h); (h)= NULL;}
#endif //qyDeleteM


#ifndef qySafeFree
#define qySafeFree(x) if (x){ ::free((void *)x); x = NULL; }
#endif //qySafeFree

#ifndef qyDelHandle

#ifdef WIN32
#define qyDelHandle(h) 	if(h) { ::DeleteObject(h); h = NULL ; }
#define qyCloseHandle(h) if (h) { ::CloseHandle(h); h = 0 ;}
#else
#define qyDelHandle(h) 	
#define qyCloseHandle(h)
#endif //WIN32

#endif //qyDelHandle

#ifndef __qyINLINE__
#define __qyINLINE__
#endif //__qyINLINE__

#ifndef qyINLINE
#define qyINLINE inline
#endif //qyINLINE

#ifndef DEFINE_NAMESPACE
#define DEFINE_NAMESPACE(N) namespace N {
#define END_NAMESPACE(N) }
#define USE_NAMESPACE(N) using namespace N;
#endif //DEFINE_NAMESPACE

#if (_MSC_VER >= 1100) && defined(__cplusplus)
#ifndef QYNovtableM
#define QYNovtableM __declspec(novtable)
#endif
#else
#define QYNovtableM
#endif

#ifndef QY_D
#define QY_D(Class) Class##Private* d = reinterpret_cast<Class##Private *>(d_ptr_)
#endif //QY_D


#ifndef QY_UNUSED
#define QY_UNUSED(p) ((void)p)
#endif //QY_UNUSED

struct t_qyPOSITION {};
typedef struct t_qyPOSITION* POSITION_l;

#define REINTERPRET_CAST(cls) reinterpret_cast<cls>
#define LPVOID_CAST(p)((void*)p)

#if defined(_DEBUG) && defined(USE_QY_DUMP)
#define DEBUG_DUMP() void Dump()
#define CALL_DEBUG_DUMP() Dump()
#else
#define DEBUG_DUMP()
#define CALL_DEBUG_DUMP()
#endif


#ifdef __cplusplus

#ifndef BEGIN_EXTERN_C

#define BEGIN_EXTERN_C extern "C" {
#define END_EXTERN_C }

#endif /* BEGIN_EXTERN_C */

#else

#define BEGIN_EXTERN_C 
#define END_EXTERN_C

#endif /* !__cplusplus */

#ifndef EXTERN
#define EXTERN extern
#endif

#if defined(WIN32)|| defined(WINCE)

#include <windows.h>
#include <tchar.h>

#define qyEXPORT(__type)     __declspec(dllexport) __type
#define qyIMPORT(__type)     __declspec(dllimport) __type

// windows path separator char
#define QY_PATH_SEPARATOR  '\\'
#define QY_PATH_SEPARATOR_ERR '/'

#define STDCALL __stdcall

#else

#define QY_PATH_SEPARATOR '/'
#define QY_PATH_SEPARATOR_ERR '\\'

// in UNIX platform define

#define far
#define near

#define VOID                void

#undef  FAR
#undef  NEAR
#define FAR                 far
#define NEAR                near


#ifndef CONST
#define CONST               const
#endif


#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#ifndef MAX_PATH
#define MAX_PATH            255
#endif

#ifndef _T

	#ifdef UNICODE
          #define _T(x)     L##x
	#else
	      #define _T(x)     x
	#endif 

#endif

typedef unsigned long		 DWORD;
typedef int                	 BOOL;
typedef unsigned char		 BYTE;
typedef unsigned short	     WORD;
typedef float			     FLOAT;
typedef FLOAT               *PFLOAT;
typedef BOOL near           *PBOOL;
typedef BOOL far            *LPBOOL;
typedef BYTE near           *PBYTE;
typedef BYTE far            *LPBYTE;
typedef int near            *PINT;
typedef int far             *LPINT;
typedef WORD near           *PWORD;
typedef WORD far            *LPWORD;
typedef long far            *LPLONG;
typedef DWORD near          *PDWORD;
typedef DWORD far           *LPDWORD;
typedef void far            *LPVOID;
typedef CONST void far      *LPCVOID;

typedef int                  INT;
typedef unsigned int         UINT;
typedef unsigned int        *PUINT;

typedef long			     __int32;
typedef long long		     __int64;


typedef char                 CHAR;
typedef short                SHORT;
typedef long                 LONG;
// tchar 
typedef CHAR                *PCHAR, *LPSTR;
typedef CONST  CHAR         *LPCSTR;

typedef wchar_t             WCHAR;
typedef WCHAR               *NWPSTR,  *LPWSTR, *PWSTR;
typedef CONST  WCHAR        *LPCWSTR, *PCWSTR;

//
typedef long long           LONGLONG;
typedef unsigned long long  ULONGLONG;

typedef long                HRESULT;
typedef unsigned long       ULONG;

// handle
#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
typedef void               *HANDLE;
typedef HANDLE             *PHANDLE;

DECLARE_HANDLE(HMETAFILE);
DECLARE_HANDLE(HINSTANCE);
typedef HINSTANCE           HMODULE;      /* HMODULEs can be used in place of HINSTANCE */


//--------------------------------------------------------------------------------------//
// error code 
#define STDMETHODCALLTYPE
#define STDCALL

#define _HRESULT_TYPEDEF_(sc) sc

#define S_OK                               ((HRESULT)0x00000000L)
#define S_FALSE                            ((HRESULT)0x00000001L)

//
// MessageId: E_POINTER
//
// MessageText:
//
//  Invalid pointer
//
#define E_POINTER                        _HRESULT_TYPEDEF_(0x80004003L)

//
// MessageId: E_NOINTERFACE
//
// MessageText:
//
//  No such interface supported
//
#define E_NOINTERFACE                    _HRESULT_TYPEDEF_(0x80004002L)

#define E_FAIL                           _HRESULT_TYPEDEF_(-1L)
#define E_OUTOFMEMORY                    _HRESULT_TYPEDEF_(-2L)

#define FAILED(r)                        ((r) != 0)
#define SUCESSED(r)                      ((r) == 0)
//--------------------------------------------------------------------------------------//

#endif // (defined(WIN32))||(defined(_WIN32))


#define qyutil_safePath(path , len)   {\
for (size_t j = 0; j < len; j++) {     \
if (path[j] == QY_PATH_SEPARATOR_ERR) {\
path[j] = QY_PATH_SEPARATOR;           \
} else if (path[j] == '\0')          {\
break;                                 \
}}}

// for lock , event
const int kEventOk         = 0x00000000L;
const int kEventAbandDoned = 0x00000080L;
const int kEventTimeout    = 0x00000102L;
const int kEventError      = 0xffffffffL;

/** */
enum QyCaseSensitivity {
    kCaseSensitive = 0,
    kCaseNotSensitive = 1 // Not case sensitive
};


#endif /* __QY_DEFINE_H__*/
