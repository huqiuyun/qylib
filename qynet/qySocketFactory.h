#ifndef QY_SocketFactory_H__
#define QY_SocketFactory_H__

#include "qysocket.h"
#include "qyAsyncSocket.h"

namespace qy
{
    class QySocketFactory
	{
	public:
        virtual ~QySocketFactory() {}

		// Returns a new socket for blocking communication.  The type can be
		// SOCK_DGRAM and SOCK_STREAM.
        virtual QySocket* createSocket(int type) = 0;

		// Returns a new socket for nonblocking communication.  The type can be
		// SOCK_DGRAM and SOCK_STREAM.
        virtual QyAsyncSocket* createAsyncSocket(int type) = 0;
	};

} // namespace qy

#endif // QY_SocketFactory_H__
