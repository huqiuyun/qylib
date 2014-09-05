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
#include "qyByteBuffer.h"
#include "qydebug.h"

namespace qy 
{
    QyBufferedReadAdapter::QyBufferedReadAdapter(QyAsyncSocket* socket, size_t buffer_size)
        : QyAsyncSocketAdapter(socket)
        , mBufferSize(buffer_size)
        , mDataLen(0)
        , mBuffering(false)
	{
            mBuffer = new char[mBufferSize];
	}

    QyBufferedReadAdapter::~QyBufferedReadAdapter()
	{
        delete [] mBuffer;
	}

    int QyBufferedReadAdapter::send(const void *pv, size_t cb)
	{
        if (mBuffering)
		{
			// TODO: Spoof error better; Signal Writeable
            mSocket->setError(EWOULDBLOCK);
			return -1;
		}
        return QyAsyncSocketAdapter::send(pv, cb);
	}

    int QyBufferedReadAdapter::recv(void *pv, size_t cb)
	{
        if (mBuffering)
		{
            mSocket->setError(EWOULDBLOCK);
			return -1;
		}

		size_t read = 0;

        if (mDataLen)
		{
            read = _min(cb, mDataLen);
            memcpy(pv, mBuffer, read);
            mDataLen -= read;
            if (mDataLen > 0) {
                memmove(mBuffer, mBuffer + read, mDataLen);
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
        mBuffering = on;
	}

    void QyBufferedReadAdapter::onReadEvent(QyAsyncSocket * socket)
	{
        ASSERT(socket == mSocket);

        if (!mBuffering)
		{
            QyAsyncSocketAdapter::onReadEvent(socket);
			return;
		}

        if (mDataLen >= mBufferSize) {
			ASSERT(false);
            mDataLen = 0;
		}

        int len = mSocket->recv(mBuffer + mDataLen, mBufferSize - mDataLen);
        if (len < 0) {
			return;
		}

        mDataLen += len;

        processInput(mBuffer, mDataLen);
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
        ASSERT(socket == mSocket);

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

    QyAsyncProxySocket::QyAsyncProxySocket(QyAsyncSocket* socket,
        const QySocketAddress& proxy,
        const std::string& username, const QyCryptString& password)
        : QyBufferedReadAdapter(socket, 1024)
        , mProxy(proxy)
        , mUser(username)
        , mPassword(password)
        , mProxyState(SS_ERROR)
	{
	}

    int QyAsyncProxySocket::connect(const QySocketAddress& addr)
	{
        mDest = addr;
        bufferInput(true);
        return QyBufferedReadAdapter::connect(mProxy);
	}

    QySocketAddress QyAsyncProxySocket::remoteAddress() const
	{
        return mDest;
	}

    void QyAsyncProxySocket::onConnectEvent(QyAsyncSocket *)
	{
        sendHello();
	}

    void QyAsyncProxySocket::processInput(char * data, size_t& len)
	{
        ASSERT(mProxyState < SS_TUNNEL);

        QyByteBuffer response(data, len);

        if (mProxyState == SS_HELLO) {
            uint8 ver=0, method=255;
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
        } else if (mProxyState == SS_AUTH) {
			uint8 ver, status;
			if (!response.ReadUInt8(ver) ||
				!response.ReadUInt8(status))
				return;

			if ((ver != 1) || (status != 0)) {
                setError(SOCKET_EACCES);
				return;
			}

            sendConnect();
        } else if (mProxyState == SS_CONNECT) {
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

            mProxyState = SS_TUNNEL;
		}

		// Consume parsed data
		len = response.Length();
		memcpy(data, response.Data(), len);

        if (mProxyState != SS_TUNNEL)
			return;

		bool remainder = (len > 0);
        bufferInput(false);
        sigConnectEvent(this);

		// FIX: if SignalConnect causes the socket to be destroyed, we are in trouble
		if (remainder)
            sigReadEvent(this); // TODO: signal this??
	}

    void QyAsyncProxySocket::sendHello()
	{
        QyByteBuffer request;
		request.WriteUInt8(5);   // Socks Version
        if (mUser.empty()) {
			request.WriteUInt8(1); // Authentication Mechanisms
			request.WriteUInt8(0); // No authentication
		} else {
			request.WriteUInt8(2); // Authentication Mechanisms
			request.WriteUInt8(0); // No authentication
			request.WriteUInt8(2); // Username/Password
		}
        directSend(request.Data(), request.Length());
        mProxyState = SS_HELLO;
	}

    void QyAsyncProxySocket::sendAuth()
	{
        QyByteBuffer request;
		request.WriteUInt8(1);      // Negotiation Version
        request.WriteUInt8(static_cast<uint8>(mUser.size()));
        request.WriteString(mUser); // Username
        request.WriteUInt8(static_cast<uint8>(mPassword.length()));
        size_t len = mPassword.length() + 1;
		char * sensitive = new char[len];
        mPassword.copyTo(sensitive, true);
		request.WriteString(sensitive); // Password
		memset(sensitive, 0, len);
		delete [] sensitive;
        directSend(request.Data(), request.Length());
        mProxyState = SS_AUTH;
	}

    void QyAsyncProxySocket::sendConnect()
	{
        QyByteBuffer request;
		request.WriteUInt8(5);             // Socks Version
		request.WriteUInt8(1);             // CONNECT
		request.WriteUInt8(0);             // Reserved
        if (mDest.isUnresolved()) {
            std::string hostname = mDest.ipAsString();
			request.WriteUInt8(3);           // DOMAINNAME
			request.WriteUInt8(static_cast<uint8>(hostname.size()));
			request.WriteString(hostname);   // Destination Hostname
		} else {
			request.WriteUInt8(1);           // IPV4
            request.WriteUInt32(mDest.ip()); // Destination IP
		}
        request.WriteUInt16(mDest.port()); // Destination Port
        directSend(request.Data(), request.Length());
        mProxyState = SS_CONNECT;
	}

    void QyAsyncProxySocket::setError(int error)
	{
        mProxyState = SS_ERROR;
        bufferInput(false);
        close();
        setError(SOCKET_EACCES);
        sigCloseEvent(this, error);
	}

} // namespace qy
