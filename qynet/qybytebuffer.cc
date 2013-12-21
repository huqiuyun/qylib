#include <algorithm>
#include <cassert>

#include "qybasictypes.h"
#include "qybytebuffer.h"

namespace qy{

static const int DEFAULT_SIZE = 2048;

ByteBuffer::ByteBuffer() {
    start_ = 0;
    end_   = 0;
    size_  = DEFAULT_SIZE;
    bytes_ = new char[size_];
}

ByteBuffer::ByteBuffer(const char* bytes, size_t len) {
    start_ = 0;
    end_   = len;
    size_  = len;
    bytes_ = new char[size_];
    memcpy(bytes_, bytes, end_);
}

ByteBuffer::ByteBuffer(const char* bytes) {
    start_ = 0;
    end_   = strlen(bytes);
    size_  = end_;
    bytes_ = new char[size_];
    memcpy(bytes_, bytes, end_);
}

ByteBuffer::~ByteBuffer() {
    delete[] bytes_;
}

bool ByteBuffer::ReadUInt8(uint8& val) {
    return ReadBytes(reinterpret_cast<char*>(&val), 1);
}

bool ByteBuffer::ReadUInt16(uint16& v) {
    if (!ReadBytes(reinterpret_cast<char*>(&v), 2)) {
        return false;
    }
    return true;
}

bool ByteBuffer::ReadUInt32(uint32& val) {
    if (!ReadBytes(reinterpret_cast<char*>(&val), 4)) {
        return false;
    }
    return true;
}

bool ByteBuffer::ReadString(std::string& val, size_t len) {
    if (len > Length()) {
        return false;
    } else {
        val.append(bytes_ + start_, len);
        start_ += len;
        return true;
    }
}

bool ByteBuffer::ReadBytes(char* val, size_t len) {
    if (len > Length()) {
        return false;
    } else {
        memcpy(val, bytes_ + start_, len);
        start_ += len;
        return true;
    }
}

void ByteBuffer::WriteUInt8(uint8 val) {
    WriteBytes(reinterpret_cast<const char*>(&val), 1);
}

void ByteBuffer::WriteUInt16(uint16 val) {
    WriteBytes(reinterpret_cast<const char*>(&val), 2);
}

void ByteBuffer::WriteUInt32(uint32 val) {
    WriteBytes(reinterpret_cast<const char*>(&val), 4);
}

void ByteBuffer::WriteString(const std::string& val) {
    WriteBytes(val.c_str(), val.size());
}

void ByteBuffer::WriteBytes(const char* val, size_t len) {
    if (Length() + len > Capacity())
        Resize(Length() + len);

    memcpy(bytes_ + end_, val, len);
    end_ += len;
}

void ByteBuffer::WriteBytes(int index,const char* val,size_t len)
{
    if( index >= end_ )
    {
        WriteBytes(val,len);
    }
    else if( index >= 0 )
    {
        if( (index + len) <= end_ )
        {
            memcpy(bytes_ + index, val, len);
        }
        else
        {
            size_t l = len - (end_- index );
            if (Length() + l > Capacity())
                Resize(Length() + l);

            memcpy(bytes_ + index, val, len);
            end_ += l;
        }
    }
}

void ByteBuffer::Resize(size_t size) {
    if (size > size_)
        size = _max(size, 3 * size_ / 2);

    size_t len = _min(end_ - start_, size);
    char* new_bytes = new char[size];
    memcpy(new_bytes, bytes_ + start_, len);
    delete [] bytes_;

    start_ = 0;
    end_   = len;
    size_  = size;
    bytes_ = new_bytes;
}

void ByteBuffer::Shift(size_t size) {
    if (size > Length())
        return;

    end_ = Length() - size;
    memmove(bytes_, bytes_ + start_ + size, end_);
    start_ = 0;
}

void ByteBuffer::Clear(void)
{
    start_ = 0;
    end_   = 0;
}
}//namesapce qy
