#ifndef __STREAMUTILS_H__
#define __STREAMUTILS_H__

#include "qysigslot.h"
#include "qystream.h"

namespace qy {
///////////////////////////////////////////////////////////////////////////////
// QyStreamRelay - acts as an intermediary between two asynchronous streams,
//  reading from one stream and writing to the other, using a pre-specified
//  amount of buffering in both directions.
///////////////////////////////////////////////////////////////////////////////

class QyStreamRelay : public sigslot::has_slots<>
{
public:
    QyStreamRelay(QyStreamInterface* s1,QyStreamInterface* s2, size_t buffer_size);
    virtual ~QyStreamRelay();

    void circulate(); // Simulate events to get things flowing
    void close();

    sigslot::signal2<QyStreamRelay*, int> sigClosed;

private:
    inline int indexOf(QyStreamInterface* s) const {
        return (s == dir_[1].stream);
    }

    inline int complement(int index) const {
        return (1-index);
    }

    bool flow(int read_index, int* error);
    void onEvent(QyStreamInterface* stream, int events, int error);

	struct Direction {
        QyStreamInterface* stream;
		char* buffer;
		size_t data_len;
	};
	Direction dir_[2];
	size_t buffer_size_;
};

///////////////////////////////////////////////////////////////////////////////
// QyStreamCounter - counts the number of bytes which are transferred in either
//  direction.
///////////////////////////////////////////////////////////////////////////////

class QyStreamCounter : public QyStreamAdapterInterface
{
public:
    explicit QyStreamCounter(QyStreamInterface* stream);

    inline void resetByteCount() { count_ = 0; }
    inline size_t byteCount() const { return count_; }

    sigslot::signal1<size_t> sigUpdateByteCount;

    // QyStreamAdapterInterface
    virtual StreamResult read(void* buffer, size_t buffer_len,size_t* read, int* error);

    virtual StreamResult write(const void* data, size_t data_len,size_t* written, int* error);

private:
	size_t count_;
};

} // namespace qy

#endif  // __STREAMUTILS_H__
