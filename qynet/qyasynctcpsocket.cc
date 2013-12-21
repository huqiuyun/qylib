#include "qyasynctcpsocket.h"
#include "qybyteorder.h"
#include "qydebug.h"

namespace qy
{

	const size_t MAX_PACKET_SIZE = 64 * 1024;

	typedef uint16 PacketLength;
	const size_t PKT_LEN_SIZE = sizeof(PacketLength);

	const size_t BUF_SIZE = MAX_PACKET_SIZE + PKT_LEN_SIZE;

    AsyncTCPSocket::AsyncTCPSocket(QyAsyncSocket* socket)
        : QyAsyncPacketSocket(socket)
		, insize_(BUF_SIZE)
		, inpos_(0)
		, outsize_(BUF_SIZE)
		, outpos_(0)
	{
		inbuf_ = new char[insize_];
		outbuf_ = new char[outsize_];

		ASSERT(socket_ != NULL);
        socket_->sigConnectEvent.connect(this, &AsyncTCPSocket::onConnectEvent);
        socket_->sigReadEvent.connect(this, &AsyncTCPSocket::onReadEvent);
        socket_->sigWriteEvent.connect(this, &AsyncTCPSocket::onWriteEvent);
        socket_->sigCloseEvent.connect(this, &AsyncTCPSocket::onCloseEvent);
	}

	AsyncTCPSocket::~AsyncTCPSocket()
	{
		delete [] inbuf_;
		delete [] outbuf_;
	}

    int AsyncTCPSocket::send(const void *pv, size_t cb)
	{
		if (cb > MAX_PACKET_SIZE) {
            socket_->setError(EMSGSIZE);
			return -1;
		}

		// If we are blocking on send, then silently drop this packet
		if (outpos_)
			return static_cast<int>(cb);

		PacketLength pkt_len = HostToNetwork16(static_cast<PacketLength>(cb));
		memcpy(outbuf_, &pkt_len, PKT_LEN_SIZE);
		memcpy(outbuf_ + PKT_LEN_SIZE, pv, cb);
		outpos_ = PKT_LEN_SIZE + cb;

        int res = flush();
		if (res <= 0) {
			// drop packet if we made no progress
			outpos_ = 0; 
			return res;
		}

		// We claim to have sent the whole thing, even if we only sent partial
		return static_cast<int>(cb);
	}

    int AsyncTCPSocket::sendTo(const void *pv, size_t cb, const QySocketAddress& addr)
	{
        if (addr == remoteAddress())
            return send(pv, cb);

		ASSERT(false);
        socket_->setError(ENOTCONN);
		return -1;
	}

    int AsyncTCPSocket::sendRaw(const void * pv, size_t cb)
	{
		if (outpos_ + cb > outsize_) {
            socket_->setError(EMSGSIZE);
			return -1;
		}

		memcpy(outbuf_ + outpos_, pv, cb);
		outpos_ += cb;

        return flush();
	}

    void AsyncTCPSocket::processInput(char * data, size_t& len)
	{
        QySocketAddress remote_addr(remoteAddress());

		while (true) {
			if (len < PKT_LEN_SIZE)
				return;

			PacketLength pkt_len;
			memcpy(&pkt_len, data, PKT_LEN_SIZE);
			pkt_len = NetworkToHost16(pkt_len);

			if (len < PKT_LEN_SIZE + pkt_len)
				return;

            sigReadPacket(data + PKT_LEN_SIZE, pkt_len, remote_addr, this);

			len -= PKT_LEN_SIZE + pkt_len;
			if (len > 0) {
				memmove(data, data + PKT_LEN_SIZE + pkt_len, len);
			}
		}
	}

    int AsyncTCPSocket::flush()
	{
        int res = socket_->send(outbuf_, outpos_);
		if (res <= 0) {
			return res;
		}
		if (static_cast<size_t>(res) <= outpos_) {
			outpos_ -= res;
		} else {
			ASSERT(false);
			return -1;
		}
		if (outpos_ > 0) {
			memmove(outbuf_, outbuf_ + res, outpos_);
		}
		return res;
	}

    void AsyncTCPSocket::onConnectEvent(QyAsyncSocket* socket)
	{
        HUNUSED(socket);
        sigConnect(this);
	}

    void AsyncTCPSocket::onReadEvent(QyAsyncSocket* socket)
	{
        HUNUSED(socket);
		ASSERT(socket == socket_);

        int len = socket_->recv(inbuf_ + inpos_, insize_ - inpos_);
		if (len < 0) {
			// TODO: Do something better like forwarding the error to the user.
            if (!socket_->isBlocking()) {
			}
			return;
		}

		inpos_ += len;

        processInput(inbuf_, inpos_);

		if (inpos_ >= insize_) {
			ASSERT(false);
			inpos_ = 0;
		}
	}

    void AsyncTCPSocket::onWriteEvent(QyAsyncSocket* socket)
	{
        HUNUSED(socket);
		ASSERT(socket == socket_);

		if (outpos_ > 0) {
            flush();
		}
	}

    void AsyncTCPSocket::onCloseEvent(QyAsyncSocket* socket, int error)
	{
        HUNUSED(socket);
        sigClose(this, error);
	}

} // namespace qy
