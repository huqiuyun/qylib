#ifndef QY_NETSTREAM_H__
#define QY_NETSTREAM_H__

#include "qyscoped_ptr.h"
#include "qysigslot.h"
#include "qybasictypes.h"

namespace qy 
{

	///////////////////////////////////////////////////////////////////////////////
    // QyStreamInterface is a generic asynchronous stream interface, supporting read,
    // write, and close operations, and asynchronous signalling of state changes.
	// The interface is designed with file, memory, and socket implementations in
	// mind.
	///////////////////////////////////////////////////////////////////////////////

    // The following enumerations are declared outside of the QyStreamInterface
	// class for brevity in use.

    // The SS_OPENING state indicates that the stream will signal open or closed
	// in the future.
    enum StreamState { SS_CLOSED, SS_OPENING, SS_OPEN };

    // Stream read/write methods return this value to indicate various success
	// and failure conditions described below.
	enum StreamResult { SR_ERROR, SR_SUCCESS, SR_BLOCK, SR_EOS };

	// StreamEvents are used to asynchronously signal state transitionss.  The flags
	// may be combined.
    //  SE_OPEN: The stream has transitioned to the SS_OPEN state
    //  SE_CLOSE: The stream has transitioned to the SS_CLOSED state
    //  SE_READ: Data is available, so read is likely to not return SR_BLOCK
    //  SE_WRITE: Data can be written, so write is likely to not return SR_BLOCK
    enum StreamEvent { SE_OPEN = 1, SE_READ = 2, SE_WRITE = 4, SE_CLOSE = 8 };

    class QyStreamInterface
	{
	public:
        virtual ~QyStreamInterface() { }

        virtual StreamState state() const = 0;

        // read attempts to fill buffer of size buffer_len.  write attempts to send
        // data_len bytes stored in data.  The variables read and write are set only
		// on SR_SUCCESS (see below).  Likewise, error is only set on SR_ERROR.
        // read and write return a value indicating:
		//  SR_ERROR: an error occurred, which is returned in a non-null error
		//    argument.  Interpretation of the error requires knowledge of the
		//    stream's concrete type, which limits its usefulness.
		//  SR_SUCCESS: some number of bytes were successfully written, which is
        //    returned in a non-null read/write argument.
		//  SR_BLOCK: the stream is in non-blocking mode, and the operation would
        //    block, or the stream is in SS_OPENING state.
		//  SR_EOS: the end-of-stream has been reached, or the stream is in the
        //    SS_CLOSED state.
        virtual StreamResult read(void* buffer, size_t buffer_len,size_t* read, int* error) = 0;
        virtual StreamResult write(const void* data, size_t data_len,size_t* written, int* error) = 0;

        // Attempt to transition to the SS_CLOSED state.  SE_CLOSE will not be
		// signalled as a result of this call.
        virtual void close() = 0;

        // Return the number of bytes that will be returned by read, if known.
        virtual bool size(size_t* size) const = 0;

		// Communicates the amount of data which will be written to the stream.  The
		// stream may choose to preallocate memory to accomodate this data.  The
		// stream may return false to indicate that there is not enough room (ie, 
        // write will return SR_EOS/SR_ERROR at some point).  Note that calling this
		// function should not affect the existing state of data in the stream.
        virtual bool reserveSize(size_t size) = 0;

		// Returns true if stream could be repositioned to the beginning.
        virtual bool rewind() = 0;

        // writeAll is a helper function which repeatedly calls write until all the
		// data is written, or something other than SR_SUCCESS is returned.  Note that
        // unlike write, the argument 'written' is always set, and may be non-zero
		// on results other than SR_SUCCESS.  The remaining arguments have the
        // same semantics as write.
        StreamResult writeAll(const void* data, size_t data_len,size_t* written, int* error);

        // Similar to readAll.  Calls read until buffer_len bytes have been read, or
        // until a non-SR_SUCCESS result is returned.  'read' is always set.
        StreamResult readAll(void* buffer, size_t buffer_len,size_t* read, int* error);

        // readLine is a helper function which repeatedly calls read until it hits
		// the end-of-line character, or something other than SR_SUCCESS.
		// TODO: this is too inefficient to keep here.  Break this out into a buffered
        // readline object or adapter
        StreamResult readLine(std::string *line);

		// Streams may signal one or more StreamEvents to indicate state changes.
		// The first argument identifies the stream on which the state change occured.
		// The second argument is a bit-wise combination of StreamEvents.
        // If SE_CLOSE is signalled, then the third argument is the associated error
		// code.  Otherwise, the value is undefined.
		// Note: Not all streams will support asynchronous event signalling.  However,
        // SS_OPENING and SR_BLOCK returned from stream member functions imply that
		// certain events will be raised in the future.
        sigslot::signal3<QyStreamInterface*, int, int> sigEvent;

	protected:
        QyStreamInterface() { }

	private:
        DISALLOW_EVIL_CONSTRUCTORS(QyStreamInterface);
	};

	///////////////////////////////////////////////////////////////////////////////
    // QyStreamAdapterInterface is a convenient base-class for adapting a stream.
	// By default, all operations are pass-through.  Override the methods that you
	// require adaptation.  Note that the adapter will delete the adapted stream.
	///////////////////////////////////////////////////////////////////////////////

    class QyStreamAdapterInterface : public QyStreamInterface,public sigslot::has_slots<>
	{
	public:
        explicit QyStreamAdapterInterface(QyStreamInterface* stream)
		{
            attach(stream);
		}

        virtual StreamState state() const {
            return stream_->state();
		}
        virtual StreamResult read(void* buffer, size_t buffer_len,size_t* read, int* error)
		{
            return stream_->read(buffer, buffer_len, read, error);
		}
        virtual StreamResult write(const void* data, size_t data_len,size_t* written, int* error)
		{
            return stream_->write(data, data_len, written, error);
		}
        virtual void close()
		{
            stream_->close();
		}
        virtual bool size(size_t* size) const
		{
            return stream_->size(size);
		}
        virtual bool reserveSize(size_t size)
		{
            return stream_->reserveSize(size);
		}
        virtual bool rewind()
		{
            return stream_->rewind();
		}

        void attach(QyStreamInterface* stream)
		{
			if (NULL != stream_.get())
                stream_->sigEvent.disconnect(this);
			stream_.reset(stream);
			if (NULL != stream_.get())
                stream_->sigEvent.connect(this, &QyStreamAdapterInterface::onEvent);
		}
        QyStreamInterface* detach()
		{ 
			if (NULL == stream_.get())
				return NULL;
            stream_->sigEvent.disconnect(this);
			return stream_.release();
		}

	protected:
		// Note that the adapter presents itself as the origin of the stream events,
		// since users of the adapter may not recognize the adapted object.
        virtual void onEvent(QyStreamInterface* stream, int events, int err)
		{
            HUNUSED(stream);
            sigEvent(this, events, err);
		}

	private:
        scoped_ptr<QyStreamInterface> stream_;
        DISALLOW_EVIL_CONSTRUCTORS(QyStreamAdapterInterface);
	};

	///////////////////////////////////////////////////////////////////////////////
    // QyStreamTap is a non-modifying, pass-through adapter, which copies all data
	// in either direction to the tap.  Note that errors or blocking on writing to
    // the tap will prevent further tap writes from occurring.
	///////////////////////////////////////////////////////////////////////////////

    class QyStreamTap : public QyStreamAdapterInterface
	{
	public:
        explicit QyStreamTap(QyStreamInterface* stream, QyStreamInterface* tap);

        void AttachTap(QyStreamInterface* tap);
        QyStreamInterface* DetachTap();
		StreamResult GetTapResult(int* error);

        // QyStreamAdapterInterface Interface
        virtual StreamResult read(void* buffer, size_t buffer_len,
            size_t* read, int* error);
        virtual StreamResult write(const void* data, size_t data_len,
			size_t* written, int* error);

	private:
        scoped_ptr<QyStreamInterface> tap_;
		StreamResult tap_result_;
		int tap_error_;
        DISALLOW_EVIL_CONSTRUCTORS(QyStreamTap);
	};

	///////////////////////////////////////////////////////////////////////////////
    // QyNullStream gives errors on read, and silently discards all written data.
	///////////////////////////////////////////////////////////////////////////////

    class QyNullStream : public QyStreamInterface
	{
	public:
        QyNullStream();
        virtual ~QyNullStream();

        // QyStreamInterface Interface
        virtual StreamState state() const;
        virtual StreamResult read(void* buffer, size_t buffer_len,
            size_t* read, int* error);
        virtual StreamResult write(const void* data, size_t data_len,
			size_t* written, int* error);
        virtual void close();
        virtual bool size(size_t* size) const;
        virtual bool reserveSize(size_t size);
        virtual bool rewind();
	};

	///////////////////////////////////////////////////////////////////////////////
    // QyFileStream is a simple implementation of a QyStreamInterface, which does not
	// support asynchronous notification.
	///////////////////////////////////////////////////////////////////////////////

    class QyFileStream : public QyStreamInterface
	{
	public:
        QyFileStream();
        virtual ~QyFileStream();

        // The semantics of filename and mode are the same as stdio's fopen
        virtual bool open(const std::string& filename, const char* mode);
        virtual bool openShare(const std::string& filename, const char* mode,int shflag);

        // By default, reads and writes are buffered for efficiency.  Disabling
        // buffering causes writes to block until the bytes on disk are updated.
        virtual bool disableBuffering();

        virtual StreamState state() const;
        virtual StreamResult read(void* buffer, size_t buffer_len,size_t* read, int* error);
        virtual StreamResult write(const void* data, size_t data_len,size_t* written, int* error);
        virtual void close();
        virtual bool size(size_t* size) const;
        virtual bool reserveSize(size_t size);
        virtual bool rewind() { return setPosition(0); }

        bool setPosition(size_t position);
        bool getPosition(size_t* position) const;
        int flush();
        static bool size(const std::string& filename, size_t* size);

	private:
		FILE* file_;
        DISALLOW_EVIL_CONSTRUCTORS(QyFileStream);
	};

	///////////////////////////////////////////////////////////////////////////////
    // QyMemoryStream is a simple implementation of a QyStreamInterface over in-memory
	// data.  It does not support asynchronous notification.
	///////////////////////////////////////////////////////////////////////////////

    class QyMemoryStream : public QyStreamInterface
	{
	public:
        QyMemoryStream();
		// Pre-populate stream with the provided data.
        QyMemoryStream(const char* data);
        QyMemoryStream(const char* data, size_t length);
        virtual ~QyMemoryStream();

        virtual StreamState state() const;
        virtual StreamResult read(void *buffer, size_t bytes, size_t *bytes_read, int *error);
        virtual StreamResult write(const void *buffer, size_t bytes, size_t *bytes_written, int *error);
        virtual void close();
        virtual bool size(size_t* size) const;
        virtual bool reserveSize(size_t size);
        virtual bool rewind() { return setPosition(0); }

        char* getBuffer() { return buffer_; }
        const char* getBuffer() const { return buffer_; }
        bool setPosition(size_t position);
        bool getPosition(size_t* position) const;

	private:
        void setContents(const char* data, size_t length);

		size_t   allocated_length_;
		char*    buffer_;
		size_t   data_length_;
		size_t   seek_position_;

	private:
        DISALLOW_EVIL_CONSTRUCTORS(QyMemoryStream);
	};

	///////////////////////////////////////////////////////////////////////////////
    // QyStringStream - reads/writes to an external std::string
	///////////////////////////////////////////////////////////////////////////////

    class QyStringStream : public QyStreamInterface
	{
	public:
        QyStringStream(std::string& str);
        QyStringStream(const std::string& str);

        virtual StreamState state() const;
        virtual StreamResult read(void* buffer, size_t buffer_len,
            size_t* read, int* error);
        virtual StreamResult write(const void* data, size_t data_len,
			size_t* written, int* error);
        virtual void close();
        virtual bool size(size_t* size) const;
        virtual bool reserveSize(size_t size);
        virtual bool rewind();

	private:
		std::string& str_;
        size_t read_pos_;
        bool read_only_;
	};

	///////////////////////////////////////////////////////////////////////////////

	// Flow attempts to move bytes from source to sink via buffer of size
	// buffer_len.  The function returns SR_SUCCESS when source reaches
	// end-of-stream (returns SR_EOS), and all the data has been written successful
	// to sink.  Alternately, if source returns SR_BLOCK or SR_ERROR, or if sink
	// returns SR_BLOCK, SR_ERROR, or SR_EOS, then the function immediately returns
	// with the unexpected StreamResult value.

    StreamResult flowStream(QyStreamInterface* source,char* buffer, size_t buffer_len,QyStreamInterface* sink);

	///////////////////////////////////////////////////////////////////////////////

} // namespace qy

#endif  // QY_NETSTREAM_H__
