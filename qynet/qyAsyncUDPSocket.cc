
#include "qyAsyncUDPSocket.h"
#include <cassert>
#include <cstring>
#include <iostream>

namespace qy 
{
	const int BUF_SIZE = 64 * 1024;
    QyAsyncUDPSocket::QyAsyncUDPSocket(QyAsyncSocket* socket) :
        QyAsyncPacketSocket(socket) {
		size_ = BUF_SIZE;
		buf_ = new char[size_];

		assert(socket_);
		// The socket should start out readable but not writable.
        socket_->sigReadEvent.connect(this, &QyAsyncUDPSocket::onReadEvent);
	}

    QyAsyncUDPSocket::~QyAsyncUDPSocket() {
		delete [] buf_;
	}

    void QyAsyncUDPSocket::onReadEvent(QyAsyncSocket* socket) {
		assert(socket == socket_);

        QySocketAddress remote_addr;
        int len = socket_->recvFrom(buf_, size_, &remote_addr);
        if (len < 0) {
			return;
		}

		// TODO: Make sure that we got all of the packet.  If we did not, then we
		// should resize our buffer to be large enough.

        sigReadPacket(buf_, (size_t)len, remote_addr, this);
	}
} // namespace qy
