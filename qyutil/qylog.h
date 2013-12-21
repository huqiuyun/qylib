#ifndef __QY_LOG_H__
#define __QY_LOG_H__

#include "qyutil/qyconfig.h"
#include <string>

DEFINE_NAMESPACE(qy)

enum eQyLogType
{
	kQyLogCout   = 0x1, // output wprintf_s
	kQyLogCerr   = 0x02, // output std::cerr
	kQyLogStderr = 0x04, // output std handler,for win32
    kQyLogFile   = 0x08, // output file
    kQyLogDebug  = 0x10, // output Debug string for win32 api
	kQyLogNet    = 0x20, // internet, Need to upload to server
	kQyLogAll    = 0xFFFF,
};

enum eQyLogLevel
{
	kQyLogUnk_LL     = 0x0001,
    kQyLogErr_LL     = 0x0002,
	kQyLogWarning_LL = 0x0004,
	kQyLogInfo_LL    = 0x0008,
	kQyLogDebug_LL   = 0x0010,
	kQyLogDev_LL     = 0x0020,
	kQyLogAll_LL     = 0xFFFF,
};

struct QYNovtableM IQyLogOut
{
	virtual bool isLog(int type,int level) const = 0;
	virtual void logOut(int type,int level, const wchar_t*, int length) = 0;
};

struct QYNovtableM IQyLog
{
	virtual void open(const wchar_t* filename,size_t file_max = 2*1024*1024/*2M*/) = 0;
	virtual void close() = 0;

	virtual void enableType(int type) = 0;
	virtual void enableLevel(int level) = 0;

	virtual void logF(int type, int level, const wchar_t* format, ...) = 0;
	virtual void logS(int type, int level, const wchar_t* log) = 0;
	virtual void logS(int type, int level, const wchar_t* log, size_t length) = 0;
	virtual void logS(int type, int level, const std::wstring& str) = 0;

	virtual void release() = 0;
};

BEGIN_EXTERN_C
// LOG
QYUTIL_API IQyLog* qyutil_logCreate();

QYUTIL_API void    qyutil_logSet(IQyLogOut* out);
QYUTIL_API void    qyutil_log(int type, int level, const wchar_t* format, ...);

END_EXTERN_C

END_NAMESPACE(qy)

#define QY_LOG    qy::qyutil_log

#endif /*__QY_ILOG_H__ */
