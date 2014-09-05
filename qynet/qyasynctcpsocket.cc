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
        , mInSize(BUF_SIZE)
        , mInpos(0)
        , mOutSize(BUF_SIZE)
        , mOutpos(0)
	{
        mInBuf = new char[mInSize];
        mOutBuf = new char[mOutSize];

        ASSERT(mSocket != NULL);
        mSocket->sigConnectEvent.connect(this, &AsyncTCPSocket::onConnectEvent);
        mSocket->sigReadEvent.connect(this, &AsyncTCPSocket::onReadEvent);
        mSocket->sigWriteEvent.connect(this, &AsyncTCPSocket::onWriteEvent);
        mSocket->sigCloseEvent.connect(this, &AsyncTCPSocket::onCloseEvent);
	}

	AsyncTCPSocket::~AsyncTCPSocket()
	{
        delete [] mInBuf;
        delete [] mOutBuf;
	}

    int AsyncTCPSocket::send(const void *pv, size_t cb)
	{
		if (cb > MAX_PACKET_SIZE) {
            mSocket->setError(EMSGSIZE);
			return -1;
		}

		// If we are blocking on send, then silently drop this packet
        if (mOutpos)
			return static_cast<int>(cb);

		PacketLength pkt_len = HostToNetwork16(static_cast<PacketLength>(cb));
        memcpy(mOutBuf, &pkt_len, PKT_LEN_SIZE);
        memcpy(mOutBuf + PKT_LEN_SIZE, pv, cb);
        mOutpos = PKT_LEN_SIZE + cb;

        int res = flush();
		if (res <= 0) {
			// drop packet if we made no progress
            mOutpos = 0;
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
        mSocket->setError(ENOTCONN);
		return -1;
	}

    int AsyncTCPSocket::sendRaw(const void * pv, size_t cb)
	{
        if (mOutpos + cb > mOutSize) {
            mSocket->setError(EMSGSIZE);
			return -1;
		}

        memcpy(mOutBuf + mOutpos, pv, cb);
        mOutpos += cb;

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
        int res = mSocket->send(mOutBuf, mOutpos);
		if (res <= 0) {
			return res;
		}
        if (static_cast<size_t>(res) <= mOutpos) {
            mOutpos -= res;
		} else {
			ASSERT(false);
			return -1;
		}
        if (mOutpos > 0) {
            memmove(mOutBuf, mOutBuf + res, mOutpos);
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
        ASSERT(socket == mSocket);

        int len = mSocket->recv(mInBuf + mInpos, mInSize - mInpos);
		if (len < 0) {
			// TODO: Do something better like forwarding the error to the user.
            if (!mSocket->isBlocking()) {
			}
			return;
		}

        mInpos += len;

        processInput(mInBuf, mInpos);

        if (mInpos >= mInSize) {
			ASSERT(false);
            mInpos = 0;
		}
	}

    void AsyncTCPSocket::onWriteEvent(QyAsyncSocket* socket)
	{
        HUNUSED(socket);
        ASSERT(socket == mSocket);

        if (mOutpos > 0) {
            flush();
		}
	}

    void AsyncTCPSocket::onCloseEvent(QyAsyncSocket* socket, int error)
	{
        HUNUSED(socket);
        sigClose(this, error);
	}

} // namespace qy
