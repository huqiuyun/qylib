#include "qyAsyncPacketSocket.h"

namespace qy 
{
    QyAsyncPacketSocket::QyAsyncPacketSocket(QyAsyncSocket* socket) : socket_(socket)
	{
	}

    QyAsyncPacketSocket::~QyAsyncPacketSocket() {
		delete socket_;
	}

    QySocketAddress QyAsyncPacketSocket::localAddress() const
	{
        return socket_->localAddress();
	}

    QySocketAddress QyAsyncPacketSocket::remoteAddress() const
	{
        return socket_->remoteAddress();
	}

    int QyAsyncPacketSocket::bind(const QySocketAddress& addr)
	{
        return socket_->bind(addr);
	}

    int QyAsyncPacketSocket::connect(const QySocketAddress& addr)
	{
        return socket_->connect(addr);
	}

    int QyAsyncPacketSocket::send(const void *pv, size_t cb)
	{
        return socket_->send(pv, cb);
	}

    int QyAsyncPacketSocket::sendTo(const void *pv, size_t cb, const QySocketAddress& addr)
	{
            return socket_->sendTo(pv, cb, addr);
	}

    int QyAsyncPacketSocket::close()
	{
        return socket_->close();
	}

    int QyAsyncPacketSocket::setOption(QySocket::Option opt, int value)
	{
        return socket_->setOption(opt, value);
	}

    int QyAsyncPacketSocket::error() const
	{
        return socket_->error();
	}

    void QyAsyncPacketSocket::setError(int error)
	{
        return socket_->setError(error);
	}

} // namespace qy
