#ifndef QY_ASYNCTCPSOCKET_H__
#define QY_ASYNCTCPSOCKET_H__

#include "qyAsyncPacketSocket.h"

namespace qy
{

	// Simulates UDP semantics over TCP.  Send and Recv packet sizes
	// are preserved, and drops packets silently on Send, rather than
	// buffer them in user space.
    class AsyncTCPSocket : public QyAsyncPacketSocket
	{
	public:
        AsyncTCPSocket(QyAsyncSocket* socket);
		virtual ~AsyncTCPSocket();

        virtual int send(const void *pv, size_t cb);
        virtual int sendTo(const void *pv, size_t cb, const QySocketAddress& addr);

        sigslot::signal1<AsyncTCPSocket*> sigConnect;
        sigslot::signal2<AsyncTCPSocket*,int> sigClose;

	protected:
        int sendRaw(const void * pv, size_t cb);
        virtual void processInput(char * data, size_t& len);

    private:
        int flush();

		// Called by the underlying socket
        void onConnectEvent(QyAsyncSocket* socket);
        void onReadEvent(QyAsyncSocket* socket);
        void onWriteEvent(QyAsyncSocket* socket);
        void onCloseEvent(QyAsyncSocket* socket, int error);

    private:
        char* inbuf_, * outbuf_;
        size_t insize_, inpos_, outsize_, outpos_;
	};

} // namespace qy

#endif // QY_ASYNCTCPSOCKET_H__
