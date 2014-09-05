#include "qyAsyncPacketSocket.h"

namespace qy 
{
    QyAsyncPacketSocket::QyAsyncPacketSocket(QyAsyncSocket* s) :
        mSocket(s)
	{
	}

    QyAsyncPacketSocket::~QyAsyncPacketSocket() {
        delete mSocket;
	}

    SOCKET QyAsyncPacketSocket::socket() const
    {
        return mSocket->socket();
    }

    QySocketAddress QyAsyncPacketSocket::localAddress() const
	{
        return mSocket->localAddress();
	}

    QySocketAddress QyAsyncPacketSocket::remoteAddress() const
	{
        return mSocket->remoteAddress();
	}

    int QyAsyncPacketSocket::bind(const QySocketAddress& addr)
	{
        return mSocket->bind(addr);
	}

    int QyAsyncPacketSocket::connect(const QySocketAddress& addr)
	{
        return mSocket->connect(addr);
	}

    int QyAsyncPacketSocket::send(const void *pv, size_t cb)
	{
        return mSocket->send(pv, cb);
	}

    int QyAsyncPacketSocket::sendTo(const void *pv, size_t cb, const QySocketAddress& addr)
	{
            return mSocket->sendTo(pv, cb, addr);
	}

    int QyAsyncPacketSocket::close()
	{
        return mSocket->close();
	}

    int QyAsyncPacketSocket::setOption(int opt, int optflag, const void *value, size_t valLen)
	{
        return mSocket->setOption(opt,optflag,value,valLen);
	}

    int QyAsyncPacketSocket::error() const
	{
        return mSocket->error();
	}

    void QyAsyncPacketSocket::setError(int error)
	{
        return mSocket->setError(error);
	}

} // namespace qy
