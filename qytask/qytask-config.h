#ifndef QY_TASKCONFIG_H__
#define QY_TASKCONFIG_H__

#include "qysysconfig.h"
#include "qydefine.h"
#include "qyctypes.h"

#ifdef QYTASK_DLL

#if defined(H_OS_WIN)

#ifdef QYTASK_EXPORTS
#define QYTASK_API __declspec(dllexport)
#else
#define QYTASK_API __declspec(dllimport)
#endif

#else

#define QYTASK_API

#endif //!defined(H_OS_WIN)

#else

#define QYTASK_API

#endif
#endif //QY_TASKCONFIG_H__
