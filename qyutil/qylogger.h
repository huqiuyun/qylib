#ifndef __QY_LOGGER_H__
#define __QY_LOGGER_H__

#include "qyutil/qyconfig.h"
#include "qyutil/qylog.h"


///////////////////////////////////////////////////////
//	***             * * * *      ****        ****    //
//	***  	      *         *     **          **     //
//	***  	     *           *    **          **     //
//	***  	     *           *    **************     //
//	***	     **  *           *    **          **     //
//	***********   *         *     **          **     //
//	***********	    * * * *      ****        ****    // 
///////////////////////////////////////////////////////2009-08-20 @loach
/*
*
* 此库可以由读者用于学习之用,不能用于商业用途
* 如果用在商业用途,一块法律追究自己承担!
*
* 作者： loach.h ( 胡秋云)
*
* 时间： 2009-08-20
*/
DEFINE_NAMESPACE(qy)

class QyLogPrivate;
class QYUTIL_API QyLog : public IQyLog
{	
public:
	~QyLog();
	QyLog();

	static QyLog* instance();

	void logInfo(const wchar_t *format, ...);
	void logDev(const wchar_t *format, ...);
	void logWaring(const wchar_t *format, ...);
	void logError(const wchar_t *format, ...);
	void logFatal(const wchar_t *format, ...);
	void logDebug(const wchar_t *format, ...);
public:
	void open(const wchar_t* filename,size_t file_max = 2*1024*1024/*2M*/);
	void close();

	void enableType(int type);
	void enableLevel(int level);

	void logF(int type, int level,const wchar_t* format, ...);
	void logS(int type, int level,const wchar_t* log);
	void logS(int type, int level,const wchar_t* log, size_t length);
	void logS(int type, int level,const std::wstring& str);

	/** delete this */
	void release();
private:
	void create(int type);

	friend class QyLogPrivate;
	QyLogPrivate* d_ptr_;
};

END_NAMESPACE(qy)

#define LOGINFO	    qy::QyLog::instance()->logInfo
#define LOGDEV	    qy::QyLog::instance()->logDev
#define LOGWARING	qy::QyLog::instance()->logWaring
#define LOGERROR	qy::QyLog::instance()->logError
#define LOGFATAL	qy::QyLog::instance()->logFatal
#define LOGDEBUG	qy::QyLog::instance()->logDebug

#endif //__QY_LOGGER_H__