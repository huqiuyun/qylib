#include "qybytebuffer.h"
#include "qyosstring.h"
#include <stdlib.h>
#include <stdio.h>

DEFINE_NAMESPACE(qy)

static const int DEFAULT_SIZE = 32;

QyByteBuffer::QyByteBuffer()
{
	construct(1);
}

QyByteBuffer::QyByteBuffer(int size)
{
	if (size <= 0)
	{
		size = 1;
	}
	construct(size);
}

void QyByteBuffer::construct(int size)
{
	start_ = 0;
	_end_   = 0;
	size_  = size;
	bytes_ = new char[size_+1];
	bytes_[size_] ='\0';
}

QyByteBuffer::QyByteBuffer(const char* bytes, size_t len)
{
	start_ = 0;
	_end_   = len;
	size_  = len;
	bytes_ = new char[size_+1];
	bytes_[size_] = '\0';
	memcpy(bytes_, bytes, _end_);
}

QyByteBuffer::QyByteBuffer(const QyByteBuffer& buffer)
:bytes_(0)
,start_(0)
,size_(0)
,_end_(0)
{
	*this = buffer;
}

QyByteBuffer& QyByteBuffer::operator = (const QyByteBuffer& buffer)
{
	start_ = 0;
	_end_   = 0;
	write( buffer.data() , buffer.length());
	return *this;
}

QyByteBuffer::~QyByteBuffer() 
{
	if (bytes_) delete[] bytes_;
}

bool QyByteBuffer::readUInt8(uint8& val)
{
	return read(reinterpret_cast<char*>(&val), 1);
}

bool QyByteBuffer::readUInt16(uint16& v) 
{
	return read(reinterpret_cast<char*>(&v), 2);
}

bool QyByteBuffer::readUInt32(uint32& val)
{
	return read(reinterpret_cast<char*>(&val), 4);
}

bool QyByteBuffer::readString(std::string& val, size_t len) 
{
	if (len > length()) 
	{
		return false;
	} 
	else 
	{
		if (len == 0)
		{
			len = length();
		}
		val.append(bytes_ + start_, len);
		start_ += len;
		return true;
	}
}

bool QyByteBuffer::read(char* val, size_t len)
{
	if (len > length())
	{
		return false;
	}
	else
	{
		memcpy(val, bytes_ + start_, len);
		start_ += len;
		return true;
	}
}

bool QyByteBuffer::read(size_t index,char* val,size_t len)
{
    if (index > 0 && index < length() && len > 0)
	{
		if (index + len > length())
		{
			len = length() - index;
		}
		memcpy(val, bytes_ + index, len);
		start_ = index + len;
		return true;
	}
	return false;
}

void QyByteBuffer::writeUInt8(uint8 val) 
{
	write(reinterpret_cast<const char*>(&val), 1);
}

void QyByteBuffer::writeUInt16(uint16 val)
{
	write(reinterpret_cast<const char*>(&val), 2);
}

void QyByteBuffer::writeUInt32(uint32 val) 
{
	write(reinterpret_cast<const char*>(&val), 4);
}

void QyByteBuffer::logString(const std::string& val)
{
	write(val.c_str(), val.size());
}

void QyByteBuffer::write(const char* val, size_t len) 
{
	if (length() + len > capacity())
	{
		resize(length() + len);
	}
	memcpy(bytes_ + _end_, val, len);
	_end_ += len;
}

void QyByteBuffer::write(size_t index,const char* val,size_t len)
{
	if (index >= _end_)
	{
		write(val,len);
	}
	else if (index != SIZE_MAX)
	{
		if ((index + len) <= _end_)
		{
			memcpy(bytes_ + index, val, len);
		}
		else
		{
			size_t l = len - (_end_- index);
			if (length() + l > capacity())
				resize(length() + l);

			memcpy(bytes_ + index, val, len);
			_end_ += l;
		}
	}
}

void QyByteBuffer::resize(size_t size) 
{
	if (size > size_)
	{
		size = MAX_(size, 3 * size_ / 2);
	}
	size_t len = MIN_(_end_ - start_, size);
	char* new_bytes = new char[size+1];
	memcpy(new_bytes, bytes_ + start_, len);
	delete [] bytes_;

	start_ = 0;
	_end_   = len;
	size_  = size;
	bytes_ = new_bytes;
	bytes_[size_] = '\0';
}

void QyByteBuffer::shift(size_t size) 
{
	if (size > length())
		return;

	_end_ = length() - size;
	memmove(bytes_, bytes_ + start_ + size, _end_);
	start_ = 0;
}

void QyByteBuffer::remove(size_t index , size_t size)
{
	if (index >= _end_)
	{
		return;
	}
	else if (index != SIZE_MAX)
	{
        size_t len = (index + size);
		if (len >= _end_)
		{
            size = _end_ - index;
			_end_ = index;
		}
        else
        {
            _end_ -= size;
        }
	}
}

void QyByteBuffer::clear(void)
{
	shift(0);
}

#if !defined(_USE_BYTEINLINE)
#include "qybytebuffer.inl"
#endif //!_USE_BYTEINLINE


END_NAMESPACE(qy)