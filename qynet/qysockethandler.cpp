#include <algorithm>
#include "qysockethandler.h"
#include "qyasyncsocket.h"

namespace  qy {

QySocketHandler::QySocketHandler()
{
}

void QySocketHandler::add(QyAsyncSocket* socket)
{
    if (mSockets.end()==std::find(mSockets.begin(),mSockets.end(),socket))
        mSockets.push_back(socket);
}

void QySocketHandler::remove(QyAsyncSocket* socket)
{
    mSockets.erase(std::remove(mSockets.begin(),mSockets.end(),socket),mSockets.end());
}

}//namespace qy
