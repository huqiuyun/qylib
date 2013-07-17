#include "qylog.h"
#include "qylogqueue.h"
#include "qydirectory.h"
#include "qycodec.h"
#include "qyosstring.h"
#include <string.h>
#include <assert.h>
#include <limits>

DEFINE_NAMESPACE(qy)

class QyLogPrivate
{
public:
	QyLogPrivate()
    :log_file_(new QyLogFile())
	,net_(NULL){
    }
	int                  enable_type_;
	int                  level_flag_;
    IQyLogNet*           net_;
    QyRefPtrT<QyLogFile> log_file_;
	QyLogT<char>         logger_;
};

QyLog::QyLog()
:d_ptr_(new QyLogPrivate())
{
	enable_type(IQyLog::All);
	enable_level(IQyLog::LL_All);
}

QyLog::~QyLog()
{
	delete d_ptr_;
}

QyLog* QyLog::Instance()
{
	static QyLog Inst;
	return &Inst;
}

void QyLog::set_log_file(const char* filename)
{
	QY_D(QyLog);
	QyDirectory dir;
    dir.createDirectoryA(filename);
    d->log_file_->setFileName(filename);
}

void QyLog::set_log_net(IQyLogNet* net)
{
    QY_D(QyLog);
    d->net_ = net;
}

void QyLog::enable_type( int type)
{
	QY_D(QyLog);
	d->enable_type_ = type;
}

void QyLog::enable_level( int levelflag /*LogLevelFlags*/)
{
	QY_D(QyLog);
	d->level_flag_ = levelflag;
}

bool QyLog::logString(int type , int level , const char* str)
{
	QY_D(QyLog);
	type = d->enable_type_ & type;
    
	if (str && *str != '\0')
	{
		return logBuffer(type , level , str , qyos_strlen(str));
	}
	return false;
}

bool QyLog::logBuffer(int type , int level , const char* buffer, size_t len)
{
	QY_D(QyLog);
	type = d->enable_type_ & type;
    
	if (0 != (type & IQyLog::Internet))
	{
		write2Net(type , level , buffer , len);
	}
	if (!isLevel(level))
	{
		return false;
	}
	return d->logger_.write(d->log_file_ , type , level , buffer , len);
}

bool QyLog::logFormat(int type , int level , const char* pszFmt, ...)
{
	QY_D(QyLog);
	type = d->enable_type_ & type;
    
	va_list argp;
	va_start(argp, pszFmt);
    
	char* buffer = NULL;
	int nLen = qyos_vsnprintf(NULL, 0, pszFmt, argp);
	if (-1 == nLen || nLen == 0)
	{
		va_end(argp);
		return false;
	}
	buffer = new char[nLen+1];
	qyos_vsnprintf(buffer, nLen, pszFmt, argp);
	buffer[nLen]='\0';
	va_end(argp);
    
	bool bRetVal = logBuffer(type, level , buffer , nLen);
    
	delete[] buffer;
    
	return bRetVal;
}

bool QyLog::write2Net(int type , int level, const char* buffer, size_t len)
{
	QY_D(QyLog);
    if (d->net_)
    {
        return d->net_->write2Net(type,level,buffer,len);
    }
	return false;
}

bool QyLog::isLevel(int level)
{
    QY_D(QyLog);
	switch(level)
	{
        case IQyLog::Unknown:
            return (d->level_flag_ & IQyLog::Unknown_flags);
        case IQyLog::Crit:
            return (d->level_flag_ & IQyLog::Crit_flag);
        case IQyLog::Err:
            return (d->level_flag_ & IQyLog::Err_flag);
        case IQyLog::Warning:
            return (d->level_flag_ & IQyLog::Warning_flag);
        case IQyLog::Info:
            return (d->level_flag_ & IQyLog::Info_flag);
        case IQyLog::Debug:
            return (d->level_flag_ & IQyLog::Debug_flag);
        case IQyLog::Stack:
            return (d->level_flag_ & IQyLog::Stack_flag);
        case IQyLog::StdErr:
            return (d->level_flag_ & IQyLog::StdErr_flag);
        case IQyLog::Bogus:
            return (d->level_flag_ & IQyLog::Bogus_flag);
	}
	return false;
}

//
class QyLogStreamPrivate
{
public:
	QyLogStreamPrivate()
	{
	}
    
	~QyLogStreamPrivate()
	{
		const char* cppFileName = strrchr(cppName_, QY_PATH_SEPARATOR);
        if (cppFileName)
        {
            cppFileName += 1;
        }
        else
        {
            cppFileName = cppName_;
        }
        QyLog::Instance()->logFormat(IQyLog::FileAppend , logLevel_ ,
                                       "(%s) : %s line(%d) function[ %s ]: %s" ,
                                       filter_, cppName_, lineNumber_ , funcName_ , data_);
		if (data_ != buffer_) {
            delete [] data_;
            data_ = 0;
        }
	}
	char		 buffer_[512];
	char *	     data_;
	size_t       cchdata_;
	size_t       cchmax_;
    
	const char*	 filter_;
	int     	 logLevel_;
	const char*	 cppName_;
	int		     lineNumber_;
	const char*	 funcName_;
};
//

QyLogStream::QyLogStream(const char* filter,
                         int logLevel, const char* cppName,
                         int lineNumber, const char* funcName)
:d_ptr_(new QyLogStreamPrivate())
{
	QY_D(QyLogStream);
    
	d->buffer_[0] = 0;
	d->data_ = d->buffer_;
	d->cchmax_ = sizeof(d->buffer_)/sizeof(wchar_t) - 1;
	d->cchdata_ = 0;
	d->filter_ = filter;
	d->logLevel_ = logLevel;
	d->cppName_ = cppName;
	d->lineNumber_ = lineNumber;
	d->funcName_ = funcName;
}

QyLogStream::~QyLogStream()
{
	delete d_ptr_;
}

size_t QyLogStream::toChar(const wchar_t *s , size_t len)
{
    QyW2C w2c(s);
    size_t bufflen = len*sizeof(wchar_t);
    char* buffer = setBuffer(bufflen);
    strncpy(buffer,w2c.c_str(),w2c.length());
    return bufflen;
}

void QyLogStream::resize(size_t n)
{
	QY_D(QyLogStream);
	if (n > 0)
	{
		d->cchdata_ += n;
	}
	d->data_[d->cchdata_] = 0;
}

char * QyLogStream::setBuffer(size_t cch)
{
	QY_D(QyLogStream);
	if (d->cchdata_ + cch > d->cchmax_)
	{
		size_t cchAlloc = sizeof(d->buffer_)/sizeof(char);
		while (cchAlloc < d->cchdata_ + cch + 1) cchAlloc *= sizeof(char);
        
		char *p = new char[cchAlloc];
		memcpy(p, d->data_, d->cchdata_ * sizeof(char));
		p[d->cchdata_] = 0;
		if (d->data_ != d->buffer_)
		{
			delete [] d->data_;
		}
		d->data_ = p;
		d->cchmax_ = cchAlloc - 1;
	}
	return d->data_ + d->cchdata_;
}

END_NAMESPACE(qy)
