#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string>

#include "qybasictypes.h"
#include "qystream.h"

#ifdef H_OS_WIN
#include "qywin32.h"
#define fileno _fileno
#endif
#include "qydebug.h"

namespace qy
{

	///////////////////////////////////////////////////////////////////////////////

    StreamResult QyStreamInterface::writeAll(const void* data, size_t data_len,
		size_t* written, int* error)
	{
		StreamResult result = SR_SUCCESS;
		size_t total_written = 0, current_written;
		while (total_written < data_len) 
		{
            result = write(static_cast<const char*>(data) + total_written,
				data_len - total_written, &current_written, error);
			if (result != SR_SUCCESS)
				break;
			total_written += current_written;
		}
		if (written)
			*written = total_written;
		return result;
	}

    StreamResult QyStreamInterface::readAll(void* buffer, size_t buffer_len,
        size_t* result_read, int* error)
	{
		StreamResult result = SR_SUCCESS;
        size_t total_read = 0, current_read;
        while (total_read < buffer_len) {
            result = read(static_cast<char*>(buffer) + total_read,
                buffer_len - total_read, &current_read, error);
			if (result != SR_SUCCESS)
				break;
            total_read += current_read;
		}
        if (result_read)
            *result_read = total_read;
		return result;
	}

    StreamResult QyStreamInterface::readLine(std::string* line)
	{
		StreamResult result = SR_SUCCESS;
		while (true) {
			char ch;
            result = read(&ch, sizeof(ch), NULL, NULL);
			if (result != SR_SUCCESS) {
				break;
			}
			if (ch == '\n') {
				break;
			}
			line->push_back(ch);
		}
		if (!line->empty()) {   // give back the line we've collected so far with
			result = SR_SUCCESS;  // a success code.  Otherwise return the last code
		}
		return result;
	}

	///////////////////////////////////////////////////////////////////////////////
    // QyStreamTap
	///////////////////////////////////////////////////////////////////////////////

    QyStreamTap::QyStreamTap(QyStreamInterface* stream, QyStreamInterface* tap)
        : QyStreamAdapterInterface(stream), mTap(NULL), mTapResult(SR_SUCCESS),
        mTapError(0)
	{
		AttachTap(tap);
	}

    void QyStreamTap::AttachTap(QyStreamInterface* tap)
	{
        mTap.reset(tap);
	}

    QyStreamInterface* QyStreamTap::DetachTap()
	{ 
        return mTap.release();
	}

    StreamResult QyStreamTap::GetTapResult(int* error)
	{
		if (error) {
            *error = mTapError;
		}
        return mTapResult;
	}

    StreamResult QyStreamTap::read(void* buffer, size_t buffer_len,
        size_t* read, int* error)
	{
        size_t backup_read;
        if (!read) {
            read = &backup_read;
		}
        StreamResult res = QyStreamAdapterInterface::read(buffer, buffer_len,
            read, error);
        if ((res == SR_SUCCESS) && (mTapResult == SR_SUCCESS)) {
            mTapResult = mTap->writeAll(buffer, *read, NULL, &mTapError);
		}
		return res;
	}

    StreamResult QyStreamTap::write(const void* data, size_t data_len,
		size_t* written, int* error) 
	{
		size_t backup_written;
		if (!written) {
			written = &backup_written;
		}
        StreamResult res = QyStreamAdapterInterface::write(data, data_len,
			written, error);
        if ((res == SR_SUCCESS) && (mTapResult == SR_SUCCESS)) {
            mTapResult = mTap->writeAll(data, *written, NULL, &mTapError);
		}
		return res;
	}

	///////////////////////////////////////////////////////////////////////////////
    // QyNullStream
	///////////////////////////////////////////////////////////////////////////////

    QyNullStream::QyNullStream() {
	}

    QyNullStream::~QyNullStream() {
	}

    StreamState QyNullStream::state() const {
        return SS_OPEN;
	}

    StreamResult QyNullStream::read(void* buffer, size_t buffer_len,
        size_t* read, int* error) {
        HUNUSED3(buffer,buffer_len,read);
        if (error) *error = -1;
        return SR_ERROR;
	}


    StreamResult QyNullStream::write(const void* data, size_t data_len,
		size_t* written, int* error) {
        HUNUSED3(data,data_len,error);
        if (written) *written = data_len;
        return SR_SUCCESS;
	}

    void QyNullStream::close() {
	}

    bool QyNullStream::size(size_t* size) const {
		if (size)
			*size = 0;
		return true;
	}

    bool QyNullStream::reserveSize(size_t size) {
        HUNUSED(size);
		return true;
	}

    bool QyNullStream::rewind() {
		return false;
	}

	///////////////////////////////////////////////////////////////////////////////
    // QyFileStream
	///////////////////////////////////////////////////////////////////////////////

    QyFileStream::QyFileStream() : mFile(NULL)
	{
	}

    QyFileStream::~QyFileStream()
	{
        QyFileStream::close();
	}

    bool QyFileStream::open(const std::string& filename, const char* mode)
	{
        close();
#ifdef H_OS_WIN

#else
        mFile = fopen(filename.c_str(), mode);
#endif
        return (mFile != NULL);
	}

    bool QyFileStream::openShare(const std::string& filename, const char* mode,int shflag)
	{
        HUNUSED(shflag);
        close();
#ifdef H_OS_WIN

#else
        return open(filename, mode);
#endif
        return (mFile != NULL);
	}

    bool QyFileStream::disableBuffering()
	{
        if (!mFile)
			return false;
        return (setvbuf(mFile, NULL, _IONBF, 0) == 0);
	}

    StreamState QyFileStream::state() const
	{
        return (mFile == NULL) ? SS_CLOSED : SS_OPEN;
	}

    StreamResult QyFileStream::read(void* buffer, size_t buffer_len,
        size_t* read, int* error)
	{
        if (!mFile)
			return SR_EOS;
        size_t result = fread(buffer, 1, buffer_len, mFile);
		if ((result == 0) && (buffer_len > 0)) {
            if (feof(mFile))
				return SR_EOS;
			if (error)
				*error = errno;
			return SR_ERROR;
		}
        if (read)
            *read = result;
		return SR_SUCCESS;
	}

    StreamResult QyFileStream::write(const void* data, size_t data_len,
		size_t* written, int* error)
	{
        if (!mFile)
			return SR_EOS;
        size_t result = fwrite(data, 1, data_len, mFile);
		if ((result == 0) && (data_len > 0)) {
			if (error)
				*error = errno;
			return SR_ERROR;
		}
		if (written)
			*written = result;
		return SR_SUCCESS;
	}

    void QyFileStream::close()
	{
        if (mFile) {
            fclose(mFile);
            mFile = NULL;
		}
	}

    bool QyFileStream::setPosition(size_t position)
	{
        if (!mFile)
			return false;
        return (fseek(mFile, position, SEEK_SET) == 0);
	}

    bool QyFileStream::getPosition(size_t * position) const
	{
		ASSERT(position != NULL);
        if (!mFile || !position)
			return false;
        long result = ftell(mFile);
		if (result < 0)
			return false;
		*position = result;
		return true;
	}

    bool QyFileStream::size(size_t * size) const
	{
		ASSERT(size != NULL);
        if (!mFile || !size)
			return false;
		struct stat file_stats;
        if (fstat(fileno(mFile), &file_stats) != 0)
			return false;
		*size = file_stats.st_size;
		return true;
	}

    bool QyFileStream::reserveSize(size_t size)
	{
		// TODO: extend the file to the proper length
        HUNUSED(size);
		return true;
	}

    bool QyFileStream::size(const std::string& filename, size_t* size)
	{
		struct stat file_stats;
		if (stat(filename.c_str(), &file_stats) != 0)
			return false;
		*size = file_stats.st_size;
		return true;
	}

    int QyFileStream::flush()
	{
        if (mFile)
		{
            return fflush (mFile);
		}
        // try to flush empty file?
		ASSERT(false);
		return 0;
	}
	///////////////////////////////////////////////////////////////////////////////


    QyMemoryStream::QyMemoryStream()
        : mAllocatedLength(0), mBuffer(NULL), mDataLength(0), mSeekPosition(0)
	{
	}

    QyMemoryStream::QyMemoryStream(const char* data)
        : mAllocatedLength(0), mBuffer(NULL), mDataLength(0), mSeekPosition(0)
	{
        setContents(data, strlen(data));
	}

    QyMemoryStream::QyMemoryStream(const char* data, size_t length)
        : mAllocatedLength(0), mBuffer(NULL), mDataLength(0), mSeekPosition(0)
	{
        setContents(data, length);
	}

    QyMemoryStream::~QyMemoryStream() {
        delete [] mBuffer;
	}

    void QyMemoryStream::setContents(const char* data, size_t length)
	{ 
        delete [] mBuffer;
        mDataLength = mAllocatedLength = length;
        mBuffer = new char[mAllocatedLength];
        memcpy(mBuffer, data, mDataLength);
	}

    StreamState QyMemoryStream::state() const
	{
        return SS_OPEN;
	}

    StreamResult QyMemoryStream::read(void *buffer, size_t bytes,
        size_t *bytes_read, int *error)
	{
        if (mSeekPosition >= mDataLength) {
			// At end of stream
			if (error) {
				*error = EOF;
			}
			return SR_EOS;
		}

        size_t remaining_length = mDataLength - mSeekPosition;
		if (bytes > remaining_length) {
            // read partial buffer
			bytes = remaining_length;
		}
        memcpy(buffer, &mBuffer[mSeekPosition], bytes);
        mSeekPosition += bytes;
        if (bytes_read) {
            *bytes_read = bytes;
		}
		return SR_SUCCESS;
	}

    StreamResult QyMemoryStream::write(const void *buffer,
		size_t bytes, size_t *bytes_written, int *error)
	{
		StreamResult sr = SR_SUCCESS;
		int error_value = 0;
		size_t bytes_written_value = 0;

        size_t new_position = mSeekPosition + bytes;
        if (new_position > mAllocatedLength) {
			// Increase buffer size to the larger of:
			// a) new position rounded up to next 256 bytes
			// b) double the previous length
			size_t new_allocated_length = _max((new_position | 0xFF) + 1,
                mAllocatedLength * 2);
			if (char* new_buffer = new char[new_allocated_length]) {
                memcpy(new_buffer, mBuffer, mDataLength);
                delete [] mBuffer;
                mBuffer = new_buffer;
                mAllocatedLength = new_allocated_length;
			} else {
				error_value = ENOMEM;
				sr = SR_ERROR;
			}
		}

		if (sr == SR_SUCCESS) {
			bytes_written_value = bytes;
            memcpy(&mBuffer[mSeekPosition], buffer, bytes);
            mSeekPosition = new_position;
            if (mDataLength < mSeekPosition) {
                mDataLength = mSeekPosition;
			}
		}

		if (bytes_written) {
			*bytes_written = bytes_written_value;
		}
		if (error) {
			*error = error_value;
		}

		return sr;
	}

    void QyMemoryStream::close()
	{
		// nothing to do
	}

    bool QyMemoryStream::setPosition(size_t position)
	{
        if (position <= mDataLength) {
            mSeekPosition = position;
			return true;
		}
		return false;
	}

    bool QyMemoryStream::getPosition(size_t *position) const
	{
		if (!position) {
			return false;
		}
        *position = mSeekPosition;
		return true;
	}

    bool QyMemoryStream::size(size_t *size) const
	{
		if (!size) {
			return false;
		}
        *size = mDataLength;
		return true;
	}

    bool QyMemoryStream::reserveSize(size_t size)
	{
        if (mAllocatedLength >= size)
			return true;

		if (char* new_buffer = new char[size]) {
            memcpy(new_buffer, mBuffer, mDataLength);
            delete [] mBuffer;
            mBuffer = new_buffer;
            mAllocatedLength = size;
			return true;
		}

		return false;
	}

	///////////////////////////////////////////////////////////////////////////////

    StreamResult flowStream(QyStreamInterface* source,
		char* buffer, size_t buffer_len,
        QyStreamInterface* sink)
	{
		ASSERT(buffer_len > 0);

		StreamResult result;
        size_t count, read_pos, write_pos;

		bool end_of_stream = false;
		do {
            // read until buffer is full, end of stream, or error
            read_pos = 0;
			do {
                result = source->read(buffer + read_pos, buffer_len - read_pos,
					&count, NULL);
				if (result == SR_EOS) {
					end_of_stream = true;
				} else if (result != SR_SUCCESS) {
					return result;
				} else {
                    read_pos += count;
				}
            } while (!end_of_stream && (read_pos < buffer_len));

            // write until buffer is empty, or error (including end of stream)
            write_pos = 0;
			do {
                result = sink->write(buffer + write_pos, read_pos - write_pos,
					&count, NULL);
				if (result != SR_SUCCESS)
					return result;

                write_pos += count;
            } while (write_pos < read_pos);
		} while (!end_of_stream);

		return SR_SUCCESS;
	}

	///////////////////////////////////////////////////////////////////////////////
    // QyStringStream - reads/writes to an external std::string
	///////////////////////////////////////////////////////////////////////////////

    QyStringStream::QyStringStream(std::string& str)
        : mString(str), mReadPos(0), mReadOnly(false)
	{
	}

    QyStringStream::QyStringStream(const std::string& str)
        : mString(const_cast<std::string&>(str)), mReadPos(0), mReadOnly(true)
	{
	}

    StreamState QyStringStream::state() const
	{
        return SS_OPEN;
	}

    StreamResult QyStringStream::read(void* buffer, size_t buffer_len,
        size_t* read, int* error)
	{
        HUNUSED(error);
        size_t available = _min(buffer_len, mString.size() - mReadPos);
		if (!available)
			return SR_EOS;
        memcpy(buffer, mString.data() + mReadPos, available);
        mReadPos += available;
        if (read)
            *read = available;
		return SR_SUCCESS;
	}

    StreamResult QyStringStream::write(const void* data, size_t data_len,
		size_t* written, int* error)
	{
        if (mReadOnly) {
			if (error) {
				*error = -1;
			}
			return SR_ERROR;
		}
        mString.append(static_cast<const char*>(data),
			static_cast<const char*>(data) + data_len);
		if (written)
			*written = data_len;
		return SR_SUCCESS;
	}

    void QyStringStream::close()
	{
	}

    bool QyStringStream::size(size_t* size) const
	{
		ASSERT(size != NULL);
        *size = mString.size();
		return true;
	}

    bool QyStringStream::reserveSize(size_t size)
	{
        if (mReadOnly)
			return false;
        mString.reserve(size);
		return true;
	}

    bool QyStringStream::rewind()
	{
        mReadPos = 0;
		return true;
	}
} // namespace qy
