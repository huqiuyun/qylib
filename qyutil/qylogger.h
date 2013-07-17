#ifndef __QY_LOGGER_H__
#define __QY_LOGGER_H__

#include "qyutil/iqylog.h"
#include "qyutil/qylog.h"
/**
 * 日志接口
 *
 * @author loach
 *
 * @date 2009-09-03
*/
class QyLogger
{
public:
	QyLogger()
		:log_(0)
	{
		log_ = QyLog::Instance();
	}
	~QyLogger()
	{		
	}
	IQyLog* Logger(void) const { return log_; }
private:
	IQyLog* log_;
};

// 宏定义
#define USE_QYLOG QyLogger logger; QyLogger* pLogger = &logger;

#define LOG_WRITE_B(type , level , buffer , len)      \
	if (pLogger && pLogger->Logger()) pLogger->Logger()->logBuffer(type,level , buffer , len)

#define LOG_WRITE_S(type , level , str)                \
	if (pLogger && pLogger->Logger()) pLogger->Logger()->logString(type,level , str)

#define LOG_WRITE_DEBUG(level , str)                   \
	if (pLogger && pLogger->Logger()) pLogger->Logger()->logString(IQyLog::VSDebugWindow, level , str)

#define LOG_WRITE_FILE(level , str)                    \
	if (pLogger && pLogger->Logger()) pLogger->Logger()->logString(IQyLog::FileAppend ,level, str)

#define LOG_WRITE_F if (pLogger && pLogger->Logger()) \
	pLogger->Logger()->logFormat

#endif //__QY_LOGGER_H__