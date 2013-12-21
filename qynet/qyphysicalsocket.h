#ifndef QY_PHYSICALSOCKET_H__
#define QY_PHYSICALSOCKET_H__

#include "qyasyncsocket.h"

namespace qy {

class QyPhysicalSocketFactory;
class PhysicalSocketPrivate;
class PhysicalSocket : public QyAsyncSocket
{
public:
    PhysicalSocket(QyPhysicalSocketFactory* factory, SOCKET s = INVALID_SOCKET);
    virtual ~PhysicalSocket();

    QySocketAddress localAddress() const;
    QySocketAddress remoteAddress() const;

    bool open(int type);
    int  bind(const QySocketAddress& addr);
    int  connect(const QySocketAddress& addr);
    int  error() const;
    void setError(int error);
    ConnState state() const;
    int  setOption(Option opt, int value);

    int  send(const void *pv, size_t cb);
    int  sendTo(const void *pv, size_t cb, const QySocketAddress& addr);
    int  recv(void *pv, size_t cb);
    int  recvFrom(void *pv, size_t cb, QySocketAddress *paddr);
    int  listen(int backlog);
    int  close();
    int  estimateMTU(uint16* mtu);

    QyPhysicalSocketFactory* factory() const;
protected:
    void updateLastError();

private:
    PhysicalSocketPrivate* d_ptr;
};

} // namespace qy

#endif // QY_PHYSICALSOCKET_H__
