#ifndef QY_AsyncPacketmSocketH__
#define QY_AsyncPacketmSocketH__

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
        virtual SOCKET socket() const;
        virtual QySocketAddress localAddress() const;
        virtual QySocketAddress remoteAddress() const;
        virtual int bind(const QySocketAddress& addr);
        virtual int connect(const QySocketAddress& addr);
        virtual int send(const void *pv, size_t cb);
        virtual int sendTo(const void *pv, size_t cb, const QySocketAddress& addr);
        virtual int close();
        virtual int setOption(int opt, int optflag, const void *value, size_t valLen);
        virtual int error() const;
        virtual void setError(int error);

		// Emitted each time a packet is read.
        sigslot::signal4<const char*, size_t, const QySocketAddress&, QyAsyncPacketSocket*> sigReadPacket;

	protected:
        QyAsyncSocket* mSocket;
	};
} // namespace qy

#endif // QY_AsyncPacketmSocketH__
