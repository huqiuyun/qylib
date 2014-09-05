#ifndef QY_SOCKETHANDLER_H
#define QY_SOCKETHANDLER_H

#include <list>
#include "qylock.h"

namespace qy {

class QyAsyncSocket;
typedef std::list<QyAsyncSocket*> SocketList;

class QySocketHandler
{
public:
    QySocketHandler();

    void add(QyAsyncSocket* socket);
    void remove(QyAsyncSocket* socket);
protected:
    SocketList mSockets;
};
} //namespace qy

#endif // QY_SOCKETHANDLER_H
