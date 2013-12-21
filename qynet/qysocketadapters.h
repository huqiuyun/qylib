
#ifndef QY_SOCKETADAPTERS_H__
#define QY_SOCKETADAPTERS_H__

#include <map>
#include <string>
#include "qycryptstring.h"
#include "qyAsyncSocket.h"

namespace qy
{
    class QyBufferedReadAdapter : public QyAsyncSocketAdapter
	{
	public:
        QyBufferedReadAdapter(QyAsyncSocket* socket, size_t buffer_size);
        virtual ~QyBufferedReadAdapter();

        virtual int send(const void *pv, size_t cb);
        virtual int recv(void *pv, size_t cb);

	protected:
        int directSend(const void *pv, size_t cb) {
            return QyAsyncSocketAdapter::send(pv, cb);
        }
        void bufferInput(bool on = true);

        virtual void processInput(char * data, size_t& len) = 0;
        virtual void onReadEvent(QyAsyncSocket * socket);

	private:
		char * buffer_;
		size_t buffer_size_, data_len_;
		bool buffering_;
	};

    class QyAsyncSSLSocket : public QyBufferedReadAdapter
	{
	public:
        QyAsyncSSLSocket(QyAsyncSocket* socket);

        virtual int connect(const QySocketAddress& addr);

	protected:
        virtual void onConnectEvent(QyAsyncSocket * socket);
        virtual void processInput(char * data, size_t& len);
	};

    class QyAsyncSocksProxySocket : public QyBufferedReadAdapter
	{
	public:
        QyAsyncSocksProxySocket(QyAsyncSocket* socket, const QySocketAddress& proxy,
            const std::string& username, const QyCryptString& password);

        virtual int connect(const QySocketAddress& addr);
        virtual QySocketAddress remoteAddress() const;

	protected:
        virtual void onConnectEvent(QyAsyncSocket * socket);
        virtual void processInput(char * data, size_t& len);

        void sendHello();
        void sendConnect();
        void sendAuth();
        void setError(int error);

	private:
        QySocketAddress proxy_, dest_;
		std::string user_;
        QyCryptString pass_;
		enum SocksState { SS_HELLO, SS_AUTH, SS_CONNECT, SS_TUNNEL, SS_ERROR } state_;
	};

} // namespace qy

#endif // QY_SOCKETADAPTERS_H__
