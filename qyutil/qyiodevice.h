#pragma once

#include "qyutil/qyconfig.h"

DEFINE_NAMESPACE(qy)

class QYUTIL_API QyIODevice
{
public:
	QyIODevice() {}
	virtual ~QyIODevice(){}

	virtual bool open()  { return false;}
	virtual void close() {};

	virtual bool read(char* val, size_t len) = 0;
	virtual bool read(size_t index , char* val, size_t len) = 0;

	virtual void write(const char* val, size_t len) = 0;
	virtual void write(size_t index , const char* val , size_t len) = 0;

	virtual const char* data() const = 0;
	virtual size_t length() const  = 0;
	//
	virtual void clear() {};
    virtual void remove(size_t index,size_t size) {}

	virtual void shift(size_t size) {}
	virtual void resize(size_t size) {}
};

END_NAMESPACE(qy)