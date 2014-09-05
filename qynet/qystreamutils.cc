#include "qystreamutils.h"
#include "qydebug.h"

namespace qy {
///////////////////////////////////////////////////////////////////////////////
// TODO: Extend so that one side can close, and other side can send
// buffered data.

QyStreamRelay::QyStreamRelay(QyStreamInterface* s1,
                         QyStreamInterface* s2,
                         size_t buffer_size) : mBuffSize(buffer_size)
{
    mDir[0].stream = s1;
    mDir[1].stream = s2;

    ASSERT(s1->state() != SS_CLOSED);
    ASSERT(s2->state() != SS_CLOSED);

	for (size_t i=0; i<2; ++i) {
        mDir[i].stream->sigEvent.connect(this, &QyStreamRelay::onEvent);
        mDir[i].buffer = new char[mBuffSize];
        mDir[i].data_len = 0;
	}
}

QyStreamRelay::~QyStreamRelay()
{
	for (size_t i=0; i<2; ++i) {
        delete mDir[i].stream;
        delete [] mDir[i].buffer;
	}
}

void
QyStreamRelay::circulate()
{
	int error = 0;
    if (!flow(0, &error) || !flow(1, &error)) {
        close();
        sigClosed(this, error);
	}
}

void
QyStreamRelay::close() {
	for (size_t i=0; i<2; ++i) {
        mDir[i].stream->sigEvent.disconnect(this);
        mDir[i].stream->close();
	}
}

bool
QyStreamRelay::flow(int read_index, int* error)
{
    Direction& reader = mDir[read_index];
    Direction& writer = mDir[complement(read_index)];

	bool progress;
	do {
		progress = false;

        while (reader.stream->state() == SS_OPEN) {
            size_t available = mBuffSize - reader.data_len;
			if (available == 0)
				break;

			*error = 0;
			size_t read = 0;
            StreamResult result
                = reader.stream->read(reader.buffer + reader.data_len, available,
				&read, error);
            if ((result == SR_BLOCK) || (result == SR_EOS))
				break;

            if (result == SR_ERROR)
				return false;

			progress = true;
			ASSERT((read > 0) && (read <= available));
			reader.data_len += read;
		}

		size_t total_written = 0;
        while (writer.stream->state() == SS_OPEN) {
			size_t available = reader.data_len - total_written;
			if (available == 0)
				break;

			*error = 0;
			size_t written = 0;
            StreamResult result
                = writer.stream->write(reader.buffer + total_written,
				available, &written, error);
            if ((result == SR_BLOCK) || (result == SR_EOS))
				break;

            if (result == SR_ERROR)
				return false;

			progress = true;
			ASSERT((written > 0) && (written <= available));
			total_written += written;
		}

		reader.data_len -= total_written;
		if (reader.data_len > 0) {
			memmove(reader.buffer, reader.buffer + total_written, reader.data_len);
		}
	} while (progress);

	return true;
}

void QyStreamRelay::onEvent(QyStreamInterface* stream, int events,
						  int error)
{
    int index = indexOf(stream);

	// Note: In the following cases, we are treating the open event as both
	// readable and writeable, for robustness.  It won't hurt if we are wrong.

    if ((events & (SE_OPEN | SE_READ))
        && !flow(index, &error)) {
            events = SE_CLOSE;
	}

    if ((events & (SE_OPEN | SE_WRITE))
        && !flow(complement(index), &error)) {
            events = SE_CLOSE;
	}

    if (events & SE_CLOSE) {
        close();
        sigClosed(this, error);
	}
}

///////////////////////////////////////////////////////////////////////////////
// QyStreamCounter - counts the number of bytes which are transferred in either
//  direction.
///////////////////////////////////////////////////////////////////////////////

QyStreamCounter::QyStreamCounter(QyStreamInterface* stream)
: QyStreamAdapterInterface(stream), mCount(0)
{
}

StreamResult QyStreamCounter::read(void* buffer, size_t buffer_len,
										   size_t* read, int* error)
{
	size_t tmp;
	if (!read)
		read = &tmp;
    StreamResult result
        = QyStreamAdapterInterface::read(buffer, buffer_len,
		read, error);
    if (result == SR_SUCCESS)
        mCount += *read;
    sigUpdateByteCount(mCount);
	return result;
}

StreamResult QyStreamCounter::write(const void* data, size_t data_len,
											size_t* written, int* error)
{
	size_t tmp;
	if (!written)
		written = &tmp;
    StreamResult result
        = QyStreamAdapterInterface::write(data, data_len, written, error);
    if (result == SR_SUCCESS)
        mCount += *written;
    sigUpdateByteCount(mCount);
	return result;
}
} // namesapce qy
