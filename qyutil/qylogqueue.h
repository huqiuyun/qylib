#ifndef __QY_LOGQUEUE_H__
#define __QY_LOGQUEUE_H__

/**
 * thread queue for log
 *
 * @author loach
 */

#include "qyutil/qythreadqueue.h"
#include "qyutil/qyosstring.h"
#include "qyutil/qyos.h"
#include "qyutil/qylog_i.h"
#include "qyutil/qyrefptr.h"

#include <list>
#include <cassert>
#include <fstream>
#include <iosfwd>
#include <time.h>
#include <string.h>

DEFINE_NAMESPACE(qy)

class QyLogFile {
public:
    QyLogFile()
    :file_(0){
    }
    
    ~QyLogFile(){
        close();
    }
    
    void setFileName(const char* filename){
        QyAutoLocker cs(&cs_);
        close();
        if (filename) {
            filename_ = filename;
        }
        else{
            filename_ = "";
        }
    }
    
    FILE* open(int type) {
        
        QyAutoLocker cs(&cs_);
        if (filename_.empty()) {
            return 0;
        }
        const char* mode = 0;
        if (0 != (type & IQyLog::FileAppend)) {
            mode = "a+b";
        }else if (0 != (type & IQyLog::File)) {
            mode = "w+b";
            close();
        }
        else
        {
            return 0;
        }
        if (!file_) {
            file_ = fopen(filename_.c_str(), mode);
            if (file_ && 0 != (type & IQyLog::UnicodeFile)) {
                unsigned char uniHead[2] = {0xFF,0xFE};
                fwrite(uniHead , sizeof(char) , 2 , file_);
            }
        }
        return file_;
    }
    
    void close() {
        if (file_){
            fclose(file_);
            file_ = 0;
        }
    }

    const char* filename() const { return filename_.c_str();}
private:
    QySmartLock<QY_CS> cs_;
    std::string  filename_;
    FILE* file_;
};

template<class T>
class QyLogData
{
public:
	size_t len;
	T*  value;
    
	int type;
	int level;
    
	QyRefPtrT<QyLogFile> file;
    
	QyLogData()
	{
		type = IQyLog::File;
		level= IQyLog::Unknown;
		len = 0;
		value  = NULL;
	}
	void release()
	{
		if (value){
			delete[] value;
			value = NULL;
		}
		len = 0;
		delete this;
	}
};

template <class T>
class QyLogBuffer {
public:
    QyLogBuffer()
    :data_(0){
    }
    QyLogBuffer(QyLogData<T>* data)
    :data_(data){
    }
    
    QyLogData<T>* data_;
};

template <class T,class LogBuffer = QyLogBuffer<T> >
class QyLogT
: public QyThreadQueue< LogBuffer >
, public IQyThreadHandlerT< LogBuffer >
{
public:
	QyLogT()
    :QyThreadQueue<LogBuffer>(this){
    }
	~QyLogT(){}
public:
    bool write(const QyRefPtrT<QyLogFile>& file, int type, int level, const T* lpszContent,size_t len){
		if (len == 0 || !lpszContent)
			return false;
        
		T* lpLogContent = new  T[len+sizeof(T)];
		if (!lpLogContent){
			assert(0);
			return false;
		}
		memcpy(lpLogContent, lpszContent , sizeof(T)*len);
		lpLogContent[len] ='\0';
        
        QyLogData<T>* p = new QyLogData<T>;
        p->len = len;
        p->value = lpLogContent;
        p->type = type;
        p->level= level;
        p->file = file;
        this->push( LogBuffer(p));
        return true;
	}
    
    bool write(const QyRefPtrT<QyLogFile>& file , int type , int level, const T* lpszFmt, ...){
		assert(lpszFmt);
        
		va_list argp;
		va_start(argp, lpszFmt);
        
        int nLogContentLen = qyos_vsnprintf(NULL, 0, lpszFmt, argp);
		if (-1 == nLogContentLen || nLogContentLen == 0){
			va_end(argp);
			return false;
		}
        
		T* lpLogContent = new T[nLogContentLen+sizeof(T)];
		if (!lpLogContent){
			va_end(argp);
			return false;
		}
		qyos_vsnprintf(lpLogContent, nLogContentLen, lpszFmt, argp);
		lpLogContent[nLogContentLen] = '\0';
        va_end(argp);
        
        QyLogData<T>* p = new QyLogData<T>;
        p->len = nLogContentLen;
        p->value = lpLogContent;
        p->type = type;
        p->level= level;
        p->file = file;
        this->push( LogBuffer(p));
		return true;
	}
protected:
    bool writelog(const LogBuffer& logbuff){
        
        QyLogData<T>* logdata = logbuff.data_;
        if (!logdata) {
            return false;
        }
        writeLogData(logdata);
        logdata->release();
		return true;
	}
    
    void writeLogData(QyLogData<T>* logdata) {
        if (!logdata->value || logdata->len == 0){
            return ;
        }
        char* pszTime = 0;
        char timebuf[64]={0};
        
        time_t ltime;
        time(&ltime);
        
        struct tm* ltm = localtime(&ltime);
        
        strftime(timebuf , 64 , "%A %B %H:%M:%S %Y  ",ltm);
        pszTime = &timebuf[0];
        
        // write file
        FILE* file = logdata->file->open(logdata->type);
        if (file)
        {
            // level
            const char* szLevel = getlevel(logdata->level);
            if (szLevel)
            {
                fwrite(szLevel ,sizeof(char),qyos_strlen(szLevel),file);
            }
            
            // time or value
            if (*pszTime != '\0')
            {
                fwrite(pszTime ,sizeof(char), qyos_strlen(pszTime), file);
            }
            
            fwrite(logdata->value ,sizeof(T),(logdata->len),file);
            
            fputwc(L'\r',file);
            fputwc(L'\n',file);
        }
    }
    
    const char* getlevel(int level){
        const char* p = 0;
        switch(level){
            case IQyLog::Unknown:
                p = "UNKNOWN: ";
                break;
            case IQyLog::Crit:
                p = "CRIT: ";
                break;
            case IQyLog::Err:
                p = "ERROR: ";
                break;
            case IQyLog::Warning:
                p = "WARNING: ";
                break;
            case IQyLog::Info:
                p = "INFO: ";
                break;
            case IQyLog::Debug:
                p = "DEBUG: ";
                break;
            case IQyLog::Stack:
                p = "STACK: ";
                break;
            case IQyLog::StdErr:
                p = "STDERR: ";
                break;
            case IQyLog::Bogus:
                p = "BOGUS: ";
                break;
        }
        return p;
    }
private:
    int onTQHandler(const LogBuffer& logBuffer,eThreadEvent e) {
        QY_UNUSED(e);
        this-> writelog(logBuffer);
        return kTResNone;
    }
};

END_NAMESPACE(qy)

#endif //__QY_LOGQUEUE_H__