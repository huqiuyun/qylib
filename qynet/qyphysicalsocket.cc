
#include "qyphysicalsocket.h"
#include "qydebug.h"

namespace qy {

class  QyPhysicalSocketPrivate
{
public:
    QyPhysicalSocketPrivate():
        mSock(INVALID_SOCKET),
        mEnabledEvents(0),
        mError(0),
        mState(QySocket::CS_CLOSED)
    {
    }
    SOCKET    mSock;
    uint32    mEnabledEvents;
    int       mError;
    QySocket::ConnState mState;
};

QyPhysicalSocket::QyPhysicalSocket(SOCKET s) :
    d_ptr(new QyPhysicalSocketPrivate)
{
    d_ptr->mSock = s;
    if (s != INVALID_SOCKET) {
        d_ptr->mEnabledEvents = kfRead | kfWrite;
        d_ptr->mState  = CS_CONNECTED;
    }
}

QyPhysicalSocket::~QyPhysicalSocket()
{
    close();
    delete d_ptr;
}

// Creates the underlying OS socket (same as the "socket" function).
bool QyPhysicalSocket::open(int type)
{
    close();
    d_ptr->mSock = ::socket(AF_INET, type, 0);
    updateLastError();
    if (type != SOCK_STREAM) {
        d_ptr->mEnabledEvents = kfRead | kfWrite;
    }
    return d_ptr->mSock != INVALID_SOCKET;
}

SOCKET QyPhysicalSocket::socket() const
{
    return d_ptr->mSock;
}

QySocketAddress QyPhysicalSocket::localAddress() const
{
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int result = ::getsockname(d_ptr->mSock, (sockaddr*)&addr, &addrlen);
    ASSERT(addrlen == sizeof(addr));
    QySocketAddress address;
    if (result >= 0) {
        address.fromSockAddr(addr);
    } else {
        ASSERT(result >= 0);
    }
    return address;
}

QySocketAddress QyPhysicalSocket::remoteAddress() const
{
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int result = ::getpeername(d_ptr->mSock, (sockaddr*)&addr, &addrlen);
    ASSERT(addrlen == sizeof(addr));
    QySocketAddress address;
    if (result >= 0) {
        address.fromSockAddr(addr);
    } else {
        ASSERT(errno == ENOTCONN);
    }
    return address;
}

int QyPhysicalSocket::bind(const QySocketAddress& addr)
{
    sockaddr_in saddr;
    addr.toSockAddr(&saddr);
    int err = ::bind(d_ptr->mSock, (sockaddr*)&saddr, sizeof(saddr));
    updateLastError();
    return err;
}

int QyPhysicalSocket::connect(const QySocketAddress& addr)
{
    // TODO: Implicit creation is required to reconnect...
    // ...but should we make it more explicit?
    if (d_ptr->mSock == INVALID_SOCKET) {
        if (!open(SOCK_STREAM))
            return SOCKET_ERROR;
    }
    QySocketAddress addr2(addr);
    if (addr2.isUnresolved())
    {
        addr2.resolve(); // TODO: Do this async later?
    }
    sockaddr_in saddr;
    addr2.toSockAddr(&saddr);
    int err = ::connect(d_ptr->mSock, (sockaddr*)&saddr, sizeof(saddr));
    updateLastError();
    if (err == 0)
    {
        d_ptr->mState = CS_CONNECTED;
    }
    else if (isBlockingError(error()))
    {
        d_ptr->mState = CS_CONNECTING;
        d_ptr->mEnabledEvents |= kfConnect;
    }
    d_ptr->mEnabledEvents |= kfRead | kfWrite;
    return err;
}

int QyPhysicalSocket::error() const
{
    return d_ptr->mError;
}

void QyPhysicalSocket::setError(int error)
{
    d_ptr->mError = error;
}

QySocket::ConnState QyPhysicalSocket::connState() const
{
    return d_ptr->mState;
}

void QyPhysicalSocket::setConnState(QySocket::ConnState s)
{
    d_ptr->mState = s;
}

uint32 QyPhysicalSocket::enabledEvent() const
{
    return d_ptr->mEnabledEvents;
}

void QyPhysicalSocket::setEnabledEvent(uint32 e)
{
    d_ptr->mEnabledEvents = e;
}

int QyPhysicalSocket::setOption(int opt, int optflag, const void *value, size_t valLen)
{
#ifdef H_OS_WIN
    value = (value == 0) ? 0 : 1;
    return ::setsockopt(
                d_ptr->mSock, opt, optflag, (const char*)value,
                valLen);
#else
    return ::setsockopt(
                d_ptr->mSock, opt, optflag, value, valLen);
#endif
    return -1;
}

int QyPhysicalSocket::send(const void *pv, size_t cb)
{
    int sent = ::send(d_ptr->mSock, reinterpret_cast<const char *>(pv), (int)cb, 0);
    updateLastError();
    ASSERT(sent <= static_cast<int>(cb));  // We have seen minidumps where this may be false
    if ((sent < 0) && isBlockingError(error())) {
        d_ptr->mEnabledEvents |= kfWrite;
    }
    return sent;
}

int QyPhysicalSocket::sendTo(const void *pv, size_t cb, const QySocketAddress& addr)
{
    sockaddr_in saddr;
    addr.toSockAddr(&saddr);
    int sent = ::sendto(d_ptr->mSock, (const char *)pv, (int)cb, 0, (sockaddr*)&saddr,sizeof(saddr));
    updateLastError();
    ASSERT(sent <= static_cast<int>(cb));  // We have seen minidumps where this may be false
    if ((sent < 0) && isBlockingError(error())) {
        d_ptr->mEnabledEvents |= kfWrite;
    }
    return sent;
}

int QyPhysicalSocket::recv(void *pv, size_t cb)
{
    int received = ::recv(d_ptr->mSock, (char *)pv, (int)cb, 0);
    if ((received == 0) && (cb != 0))
    {
        // Note: on graceful shutdown, recv can return 0.  In this case, we
        // pretend it is blocking, and then signal close, so that simplifying
        // assumptions can be made about Recv.
        d_ptr->mError = EWOULDBLOCK;
        return SOCKET_ERROR;
    }
    updateLastError();
    if ((received >= 0) || isBlockingError(error())) {
        d_ptr->mEnabledEvents |= kfRead;
    }
    return received;
}

int QyPhysicalSocket::recvFrom(void *pv, size_t cb, QySocketAddress *paddr)
{
    sockaddr_in saddr;
    socklen_t cbAddr = sizeof(saddr);
    int received = ::recvfrom(d_ptr->mSock, (char *)pv, (int)cb, 0, (sockaddr*)&saddr,&cbAddr);
    updateLastError();
    if ((received >= 0) && (paddr != NULL))
        paddr->fromSockAddr(saddr);
    if ((received >= 0) || isBlockingError(error())) {
        d_ptr->mEnabledEvents |= kfRead;
    }
    return received;
}

int QyPhysicalSocket::listen(int backlog)
{
    int err = ::listen(d_ptr->mSock, backlog);
    updateLastError();
    if (err == 0)
        d_ptr->mState = CS_CONNECTING;
    d_ptr->mEnabledEvents |= kfRead;
    return err;
}

int QyPhysicalSocket::close()
{
    int err = 0;
    if (d_ptr->mSock != INVALID_SOCKET) {
        err = ::closesocket(d_ptr->mSock);
        updateLastError();
        d_ptr->mSock = INVALID_SOCKET;
        d_ptr->mState = CS_CLOSED;
        d_ptr->mEnabledEvents = 0;
    }
    return err;
}

int QyPhysicalSocket::estimateMTU(uint16* mtu)
{
    HUNUSED(mtu);
    QySocketAddress addr = remoteAddress();
    if (addr.isAny())
    {
        d_ptr->mError = ENOTCONN;
        return -1;
    }
#if defined(H_OS_WIN)
#else
#endif
    return 0;
}

void QyPhysicalSocket::updateLastError()
{
#ifdef H_OS_WIN
    d_ptr->mError = WSAGetLastError();
#else
    d_ptr->mError = errno;
#endif
}

} // namespace qy
