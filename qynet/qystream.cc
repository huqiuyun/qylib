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
        : QyStreamAdapterInterface(stream), tap_(NULL), tap_result_(SR_SUCCESS),
		tap_error_(0)
	{
		AttachTap(tap);
	}

    void QyStreamTap::AttachTap(QyStreamInterface* tap)
	{
		tap_.reset(tap);
	}

    QyStreamInterface* QyStreamTap::DetachTap()
	{ 
		return tap_.release();
	}

    StreamResult QyStreamTap::GetTapResult(int* error)
	{
		if (error) {
			*error = tap_error_;
		}
		return tap_result_;
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
		if ((res == SR_SUCCESS) && (tap_result_ == SR_SUCCESS)) {
            tap_result_ = tap_->writeAll(buffer, *read, NULL, &tap_error_);
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
		if ((res == SR_SUCCESS) && (tap_result_ == SR_SUCCESS)) {
            tap_result_ = tap_->writeAll(data, *written, NULL, &tap_error_);
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

    QyFileStream::QyFileStream() : file_(NULL)
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
        file_ = fopen(filename.c_str(), mode);
#endif
		return (file_ != NULL);
	}

    bool QyFileStream::openShare(const std::string& filename, const char* mode,int shflag)
	{
        HUNUSED(shflag);
        close();
#ifdef H_OS_WIN

#else
        return open(filename, mode);
#endif
		return (file_ != NULL);
	}

    bool QyFileStream::disableBuffering()
	{
		if (!file_)
			return false;
		return (setvbuf(file_, NULL, _IONBF, 0) == 0);
	}

    StreamState QyFileStream::state() const
	{
        return (file_ == NULL) ? SS_CLOSED : SS_OPEN;
	}

    StreamResult QyFileStream::read(void* buffer, size_t buffer_len,
        size_t* read, int* error)
	{
		if (!file_)
			return SR_EOS;
        size_t result = fread(buffer, 1, buffer_len, file_);
		if ((result == 0) && (buffer_len > 0)) {
			if (feof(file_))
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
		if (!file_)
			return SR_EOS;
        size_t result = fwrite(data, 1, data_len, file_);
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
		if (file_) {
            fclose(file_);
			file_ = NULL;
		}
	}

    bool QyFileStream::setPosition(size_t position)
	{
		if (!file_)
			return false;
		return (fseek(file_, position, SEEK_SET) == 0);
	}

    bool QyFileStream::getPosition(size_t * position) const
	{
		ASSERT(position != NULL);
		if (!file_ || !position)
			return false;
		long result = ftell(file_);
		if (result < 0)
			return false;
		*position = result;
		return true;
	}

    bool QyFileStream::size(size_t * size) const
	{
		ASSERT(size != NULL);
		if (!file_ || !size)
			return false;
		struct stat file_stats;
		if (fstat(fileno(file_), &file_stats) != 0)
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
		if (file_)
		{
            return fflush (file_);
		}
        // try to flush empty file?
		ASSERT(false);
		return 0;
	}
	///////////////////////////////////////////////////////////////////////////////


    QyMemoryStream::QyMemoryStream()
		: allocated_length_(0), buffer_(NULL), data_length_(0), seek_position_(0)
	{
	}

    QyMemoryStream::QyMemoryStream(const char* data)
		: allocated_length_(0), buffer_(NULL), data_length_(0), seek_position_(0)
	{
        setContents(data, strlen(data));
	}

    QyMemoryStream::QyMemoryStream(const char* data, size_t length)
		: allocated_length_(0), buffer_(NULL), data_length_(0), seek_position_(0)
	{
        setContents(data, length);
	}

    QyMemoryStream::~QyMemoryStream() {
		delete [] buffer_;
	}

    void QyMemoryStream::setContents(const char* data, size_t length)
	{ 
		delete [] buffer_;
		data_length_ = allocated_length_ = length;
		buffer_ = new char[allocated_length_];
		memcpy(buffer_, data, data_length_);
	}

    StreamState QyMemoryStream::state() const
	{
        return SS_OPEN;
	}

    StreamResult QyMemoryStream::read(void *buffer, size_t bytes,
        size_t *bytes_read, int *error)
	{
		if (seek_position_ >= data_length_) {
			// At end of stream
			if (error) {
				*error = EOF;
			}
			return SR_EOS;
		}

		size_t remaining_length = data_length_ - seek_position_;
		if (bytes > remaining_length) {
            // read partial buffer
			bytes = remaining_length;
		}
		memcpy(buffer, &buffer_[seek_position_], bytes);
		seek_position_ += bytes;
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

		size_t new_position = seek_position_ + bytes;
		if (new_position > allocated_length_) {
			// Increase buffer size to the larger of:
			// a) new position rounded up to next 256 bytes
			// b) double the previous length
			size_t new_allocated_length = _max((new_position | 0xFF) + 1,
				allocated_length_ * 2);
			if (char* new_buffer = new char[new_allocated_length]) {
				memcpy(new_buffer, buffer_, data_length_);
				delete [] buffer_;
				buffer_ = new_buffer;
				allocated_length_ = new_allocated_length;
			} else {
				error_value = ENOMEM;
				sr = SR_ERROR;
			}
		}

		if (sr == SR_SUCCESS) {
			bytes_written_value = bytes;
			memcpy(&buffer_[seek_position_], buffer, bytes);
			seek_position_ = new_position;
			if (data_length_ < seek_position_) {
				data_length_ = seek_position_;
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
		if (position <= data_length_) {
			seek_position_ = position;
			return true;
		}
		return false;
	}

    bool QyMemoryStream::getPosition(size_t *position) const
	{
		if (!position) {
			return false;
		}
		*position = seek_position_;
		return true;
	}

    bool QyMemoryStream::size(size_t *size) const
	{
		if (!size) {
			return false;
		}
		*size = data_length_;
		return true;
	}

    bool QyMemoryStream::reserveSize(size_t size)
	{
		if (allocated_length_ >= size)
			return true;

		if (char* new_buffer = new char[size]) {
			memcpy(new_buffer, buffer_, data_length_);
			delete [] buffer_;
			buffer_ = new_buffer;
			allocated_length_ = size;
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
        : str_(str), read_pos_(0), read_only_(false)
	{
	}

    QyStringStream::QyStringStream(const std::string& str)
        : str_(const_cast<std::string&>(str)), read_pos_(0), read_only_(true)
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
        size_t available = _min(buffer_len, str_.size() - read_pos_);
		if (!available)
			return SR_EOS;
        memcpy(buffer, str_.data() + read_pos_, available);
        read_pos_ += available;
        if (read)
            *read = available;
		return SR_SUCCESS;
	}

    StreamResult QyStringStream::write(const void* data, size_t data_len,
		size_t* written, int* error)
	{
        if (read_only_) {
			if (error) {
				*error = -1;
			}
			return SR_ERROR;
		}
		str_.append(static_cast<const char*>(data),
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
		*size = str_.size();
		return true;
	}

    bool QyStringStream::reserveSize(size_t size)
	{
        if (read_only_)
			return false;
		str_.reserve(size);
		return true;
	}

    bool QyStringStream::rewind()
	{
        read_pos_ = 0;
		return true;
	}
} // namespace qy
