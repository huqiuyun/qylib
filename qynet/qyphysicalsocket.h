#ifndef QY_PHYSICALSOCKET_H__
#define QY_PHYSICALSOCKET_H__

#include "qyasyncsocket.h"

namespace qy {

class QyPhysicalSocketPrivate;
class QyPhysicalSocket : public QyAsyncSocket
{
public:
    QyPhysicalSocket(SOCKET s = INVALID_SOCKET);
    virtual ~QyPhysicalSocket();

    SOCKET socket() const;
    QySocketAddress localAddress() const;
    QySocketAddress remoteAddress() const;

    int  error() const;
    void setError(int error);
    ConnState connState() const;
    void setConnState(ConnState s);
    uint32 enabledEvent() const;
    void setEnabledEvent(uint32 e);

    bool open(int type);
    int  bind(const QySocketAddress& addr);
    int  connect(const QySocketAddress& addr);
    int  setOption(int opt, int optflag, const void *value, size_t valLen);

    int  send(const void *pv, size_t cb);
    int  sendTo(const void *pv, size_t cb, const QySocketAddress& addr);
    int  recv(void *pv, size_t cb);
    int  recvFrom(void *pv, size_t cb, QySocketAddress *paddr);
    int  listen(int backlog);
    int  close();
    int  estimateMTU(uint16* mtu);

protected:
    void updateLastError();

private:
    QyPhysicalSocketPrivate* d_ptr;
};

} // namespace qy

#endif // QY_PHYSICALSOCKET_H__
