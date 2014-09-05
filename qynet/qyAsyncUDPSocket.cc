
#include "qyAsyncUDPSocket.h"
#include <cassert>
#include <cstring>
#include <iostream>

namespace qy 
{
	const int BUF_SIZE = 64 * 1024;
    QyAsyncUDPSocket::QyAsyncUDPSocket(QyAsyncSocket* socket) :
        QyAsyncPacketSocket(socket) {
        mSize = BUF_SIZE;
        mBuf = new char[mSize];

        assert(mSocket);
		// The socket should start out readable but not writable.
        mSocket->sigReadEvent.connect(this, &QyAsyncUDPSocket::onReadEvent);
	}

    QyAsyncUDPSocket::~QyAsyncUDPSocket() {
        delete [] mBuf;
	}

    void QyAsyncUDPSocket::onReadEvent(QyAsyncSocket* socket) {
        assert(socket == mSocket);

        QySocketAddress remote_addr;
        int len = mSocket->recvFrom(mBuf, mSize, &remote_addr);
        if (len < 0) {
			return;
		}

		// TODO: Make sure that we got all of the packet.  If we did not, then we
		// should resize our buffer to be large enough.

        sigReadPacket(mBuf, (size_t)len, remote_addr, this);
	}
} // namespace qy
