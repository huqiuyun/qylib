#ifndef __QY_UTIL_H__
#define __QY_UTIL_H__

#ifdef QYUTIL_DLL

#if defined(WIN32) || defined(_WIN32) || defined(WINCE)

#ifdef QYUTIL_EXPORTS
#define QYUTIL_API __declspec(dllexport)
#else
#define QYUTIL_API __declspec(dllimport)
#endif

#else

#define QYUTIL_API

#endif //!defined(WIN32) || defined(_WIN32) || defined(WINCE)

#else

#define QYUTIL_API

#endif

#if defined(WIN32) || defined(_WIN32) || defined(WINCE)

#include <windows.h>
typedef HMODULE QyHMODULE;

#else

typedef void* QyHMODULE;

#endif //defined(WIN32) || defined(_WIN32)

#include "qyutil/qydefine.h"

DEFINE_NAMESPACE(qy)

BEGIN_EXTERN_C

QYUTIL_API void    qyutil_initialize(void);
QYUTIL_API void    qyutil_unInitialize(void);

END_EXTERN_C

// for qywndmsg.h
typedef void (*FnMsgPush)(unsigned int msgid,WPARAM wParam , LPARAM& lParam);
typedef void (*FnMsgRemove)(unsigned int msgid,WPARAM wParam , LPARAM& lParam);

END_NAMESPACE(qy)

#endif //__QY_UTIL_H__

