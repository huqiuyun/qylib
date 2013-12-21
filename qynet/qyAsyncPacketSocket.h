#ifndef QY_AsyncPacketSocket_H__
#define QY_AsyncPacketSocket_H__

#include "qyAsyncSocket.h"

namespace qy
{
	// Provides the ability to receive packets asynchronously.  Sends are not
	// buffered since it is acceptable to drop packets under high load.
    class QyAsyncPacketSocket : public sigslot::has_slots<>
	{
	public:
        QyAsyncPacketSocket(QyAsyncSocket* socket);
        virtual ~QyAsyncPacketSocket();

		// Relevant socket methods:
        virtual QySocketAddress localAddress() const;
        virtual QySocketAddress remoteAddress() const;
        virtual int bind(const QySocketAddress& addr);
        virtual int connect(const QySocketAddress& addr);
        virtual int send(const void *pv, size_t cb);
        virtual int sendTo(const void *pv, size_t cb, const QySocketAddress& addr);
        virtual int close();
        virtual int setOption(QySocket::Option opt, int value);
        virtual int error() const;
        virtual void setError(int error);

		// Emitted each time a packet is read.
        sigslot::signal4<const char*, size_t, const QySocketAddress&, QyAsyncPacketSocket*> sigReadPacket;

	protected:
        QyAsyncSocket* socket_;
	};
} // namespace qy

#endif // QY_AsyncPacketSocket_H__
