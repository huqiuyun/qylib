#include "qysysconfig.h"


#ifdef H_OS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#define _WINSOCKAPI_
#include <windows.h>
#define SECURITY_WIN32
#include <security.h>
#endif
#include <time.h>
#include <errno.h>
#include "qysocketadapters.h"
#include "qybytebuffer.h"
#include "qydebug.h"

namespace qy 
{
    QyBufferedReadAdapter::QyBufferedReadAdapter(QyAsyncSocket* socket, size_t buffer_size)
        : QyAsyncSocketAdapter(socket)
		, buffer_size_(buffer_size)
		, data_len_(0)
		, buffering_(false)
	{
			buffer_ = new char[buffer_size_];
	}

    QyBufferedReadAdapter::~QyBufferedReadAdapter()
	{
		delete [] buffer_;
	}

    int QyBufferedReadAdapter::send(const void *pv, size_t cb)
	{
		if (buffering_) 
		{
			// TODO: Spoof error better; Signal Writeable
            socket_->setError(EWOULDBLOCK);
			return -1;
		}
        return QyAsyncSocketAdapter::send(pv, cb);
	}

    int QyBufferedReadAdapter::recv(void *pv, size_t cb)
	{
		if (buffering_)
		{
            socket_->setError(EWOULDBLOCK);
			return -1;
		}

		size_t read = 0;

		if (data_len_)
		{
			read = _min(cb, data_len_);
			memcpy(pv, buffer_, read);
			data_len_ -= read;
			if (data_len_ > 0) {
				memmove(buffer_, buffer_ + read, data_len_);
			}
			pv = static_cast<char *>(pv) + read;
			cb -= read;
		}

		// FIX: If cb == 0, we won't generate another read event

        int res = QyAsyncSocketAdapter::recv(pv, cb);
		if (res < 0)
			return res;

		return res + static_cast<int>(read);
	}

    void QyBufferedReadAdapter::bufferInput(bool on)
	{
		buffering_ = on;
	}

    void QyBufferedReadAdapter::onReadEvent(QyAsyncSocket * socket)
	{
		ASSERT(socket == socket_);

		if (!buffering_) 
		{
            QyAsyncSocketAdapter::onReadEvent(socket);
			return;
		}

        if (data_len_ >= buffer_size_) {
			ASSERT(false);
			data_len_ = 0;
		}

        int len = socket_->recv(buffer_ + data_len_, buffer_size_ - data_len_);
        if (len < 0) {
			return;
		}

		data_len_ += len;

        processInput(buffer_, data_len_);
	}

	///////////////////////////////////////////////////////////////////////////////

	const uint8 SSL_SERVER_HELLO[] = {
		22,3,1,0,74,2,0,0,70,3,1,66,133,69,167,39,169,93,160,
		179,197,231,83,218,72,43,63,198,90,202,137,193,88,82,
		161,120,60,91,23,70,0,133,63,32,14,211,6,114,91,91,
		27,95,21,172,19,249,136,83,157,155,232,61,123,12,48,
		50,110,56,77,162,117,87,65,108,52,92,0,4,0
	};

	const char SSL_CLIENT_HELLO[] = {
		-128,70,1,3,1,0,45,0,0,0,16,1,0,-128,3,0,-128,7,0,-64,6,0,64,2,0,
		-128,4,0,-128,0,0,4,0,-2,-1,0,0,10,0,-2,-2,0,0,9,0,0,100,0,0,98,0,
		0,3,0,0,6,31,23,12,-90,47,0,120,-4,70,85,46,-79,-125,57,-15,-22
	};

    QyAsyncSSLSocket::QyAsyncSSLSocket(QyAsyncSocket* socket)
        : QyBufferedReadAdapter(socket, 1024)
	{
	}

    int QyAsyncSSLSocket::connect(const QySocketAddress& addr)
	{
		// Begin buffering before we connect, so that there isn't a race condition between
		// potential senders and receiving the OnConnectEvent signal
        bufferInput(true);
        return QyBufferedReadAdapter::connect(addr);
	}

    void QyAsyncSSLSocket::onConnectEvent(QyAsyncSocket * socket)
	{
		ASSERT(socket == socket_);

		// TODO: we could buffer output too...
        int res = directSend(SSL_CLIENT_HELLO, sizeof(SSL_CLIENT_HELLO));
		ASSERT(res == sizeof(SSL_CLIENT_HELLO));
        HUNUSED2(socket,res);
	}

    void QyAsyncSSLSocket::processInput(char * data, size_t& len)
	{
		if (len < sizeof(SSL_SERVER_HELLO))
			return;

		if (memcmp(SSL_SERVER_HELLO, data, sizeof(SSL_SERVER_HELLO)) != 0) {
            close();
            sigCloseEvent(this, 0); // TODO: error code?
			return;
		}

		len -= sizeof(SSL_SERVER_HELLO);
		if (len > 0) {
			memmove(data, data + sizeof(SSL_SERVER_HELLO), len);
		}

		bool remainder = (len > 0);
        bufferInput(false);
        sigConnectEvent(this);

		// FIX: if SignalConnect causes the socket to be destroyed, we are in trouble
		if (remainder)
            sigReadEvent(this);
	}

	///////////////////////////////////////////////////////////////////////////////

    QyAsyncSocksProxySocket::QyAsyncSocksProxySocket(QyAsyncSocket* socket,
        const QySocketAddress& proxy,
        const std::string& username, const QyCryptString& password)
        : QyBufferedReadAdapter(socket, 1024)
		, proxy_(proxy)
		, user_(username)
		, pass_(password)
		, state_(SS_ERROR)
	{
	}

    int QyAsyncSocksProxySocket::connect(const QySocketAddress& addr)
	{
		dest_ = addr;
        bufferInput(true);
        return QyBufferedReadAdapter::connect(proxy_);
	}

    QySocketAddress QyAsyncSocksProxySocket::remoteAddress() const
	{
		return dest_;
	}

    void QyAsyncSocksProxySocket::onConnectEvent(QyAsyncSocket *)
	{
        sendHello();
	}

    void QyAsyncSocksProxySocket::processInput(char * data, size_t& len)
	{
		ASSERT(state_ < SS_TUNNEL);

		ByteBuffer response(data, len);

		if (state_ == SS_HELLO) {
			uint8 ver, method;
			if (!response.ReadUInt8(ver) ||
				!response.ReadUInt8(method))
				return;

			if (ver != 5) {
                setError(0);
				return;
			}

			if (method == 0) {
                sendConnect();
			} else if (method == 2) {
                sendAuth();
			} else {
                setError(0);
				return;
			}
		} else if (state_ == SS_AUTH) {
			uint8 ver, status;
			if (!response.ReadUInt8(ver) ||
				!response.ReadUInt8(status))
				return;

			if ((ver != 1) || (status != 0)) {
                setError(SOCKET_EACCES);
				return;
			}

            sendConnect();
		} else if (state_ == SS_CONNECT) {
			uint8 ver, rep, rsv, atyp;
			if (!response.ReadUInt8(ver) ||
				!response.ReadUInt8(rep) ||
				!response.ReadUInt8(rsv) ||
				!response.ReadUInt8(atyp))
				return;

			if ((ver != 5) || (rep != 0)) {
                setError(0);
				return;
			}

			uint16 port;
			if (atyp == 1) {
				uint32 addr;
				if (!response.ReadUInt32(addr) ||
					!response.ReadUInt16(port))
                    return;

			} else if (atyp == 3) {
				uint8 len;
				std::string addr;
				if (!response.ReadUInt8(len) ||
					!response.ReadString(addr, len) ||
					!response.ReadUInt16(port))
                    return;

			} else if (atyp == 4) {
				std::string addr;
				if (!response.ReadString(addr, 16) ||
					!response.ReadUInt16(port))
                    return;

			} else {
                setError(0);
				return;
			}

			state_ = SS_TUNNEL;
		}

		// Consume parsed data
		len = response.Length();
		memcpy(data, response.Data(), len);

		if (state_ != SS_TUNNEL)
			return;

		bool remainder = (len > 0);
        bufferInput(false);
        sigConnectEvent(this);

		// FIX: if SignalConnect causes the socket to be destroyed, we are in trouble
		if (remainder)
            sigReadEvent(this); // TODO: signal this??
	}

    void QyAsyncSocksProxySocket::sendHello()
	{
		ByteBuffer request;
		request.WriteUInt8(5);   // Socks Version
		if (user_.empty()) {
			request.WriteUInt8(1); // Authentication Mechanisms
			request.WriteUInt8(0); // No authentication
		} else {
			request.WriteUInt8(2); // Authentication Mechanisms
			request.WriteUInt8(0); // No authentication
			request.WriteUInt8(2); // Username/Password
		}
        directSend(request.Data(), request.Length());
		state_ = SS_HELLO;
	}

    void QyAsyncSocksProxySocket::sendAuth()
	{
		ByteBuffer request;
		request.WriteUInt8(1);      // Negotiation Version
		request.WriteUInt8(static_cast<uint8>(user_.size()));
		request.WriteString(user_); // Username
        request.WriteUInt8(static_cast<uint8>(pass_.length()));
        size_t len = pass_.length() + 1;
		char * sensitive = new char[len];
        pass_.copyTo(sensitive, true);
		request.WriteString(sensitive); // Password
		memset(sensitive, 0, len);
		delete [] sensitive;
        directSend(request.Data(), request.Length());
		state_ = SS_AUTH;
	}

    void QyAsyncSocksProxySocket::sendConnect()
	{
		ByteBuffer request;
		request.WriteUInt8(5);             // Socks Version
		request.WriteUInt8(1);             // CONNECT
		request.WriteUInt8(0);             // Reserved
        if (dest_.isUnresolved()) {
            std::string hostname = dest_.ipAsString();
			request.WriteUInt8(3);           // DOMAINNAME
			request.WriteUInt8(static_cast<uint8>(hostname.size()));
			request.WriteString(hostname);   // Destination Hostname
		} else {
			request.WriteUInt8(1);           // IPV4
			request.WriteUInt32(dest_.ip()); // Destination IP
		}
		request.WriteUInt16(dest_.port()); // Destination Port
        directSend(request.Data(), request.Length());
		state_ = SS_CONNECT;
	}

    void QyAsyncSocksProxySocket::setError(int error)
	{
		state_ = SS_ERROR;
        bufferInput(false);
        close();
        setError(SOCKET_EACCES);
        sigCloseEvent(this, error);
	}

} // namespace qy
