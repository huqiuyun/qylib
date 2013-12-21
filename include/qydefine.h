/**
 * @author loach
 *
 * @date 2009-06-08
 *
 */
#ifndef  QY_DEFINE_H__
#define  QY_DEFINE_H__

#include <stddef.h>
#include "qysysconfig.h"
#include "qyctypes.h"

#ifndef qyRelease
#define qyRelease(p) if (p) { (p)->Release(); (p) = NULL; }
#endif //qyRelease

#ifndef qyAddRef
#define qyAddRef(p) if (p) (p)->AddRef();
#endif //qyAddRef

#ifndef qyDelete
#define qyDelete(h)  if(h) { delete (h); (h)= NULL;}
#define qyDeleteArray(h) if(h) { delete[] (h); (h)= NULL;}
#endif //qyDeleteM


#ifndef qySafeFree
#define qySafeFree(x) if (x){ ::free((void *)x); x = NULL; }
#endif //qySafeFree

#ifndef qyDelHandle

#ifdef H_OS_WIN
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
#define QY_D(Class) Class##Private* d = reinterpret_cast<Class##Private *>(d_ptr)
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

#ifdef H_OS_WIN

#include <windows.h>
#include <tchar.h>

#define QY_EXPORT(__type)     __declspec(dllexport) __type
#define QY_IMPORT(__type)     __declspec(dllimport) __type

// windows path separator char
#define QY_PATH_SEPARATOR  '\\'
#define QY_PATH_SEPARATOR_ERR '/'

#else

#define QY_PATH_SEPARATOR '/'
#define QY_PATH_SEPARATOR_ERR '\\'

// handle
#define DECLARE_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
typedef void               *HANDLE;
typedef HANDLE             *PHANDLE;

DECLARE_HANDLE(HMETAFILE);
DECLARE_HANDLE(HINSTANCE);
typedef HINSTANCE           HMODULE;      /* HMODULEs can be used in place of HINSTANCE */

typedef bool  BOOL;
#define TRUE  1
#define FALSE 0

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

#endif // H_OS_WIN

template <class T>
qyINLINE const T &
qyos_MIN (const T &t1, const T &t2) {
    return t2 > t1 ? t1 : t2;
}

template <class T>
qyINLINE const T &
qyos_MAX (const T &t1, const T &t2) {
    return t1 > t2 ? t1 : t2;
}

template <class T>
qyINLINE const T &
qyos_MIN (const T &t1, const T &t2, const T &t3) {
    return qyos_MIN(qyos_MIN (t1, t2), t3);
}

template <class T>
qyINLINE const T &
qyos_MAX (const T &t1, const T &t2, const T &t3) {
    return qyos_MAX(qyos_MAX (t1, t2), t3);
}

template <class T>
qyINLINE const T &
qyos_RANGE (const T &min, const T &max, const T &val) {
    return qyos_MIN (qyos_MAX (min, val), max);
}

qyINLINE int qyos_hiword(unsigned long d) {
    return (int)((d >> 16) & 0xFFFF);
}

qyINLINE int qyos_loword(unsigned long d) {
    return (int)(d & 0xFFFF);
}

// for lock , event
const int kEventOk           = 0x00000000L;
const int kEventAbandDoned   = 0x00000080L;
const int kEventTimeout      = 0x00000102L;
const int kEventError        = 0xffffffffL;
const unsigned long kForever = 0xFFFFFFFF;

#endif /* QY_DEFINE_H__*/
