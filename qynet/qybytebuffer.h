#ifndef QY_BYTEBUFFER_H__
#define QY_BYTEBUFFER_H__

#include <string>
#include "qybasictypes.h"

namespace qy
{
    class QyByteBuffer
	{
	public:
        QyByteBuffer();
        QyByteBuffer(const char* bytes, size_t len);
        QyByteBuffer(const char* bytes); // uses strlen
        ~QyByteBuffer();

        const char* Data() const { return mBytes + mStart; }
        size_t Length() { return mEnd - mStart; }
        size_t Capacity() { return mSize - mStart; }

		bool ReadUInt8(uint8& val);
		bool ReadUInt16(uint16& val);
		bool ReadUInt32(uint32& val);
		bool ReadString(std::string& val, size_t len); // append to val
		bool ReadBytes(char* val, size_t len);

		void WriteUInt8(uint8 val);
		void WriteUInt16(uint16 val);
		void WriteUInt32(uint32 val);
		void WriteString(const std::string& val);
		void WriteBytes(const char* val, size_t len);
        void WriteBytes(uint32 index,const char* val,size_t len);

		void Resize(size_t size);
		void Shift(size_t size);

		//
		void Clear();
	private:
        char* mBytes;
        size_t mSize;
        size_t mStart;
        size_t mEnd;
	};
}

#endif // QY_BYTEBUFFER_H__
