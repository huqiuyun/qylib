#ifndef QY_SocketFactory_H__
#define QY_SocketFactory_H__

#include "qysocket.h"
#include "qywake.h"
#include "qyAsyncSocket.h"

namespace qy
{
    enum EventMode
    {
        EM_SELECT = 0,
        EM_IOCP//windows iocp , linux epoll
    };

    class QySocketFactory : public QyWake
	{
	public:
        QySocketFactory(EventMode mode = EM_SELECT) :
            mMode(mode)
        {
        }
        virtual ~QySocketFactory() {}

        EventMode mode() const { return mMode; }
		// Returns a new socket for blocking communication.  The type can be
		// SOCK_DGRAM and SOCK_STREAM.
        virtual QySocket* createSocket(int type) = 0;

		// Returns a new socket for nonblocking communication.  The type can be
		// SOCK_DGRAM and SOCK_STREAM.
        virtual QyAsyncSocket* createAsyncSocket(int type) = 0;
    private:
        EventMode mMode;
	};

} // namespace qy

#endif // QY_SocketFactory_H__
