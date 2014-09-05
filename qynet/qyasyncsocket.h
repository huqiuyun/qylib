#ifndef QY_ASYNCSOCKET_H__
#define QY_ASYNCSOCKET_H__

#include "qysigslot.h"
#include "qysocket.h"

namespace qy 
{
	// Provides the ability to perform socket I/O asynchronously.

    class QyAsyncSocket : public QySocket , public sigslot::has_slots<>
	{
	public:
        virtual ~QyAsyncSocket() {}

        sigslot::signal1<QyAsyncSocket*> sigReadEvent;  // ready to read
        sigslot::signal1<QyAsyncSocket*> sigWriteEvent; // ready to write
        sigslot::signal1<QyAsyncSocket*> sigConnectEvent; // connected
        sigslot::signal2<QyAsyncSocket*,int> sigCloseEvent; // closed
	};

    class QyAsyncSocketAdapter : public QyAsyncSocket
    {
    public:
        QyAsyncSocketAdapter(QySocket * socket)
            : mSocket(socket)
        {
        }
        QyAsyncSocketAdapter(QyAsyncSocket * socket) : mSocket(socket)
        {
            socket->sigConnectEvent.connect(this, &QyAsyncSocketAdapter::onConnectEvent);
            socket->sigReadEvent.connect(this, &QyAsyncSocketAdapter::onReadEvent);
            socket->sigWriteEvent.connect(this, &QyAsyncSocketAdapter::onWriteEvent);
            socket->sigCloseEvent.connect(this, &QyAsyncSocketAdapter::onCloseEvent);
        }
        virtual ~QyAsyncSocketAdapter() { delete mSocket; }

        virtual SOCKET socket() const { return mSocket->socket();}
        virtual QySocketAddress localAddress() const { return mSocket->localAddress(); }
        virtual QySocketAddress remoteAddress() const { return mSocket->remoteAddress(); }

        virtual bool open(int type) { return mSocket->open(type); }
        virtual int  bind(const QySocketAddress& addr) { return mSocket->bind(addr); }
        virtual int  connect(const QySocketAddress& addr) {return mSocket->connect(addr); }
        virtual int  send(const void *pv, size_t cb) { return mSocket->send(pv, cb); }
        virtual int  sendTo(const void *pv, size_t cb, const QySocketAddress& addr) { return mSocket->sendTo(pv, cb, addr); }
        virtual int  recv(void *pv, size_t cb) { return mSocket->recv(pv, cb); }
        virtual int  recvFrom(void *pv, size_t cb, QySocketAddress *paddr) { return mSocket->recvFrom(pv, cb, paddr); }
        virtual int  listen(int backlog) { return mSocket->listen(backlog); }
        virtual int  close() { return mSocket->close(); }
        virtual int  error() const { return mSocket->error(); }
        virtual void setError(int error) { return mSocket->setError(error); }
        virtual ConnState connState() const { return mSocket->connState(); }
        virtual int estimateMTU(uint16* mtu) { return mSocket->estimateMTU(mtu); }
        virtual int setOption(int opt, int optflag, const void *value, size_t valLen) { return mSocket->setOption(opt,optflag, value,valLen); }

    protected:
        virtual void onConnectEvent(QyAsyncSocket *) { sigConnectEvent(this); }
        virtual void onReadEvent(QyAsyncSocket * ) { sigReadEvent(this); }
        virtual void onWriteEvent(QyAsyncSocket * ) { sigWriteEvent(this); }
        virtual void onCloseEvent(QyAsyncSocket * , int err) { sigCloseEvent(this, err); }

        QySocket * mSocket;
    };

} // namespace qy

#endif // QY_ASYNCSOCKET_H__
