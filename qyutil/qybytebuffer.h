/**
*  @brief String opeartion.
*  @file  qybytebuffer.h
*  @author loach 
*  @Email < loachmr@sina.com >
*
* Copyright (C) 1996-2010 loach, All Rights Reserved
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*/
#pragma once

#include "qyutil/qyconfig.h"
#include "qyutil/qyctypes.h"
#include "qyutil/qyiodevice.h"
#include <string>

DEFINE_NAMESPACE(qy)
/**
 * @date 2009-09-08
 *
 * @author by loach
 *
*/
class QYUTIL_API QyByteBuffer : public QyIODevice
{
public:
	QyByteBuffer();
	QyByteBuffer(int size);
	QyByteBuffer(const char* bytes, size_t len);
	~QyByteBuffer();

	QyByteBuffer(const QyByteBuffer& buffer);
	QyByteBuffer& operator = (const QyByteBuffer& buffer);

	const char* data() const { return bytes_ + start_; }
	size_t length() const { return end_ - start_; }
	size_t capacity() const { return size_ - start_; }
	bool empty() const { return (length() == 0);}
	void clear(void);
	void remove(size_t index , size_t size);

	/** 调整内存大小*/
	void resize(size_t size);
	void shift(size_t size);

	// >> read
	QyByteBuffer& operator>>(char& val);
	QyByteBuffer& operator>>(uint8& val);

	QyByteBuffer& operator>>(short& val);
	QyByteBuffer& operator>>(uint16& val);

	QyByteBuffer& operator>>(int& val);
	QyByteBuffer& operator>>(uint& val);

	QyByteBuffer& operator>>(int64& val);
	QyByteBuffer& operator>>(uint64& val);

	QyByteBuffer& operator>>(float& val);
	QyByteBuffer& operator>>(double& val);
	QyByteBuffer& operator>>(std::string& val);
	QyByteBuffer& operator>>(QyByteBuffer& val);

	bool readUInt8(uint8& val);
	bool readUInt16(uint16& val);
	bool readUInt32(uint32& val);
	bool readString(std::string& val, size_t len = 0); // append to val
	bool read(char* val, size_t len);
	bool read(size_t index ,char* val ,size_t len);

	// << write

	QyByteBuffer& operator<<(const char& val);
	QyByteBuffer& operator<<(const uint8& val);

	QyByteBuffer& operator<<(const short& val);
	QyByteBuffer& operator<<(const uint16& val);

	QyByteBuffer& operator<<(const int& val);
	QyByteBuffer& operator<<(const uint& val);

	QyByteBuffer& operator<<(const int64& val);
	QyByteBuffer& operator<<(const uint64& val);

	QyByteBuffer& operator<<(const float& val);
	QyByteBuffer& operator<<(const double& val);

	QyByteBuffer& operator<<(const char* val);
	QyByteBuffer& operator<<(const std::string& val);
	QyByteBuffer& operator<<(const QyByteBuffer& val);

	void writeUInt8(uint8 val);
	void writeUInt16(uint16 val);
	void writeUInt32(uint32 val);
	void logString(const std::string& val);
	void write(const char* val, size_t len);

	void write(size_t index,const char* val,size_t len);

private:
	void construct(int size);
private:
	char* bytes_;
	size_t size_;
	size_t start_;
	size_t end_;
};

#if defined(_USE_BYTEINLINE)
#include "qybytebuffer.inl"
#endif // _USE_BYTEINLINE

END_NAMESPACE(qy)