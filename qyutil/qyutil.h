#ifndef __QY_UTIL_H__
#define __QY_UTIL_H__

#ifdef QY_UTIL_DLL

#if defined(WIN32) || defined(_WIN32) || defined(WINCE)

#ifdef QY_UTIL_EXPORTS
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

/** 初始化 */
QYUTIL_API void qyutil_initialize(void);

/** 反初始 */
QYUTIL_API void qyutil_unInitialize(void);

END_EXTERN_C

END_NAMESPACE(qy)

#endif //__QY_UTIL_H__

