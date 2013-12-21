#ifndef QY_SOCKET_H__
#define QY_SOCKET_H__

#include "qysysconfig.h"

#ifdef H_OS_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#define _WINSOCKAPI_
#include "qywin32.h"
#else
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define SOCKET_EACCES EACCES
#endif

#include "qybasictypes.h"
#include "qySocketAddress.h"

// Rather than converting errors into a private namespace,
// Reuse the POSIX socket api errors. Note this depends on
// Win32 compatibility.

#ifdef H_OS_WIN
#define EWOULDBLOCK     WSAEWOULDBLOCK
#define EINPROGRESS     WSAEINPROGRESS
#define EALREADY        WSAEALREADY
#define ENOTSOCK        WSAENOTSOCK
#define EDESTADDRREQ    WSAEDESTADDRREQ
#define EMSGSIZE        WSAEMSGSIZE
#define EPROTOTYPE      WSAEPROTOTYPE
#define ENOPROTOOPT     WSAENOPROTOOPT
#define EPROTONOSUPPORT WSAEPROTONOSUPPORT
#define ESOCKTNOSUPPORT WSAESOCKTNOSUPPORT
#define EOPNOTSUPP      WSAEOPNOTSUPP
#define EPFNOSUPPORT    WSAEPFNOSUPPORT
#define EAFNOSUPPORT    WSAEAFNOSUPPORT
#define EADDRINUSE      WSAEADDRINUSE
#define EADDRNOTAVAIL   WSAEADDRNOTAVAIL
#define ENETDOWN        WSAENETDOWN
#define ENETUNREACH     WSAENETUNREACH
#define ENETRESET       WSAENETRESET
#define ECONNABORTED    WSAECONNABORTED
#define ECONNRESET      WSAECONNRESET
#define ENOBUFS         WSAENOBUFS
#define EISCONN         WSAEISCONN
#define ENOTCONN        WSAENOTCONN
#define ESHUTDOWN       WSAESHUTDOWN
#define ETOOMANYREFS    WSAETOOMANYREFS
#define ETIMEDOUT       WSAETIMEDOUT
#define ECONNREFUSED    WSAECONNREFUSED
#define ELOOP           WSAELOOP
#undef ENAMETOOLONG // remove errno.h's definition
#define ENAMETOOLONG    WSAENAMETOOLONG
#define EHOSTDOWN       WSAEHOSTDOWN
#define EHOSTUNREACH    WSAEHOSTUNREACH
#undef ENOTEMPTY // remove errno.h's definition
#define ENOTEMPTY       WSAENOTEMPTY
#define EPROCLIM        WSAEPROCLIM
#define EUSERS          WSAEUSERS
#define EDQUOT          WSAEDQUOT
#define ESTALE          WSAESTALE
#define EREMOTE         WSAEREMOTE
#undef EACCES
#define SOCKET_EACCES   WSAEACCES

#else
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
#define closesocket(s) close(s)
typedef int SOCKET;

#endif // !H_OS_WIN

namespace qy 
{
    inline bool isBlockingError(int e)
	{
		return (e == EWOULDBLOCK) || (e == EAGAIN) || (e == EINPROGRESS);
	}

    const int kfRead    = 0x0001;
    const int kfWrite   = 0x0002;
    const int kfConnect = 0x0004;
    const int kfClose   = 0x0008;

	// General interface for the socket implementations of various networks.  The
	// methods match those of normal UNIX sockets very closely.
    class QySocket
	{
	public:
        virtual ~QySocket() {}

		// Returns the address to which the socket is bound.  If the socket is not
		// bound, then the any-address is returned.
        virtual QySocketAddress localAddress() const = 0;

		// Returns the address to which the socket is connected.  If the socket is
		// not connected, then the any-address is returned.
        virtual QySocketAddress remoteAddress() const = 0;

        virtual bool open(int type) = 0;
        virtual int  bind(const QySocketAddress& addr) = 0;
        virtual int  connect(const QySocketAddress& addr) = 0;
        virtual int  send(const void *pv, size_t cb) = 0;
        virtual int  sendTo(const void *pv, size_t cb, const QySocketAddress& addr) = 0;
        virtual int  recv(void *pv, size_t cb) = 0;
        virtual int  recvFrom(void *pv, size_t cb, QySocketAddress *paddr) = 0;
        virtual int  listen(int backlog) = 0;
        virtual int  close() = 0;
        virtual int  error() const = 0;
        virtual void setError(int error) = 0;
        inline bool  isBlocking() const { return isBlockingError(error()); }

		enum ConnState
		{
			CS_CLOSED,
			CS_CONNECTING,
			CS_CONNECTED
		};
        virtual ConnState state() const = 0;

		enum Option 
		{
			OPT_DONTFRAGMENT
		};
        virtual int setOption(Option opt, int value) = 0;

		// Fills in the given uint16 with the current estimate of the MTU along the
		// path to the address to which this socket is connected.
        virtual int estimateMTU(uint16* mtu) = 0;

	protected:
        QySocket() {}

	private:
        DISALLOW_EVIL_CONSTRUCTORS(QySocket);
	};

} // namespace qy

#endif // QY_SOCKET_H__
