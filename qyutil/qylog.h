#ifndef __QY_LOG_H__
#define __QY_LOG_H__

#include "qyutil/qyconfig.h"
#include "qyutil/qylog_i.h"
#include <string>

///////////////////////////////////////////////////////
//	***             * * * *      ****        ****    //
//	***  	      *         *     **          **     //
//	***  	     *           *    **          **     //
//	***  	     *           *    **************     //
//	***	     **  *           *    **          **     //
//	***********   *         *     **          **     //
//	***********	    * * * *      ****        ****    // 
///////////////////////////////////////////////////////2009-08-20 @loach

DEFINE_NAMESPACE(qy)

class IQyLogNet
{
public:
    virtual bool write2Net(int type , int level , const char* buffer,size_t len) = 0;
};

class QyLogPrivate;
/*
*
* @author loach.h
*
* @date 2009-08-20
*/

class QYUTIL_API QyLog : public IQyLog
{
private:
	QyLog();
public:
	static QyLog* Instance();
public:
	~QyLog();
    void set_log_net(IQyLogNet* net);
	void set_log_file(const char* filename);
	void enable_type( int type);
	void enable_level( int level /*LogLevelFlags*/);
	virtual bool logString(int type , int level , const char* str);
	virtual bool logBuffer(int type , int level , const char* buffer,size_t len);
	virtual bool logFormat(int type , int level , const char* pszFmt, ...);
private:
	bool isLevel(int level);
	bool write2Net(int type , int level , const char* buffer,size_t len);
private:
	friend class QyLogPrivate;
	QyLogPrivate* d_ptr_;
};


class QyLogStreamPrivate;
class QYUTIL_API QyLogStream
{
public:
	QyLogStream(const char* filiter, int loglevel, const char* cppname, int line, const char* funcname);
	~QyLogStream();

	inline QyLogStream &operator<<(bool b)	  
	{ return (*this) << (b == true ? L"true" : L"false"); }

    inline QyLogStream &operator<<(char ch)	 
	{ return (*this) << (signed long)ch; }

    inline QyLogStream &operator<<(signed short i)
	{ return (*this) << (signed long)i; }

    inline QyLogStream &operator<<(signed int i)	
	{ return (*this) << (signed long)i; }

    inline QyLogStream &operator<<(unsigned int i)	
	{ return (*this) << (unsigned long)i; }

    inline QyLogStream &operator<<(unsigned short i)
	{ return (*this) << (unsigned long)i; }

    inline QyLogStream &operator<<(float f)
	{ return (*this) << (double)f; }

	inline QyLogStream &operator<<(unsigned long i)		 
	{ int n = snprintf(setBuffer(16), 16,  "%lu", i); resize(n); return *this; }

    inline QyLogStream &operator<<(signed long i)		 
	{ int n = snprintf(setBuffer(16), 16,  "%ld", i); resize(n); return *this; }

	inline QyLogStream &operator<<(const void *ptr)		 
	{ int n = snprintf(setBuffer(16), 16,  "0x%08lX", (unsigned long)ptr); resize(n); return *this; }

	inline QyLogStream &operator<<(__int64 i)
#if defined(WIN32)
	{ int n = snprintf(setBuffer(16), 16, "%I64d", i); resize(n); return *this; }
#else
    { int n = snprintf(setBuffer(16), 16, "%lld", i); resize(n); return *this; }
#endif
    
    inline QyLogStream &operator<<(double f)
	{ int n = snprintf(setBuffer(16), 16,  "%0.6f", f);	resize(n); return *this; }

	inline QyLogStream &operator<<(const char* s)
	{ size_t n = strlen(s); memcpy(setBuffer(n), s, n * sizeof(char)); resize(n); return *this; }

    inline QyLogStream &operator<<(const wchar_t *s)
	{ size_t len = wcslen(s); size_t n = toChar(s, len); resize(n); return *this; }

	inline QyLogStream &operator<<(const std::wstring &s)
	{ size_t len = s.length(); size_t n = toChar(s.c_str(), len); resize(n); return *this; }

	inline QyLogStream &operator<<(const std::string &s)
	{ size_t n = s.length(); memcpy(setBuffer(n), s.c_str(), n * sizeof(char)); resize(n); return *this; }


private:
	/** unicode convert */
	size_t    toChar(const wchar_t* s , size_t len);
	void      resize(size_t n);
	/** buffer */
	char*     setBuffer(size_t cch);

private:
	friend class QyLogStreamPrivate;
	QyLogStreamPrivate* d_ptr_;
};


#define LogFinal(filter_name)	QyLogStream(filter_name, IQyLog::Info, __FILE__, (WORD)__LINE__, __FUNCTION__)

#define LogDev(filter_name)		QyLogStream(filter_name, IQyLog::Info, __FILE__, (WORD)__LINE__, __FUNCTION__)

#define LogTemp(filter_name)	QyLogStream(filter_name, IQyLog::Debug, __FILE__, (WORD)__LINE__, __FUNCTION__)

END_NAMESPACE(qy)

#endif //__QY_LOG_H__