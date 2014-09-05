#include <algorithm>
#include <cassert>

#include "qybasictypes.h"
#include "qyByteBuffer.h"

namespace qy{

static const int DEFAULT_SIZE = 2048;

QyByteBuffer::QyByteBuffer() {
    mStart = 0;
    mEnd   = 0;
    mSize  = DEFAULT_SIZE;
    mBytes = new char[mSize];
}

QyByteBuffer::QyByteBuffer(const char* bytes, size_t len) {
    mStart = 0;
    mEnd   = len;
    mSize  = len;
    mBytes = new char[mSize];
    memcpy(mBytes, bytes, mEnd);
}

QyByteBuffer::QyByteBuffer(const char* bytes) {
    mStart = 0;
    mEnd   = strlen(bytes);
    mSize  = mEnd;
    mBytes = new char[mSize];
    memcpy(mBytes, bytes, mEnd);
}

QyByteBuffer::~QyByteBuffer() {
    delete[] mBytes;
}

bool QyByteBuffer::ReadUInt8(uint8& val) {
    return ReadBytes(reinterpret_cast<char*>(&val), 1);
}

bool QyByteBuffer::ReadUInt16(uint16& v) {
    if (!ReadBytes(reinterpret_cast<char*>(&v), 2)) {
        return false;
    }
    return true;
}

bool QyByteBuffer::ReadUInt32(uint32& val) {
    if (!ReadBytes(reinterpret_cast<char*>(&val), 4)) {
        return false;
    }
    return true;
}

bool QyByteBuffer::ReadString(std::string& val, size_t len) {
    if (len > Length()) {
        return false;
    } else {
        val.append(mBytes + mStart, len);
        mStart += len;
        return true;
    }
}

bool QyByteBuffer::ReadBytes(char* val, size_t len) {
    if (len > Length()) {
        return false;
    } else {
        memcpy(val, mBytes + mStart, len);
        mStart += len;
        return true;
    }
}

void QyByteBuffer::WriteUInt8(uint8 val) {
    WriteBytes(reinterpret_cast<const char*>(&val), 1);
}

void QyByteBuffer::WriteUInt16(uint16 val) {
    WriteBytes(reinterpret_cast<const char*>(&val), 2);
}

void QyByteBuffer::WriteUInt32(uint32 val) {
    WriteBytes(reinterpret_cast<const char*>(&val), 4);
}

void QyByteBuffer::WriteString(const std::string& val) {
    WriteBytes(val.c_str(), val.size());
}

void QyByteBuffer::WriteBytes(const char* val, size_t len) {
    if (Length() + len > Capacity())
        Resize(Length() + len);

    memcpy(mBytes + mEnd, val, len);
    mEnd += len;
}

void QyByteBuffer::WriteBytes(uint32 index,const char* val,size_t len)
{
    if (index >= mEnd)
    {
        WriteBytes(val,len);
    }
    else
    {
        if( (index + len) <= mEnd )
        {
            memcpy(mBytes + index, val, len);
        }
        else
        {
            size_t l = len - (mEnd- index );
            if (Length() + l > Capacity())
                Resize(Length() + l);

            memcpy(mBytes + index, val, len);
            mEnd += l;
        }
    }
}

void QyByteBuffer::Resize(size_t size) {
    if (size > mSize)
        size = _max(size, 3 * mSize / 2);

    size_t len = _min(mEnd - mStart, size);
    char* new_bytes = new char[size];
    memcpy(new_bytes, mBytes + mStart, len);
    delete [] mBytes;

    mStart = 0;
    mEnd   = len;
    mSize  = size;
    mBytes = new_bytes;
}

void QyByteBuffer::Shift(size_t size) {
    if (size > Length())
        return;

    mEnd = Length() - size;
    memmove(mBytes, mBytes + mStart + size, mEnd);
    mStart = 0;
}

void QyByteBuffer::Clear(void)
{
    mStart = 0;
    mEnd   = 0;
}
}//namesapce qy
