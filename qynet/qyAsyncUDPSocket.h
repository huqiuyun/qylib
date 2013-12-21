#ifndef QY_AsyncUDPSocket_H__
#define QY_AsyncUDPSocket_H__

#include "qyAsyncPacketSocket.h"
#include "qysocketfactory.h"

namespace qy 
{
	// Provides the ability to receive packets asynchronously.  Sends are not
	// buffered since it is acceptable to drop packets under high load.
    class QyAsyncUDPSocket : public QyAsyncPacketSocket
	{
	public:
        QyAsyncUDPSocket(QyAsyncSocket* socket);
        virtual ~QyAsyncUDPSocket();

	private:
		char* buf_;
		size_t size_;

		// Called when the underlying socket is ready to be read from.
        void onReadEvent(QyAsyncSocket* socket);
	};

} // namespace qy

#endif // QY_AsyncUDPSocket_H__
