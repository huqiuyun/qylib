#ifndef QYDISPATCHER_H
#define QYDISPATCHER_H

#include "qysocket.h"

namespace qy {

class QyDispatcher
{
public:
    QyDispatcher():
        mEventIndex(-1)
    {
    }

    virtual ~QyDispatcher() {
    }
    int indexOfEvent() const { return mEventIndex;}
    void setEventIndex(int index) { mEventIndex = index;}

    virtual void onPreEvent(uint32 ff) = 0;
    virtual void onEvent(uint32 ff, int err) = 0;

    virtual uint32 requestedEvents() const = 0;
    virtual WSAEVENT wsaEvent() const = 0;
    virtual SOCKET sock() const = 0;
private:
    int mEventIndex;
};

} //namespace qy

#endif // QYDISPATCHER_H
