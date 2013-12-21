
#include "qyphysicalsocket.h"
#include "qyphysicalsocketfactory.h"
#include "qydebug.h"

namespace qy {

class  PhysicalSocketPrivate
{
public:
    PhysicalSocketPrivate():
        sfactroy_(NULL),
        s_(INVALID_SOCKET),
        enabled_events_(0),
        error_(0),
        state_(QySocket::CS_CLOSED)
    {
    }
    QyPhysicalSocketFactory* sfactroy_;
    SOCKET    s_;
    uint32    enabled_events_;
    int       error_;
    QySocket::ConnState state_;
};

PhysicalSocket::PhysicalSocket(QyPhysicalSocketFactory* factory, SOCKET s) :
    d_ptr(new PhysicalSocketPrivate)
{
    d_ptr->sfactroy_ = factory;
    d_ptr->s_ = s;
    if (s != INVALID_SOCKET) {
        d_ptr->enabled_events_ = kfRead | kfWrite;
        d_ptr->state_  = CS_CONNECTED;
    }
}

PhysicalSocket::~PhysicalSocket()
{
    close();
    delete d_ptr;
}

// Creates the underlying OS socket (same as the "socket" function).
bool PhysicalSocket::open(int type)
{
    close();
    d_ptr->s_ = ::socket(AF_INET, type, 0);
    updateLastError();
    if (type != SOCK_STREAM)
        d_ptr->enabled_events_ = kfRead | kfWrite;
    return d_ptr->s_ != INVALID_SOCKET;
}

QySocketAddress PhysicalSocket::localAddress() const
{
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int result = ::getsockname(d_ptr->s_, (sockaddr*)&addr, &addrlen);
    ASSERT(addrlen == sizeof(addr));
    QySocketAddress address;
    if (result >= 0) {
        address.fromSockAddr(addr);
    } else {
        ASSERT(result >= 0);
    }
    return address;
}

QySocketAddress PhysicalSocket::remoteAddress() const
{
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int result = ::getpeername(d_ptr->s_, (sockaddr*)&addr, &addrlen);
    ASSERT(addrlen == sizeof(addr));
    QySocketAddress address;
    if (result >= 0) {
        address.fromSockAddr(addr);
    } else {
        ASSERT(errno == ENOTCONN);
    }
    return address;
}

int PhysicalSocket::bind(const QySocketAddress& addr)
{
    sockaddr_in saddr;
    addr.toSockAddr(&saddr);
    int err = ::bind(d_ptr->s_, (sockaddr*)&saddr, sizeof(saddr));
    updateLastError();
    return err;
}

int PhysicalSocket::connect(const QySocketAddress& addr)
{
    // TODO: Implicit creation is required to reconnect...
    // ...but should we make it more explicit?
    if (d_ptr->s_ == INVALID_SOCKET) {
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
    int err = ::connect(d_ptr->s_, (sockaddr*)&saddr, sizeof(saddr));
    updateLastError();
    if (err == 0)
    {
        d_ptr->state_ = CS_CONNECTED;
    }
    else if (isBlockingError(error()))
    {
        d_ptr->state_ = CS_CONNECTING;
        d_ptr->enabled_events_ |= kfConnect;
    }
    d_ptr->enabled_events_ |= kfRead | kfWrite;
    return err;
}

int PhysicalSocket::error() const
{
    return d_ptr->error_;
}

void PhysicalSocket::setError(int error)
{
    d_ptr->error_ = error;
}

QySocket::ConnState PhysicalSocket::state() const
{
    return d_ptr->state_;
}

int PhysicalSocket::setOption(Option opt, int value)
{
    HUNUSED2(opt,value);
    return -1;
}

int PhysicalSocket::send(const void *pv, size_t cb)
{
    int sent = ::send(d_ptr->s_, reinterpret_cast<const char *>(pv), (int)cb, 0);
    updateLastError();
    ASSERT(sent <= static_cast<int>(cb));  // We have seen minidumps where this may be false
    if ((sent < 0) && isBlockingError(error())) {
        d_ptr->enabled_events_ |= kfWrite;
    }
    return sent;
}

int PhysicalSocket::sendTo(const void *pv, size_t cb, const QySocketAddress& addr)
{
    sockaddr_in saddr;
    addr.toSockAddr(&saddr);
    int sent = ::sendto(d_ptr->s_, (const char *)pv, (int)cb, 0, (sockaddr*)&saddr,sizeof(saddr));
    updateLastError();
    ASSERT(sent <= static_cast<int>(cb));  // We have seen minidumps where this may be false
    if ((sent < 0) && isBlockingError(error())) {
        d_ptr->enabled_events_ |= kfWrite;
    }
    return sent;
}

int PhysicalSocket::recv(void *pv, size_t cb)
{
    int received = ::recv(d_ptr->s_, (char *)pv, (int)cb, 0);
    if ((received == 0) && (cb != 0))
    {
        // Note: on graceful shutdown, recv can return 0.  In this case, we
        // pretend it is blocking, and then signal close, so that simplifying
        // assumptions can be made about Recv.
        d_ptr->error_ = EWOULDBLOCK;
        return SOCKET_ERROR;
    }
    updateLastError();
    if ((received >= 0) || isBlockingError(error())) {
        d_ptr->enabled_events_ |= kfRead;
    }
    return received;
}

int PhysicalSocket::recvFrom(void *pv, size_t cb, QySocketAddress *paddr)
{
    sockaddr_in saddr;
    socklen_t cbAddr = sizeof(saddr);
    int received = ::recvfrom(d_ptr->s_, (char *)pv, (int)cb, 0, (sockaddr*)&saddr,&cbAddr);
    updateLastError();
    if ((received >= 0) && (paddr != NULL))
        paddr->fromSockAddr(saddr);
    if ((received >= 0) || isBlockingError(error())) {
        d_ptr->enabled_events_ |= kfRead;
    }
    return received;
}

int PhysicalSocket::listen(int backlog)
{
    int err = ::listen(d_ptr->s_, backlog);
    updateLastError();
    if (err == 0)
        d_ptr->state_ = CS_CONNECTING;
    d_ptr->enabled_events_ |= kfRead;
    return err;
}

int PhysicalSocket::close()
{
    if (d_ptr->s_ == INVALID_SOCKET)
        return 0;
    int err = ::closesocket(d_ptr->s_);
    updateLastError();
    d_ptr->s_ = INVALID_SOCKET;
    d_ptr->state_ = CS_CLOSED;
    d_ptr->enabled_events_ = 0;
    return err;
}

int PhysicalSocket::estimateMTU(uint16* mtu)
{
    HUNUSED(mtu);
    QySocketAddress addr = remoteAddress();
    if (addr.isAny())
    {
        d_ptr->error_ = ENOTCONN;
        return -1;
    }
    ASSERT(0);

    return 0;
}

void PhysicalSocket::updateLastError()
{
#ifdef H_OS_WIN
    d_ptr->error_ = WSAGetLastError();
#else
    d_ptr->error_ = errno;
#endif
}

QyPhysicalSocketFactory* PhysicalSocket::factory() const
{
    return d_ptr->sfactroy_;
}

} // namespace qy
