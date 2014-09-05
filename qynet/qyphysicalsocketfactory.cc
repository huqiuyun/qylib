#include "qyPhysicalSocketFactory.h"
#include "qyphysicalsocket.h"
#include "qycriticalsection.h"
#include "qydispatcher.h"
#include <vector>
#include <algorithm>
#include <iostream>

#ifdef H_OS_WIN
class WinsockInitializer
{
public:
	WinsockInitializer() 
	{
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD(1, 0);
		err_ = WSAStartup(wVersionRequested, &wsaData);
	}
	~WinsockInitializer() 
	{
		WSACleanup();
	}
	int error() {
		return err_;
	}
private:
	int err_;
};
WinsockInitializer gWinsockinit;

inline uint32 eventsToFlags(uint32 events)
{
    uint32 ffFD = FD_CLOSE | FD_ACCEPT;
    if (events & kfRead)
        ffFD |= FD_READ;
    if (events & kfWrite)
        ffFD |= FD_WRITE;
    if (events & kfConnect)
        ffFD |= FD_CONNECT;
    return ffFD;
}
#else
inline uint32 eventsToFlags(uint32 events)
{
    return events;
}
#endif

namespace qy 
{
    class QyEventDispatcher : public QyDispatcher
    {
        QyEventDispatcher(QyPhysicalSocketFactory* factory) :
            mFactory(factory)
        {
#ifdef H_OS_WIN
            if (mWsaEvent = WSACreateEvent()) {
                factory->add(this);
            }
#else
            mSignaled = false;
            pipe(mFd);
            factory->add(this);
#endif
        }

        ~QyEventDispatcher()
        {
#ifdef H_OS_WIN
            if (mWsaEvent != NULL) {
                mFactory->remove(this);
                WSACloseEvent(mWsaEvent);
                mWsaEvent = NULL;
            }
#else
            mFactory->remove(this);
            close(mFd[0]);
            close(mFd[1]);
#endif
        }

        void signal()
        {
#ifdef H_OS_WIN
            if (mWsaEvent != NULL)
                WSASetEvent(mWsaEvent);
#else
            QyCritScope cs(&mCrit);
            if (!mSignaled) {
              uint8 b = 0;
              write(mFd[1], &b, sizeof(b));
              mSignaled = true;
            }
#endif
        }

        uint32 requestedEvents() const
        {
#ifdef H_OS_WIN
            return 0;
#else
            return kfRead;
#endif
        }

        WSAEVENT wsaEvent() const
        {
#ifdef H_OS_WIN
            return mWsaEvent;
#else
            return mFd[0];
#endif
        }

        SOCKET sock() const
        {
            return INVALID_SOCKET;
        }

        bool  checkClose()
        {
            return false;
        }

        void onPreEvent(uint32 ff)
        {
            HUNUSED(ff);
#ifdef H_OS_WIN
            WSAResetEvent(mWsaEvent);
#else
            QyCritScope cs(&mCrit);
            if (mSignaled) {
              uint8 b;
              read(mFd[0], &b, sizeof(b));
              mSignaled = false;
            }
#endif
        }

        void onEvent(uint32 ff, int err)
        {
            HUNUSED2(ff,err);
        }

    private:
        QyPhysicalSocketFactory* mFactory;
#ifdef H_OS_WIN
        WSAEVENT mWsaEvent;
#else
        int mFd[2];
        bool mSignaled;
        QyCriticalSection mCrit;
#endif
    };

    class QySocketDispatcher : public QyDispatcher, public QyPhysicalSocket
    {
    public:
        QySocketDispatcher(QyPhysicalSocketFactory* factory) :
            mFactory(factory)
        {
        }

        ~QySocketDispatcher()
        {
            close();
        }

        void initialize()
        {
            SOCKET s = socket();
#ifdef H_OS_WIN
            ulong argp = 1;
            ioctlsocket(s, FIONBIO, &argp);
#else
            fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK);
#endif
        }

        bool open(int type)
        {
            if (QyPhysicalSocket::open(type)) {
                mFactory->add(this);
                initialize();
                return true;
            }
            return false;
        }

        int close()
        {
            if (socket() != INVALID_SOCKET) {
                mFactory->remove(this);
                return QyPhysicalSocket::close();
            }
            return 0;
        }

        uint32 requestedEvents() const
        {
            return enabledEvent();
        }

        WSAEVENT wsaEvent() const
        {
            return -1;
        }

        SOCKET sock() const
        {
            return socket();
        }

        void onPreEvent(uint32 ff)
        {
            if ((ff & kfConnect) != 0) {
               setConnState(CS_CONNECTED);
            }
        }

        void onEvent(uint32 ff, int err)
        {
            if ((ff & kfRead) != 0) {
                setEnabledEvent(enabledEvent() & ~kfRead);
                sigReadEvent(this);
            }

            if (((ff & kfWrite) != 0)) {
                setEnabledEvent(enabledEvent() & ~kfWrite);
                sigWriteEvent(this);
            }

            if (((ff & kfConnect) != 0)) {
                setEnabledEvent(enabledEvent() & ~kfConnect);
                sigConnectEvent(this);
            }

            if (((ff & kfClose) != 0)) {
                sigCloseEvent(this, err);
            }
        }
    private:
        QyPhysicalSocketFactory* mFactory;
    };

    //
    class QyPhysicalSocketFactoryPrivate
    {
    public:
        QyPhysicalSocketFactoryPrivate()
        {
            mfWait = false;
        }

        bool wait(unsigned long cms,bool threadrun)
        {
            return _platform(cms,threadrun);
        }

        void wake()
        {
            // epoll , iocp , select
        }

#ifdef H_OS_WIN
        bool _platform(unsigned long cmsWait,bool threadrun)
        {
            HUNUSED(threadrun);
            int cmsTotal = cmsWait;
            int cmsElapsed = 0;
            uint32 msStart = GetMillisecondCount();
            WSAEVENT socket_ev = WSACreateEvent();

            mfWait = true;
            while (mfWait) {
                std::vector<WSAEVENT> events;
                std::vector<QyDispatcher*> event_owners;

                events.push_back(socket_ev);

                { // local lock
                    QyCritScope cr(&mCrit);
                    for (size_t i = 0; i < mDispatchers.size(); ++i) {
                        QyDispatcher * disp = mDispatchers[i];

                        SOCKET s = disp->sock();
                        if (s != INVALID_SOCKET)
                        {
                            //将一个事件对象同一个socket绑定并设置要监视的网络事件
                            WSAEventSelect(s, events[0], eventsToFlags(disp->requestedEvents()));
                            disp->setEventIndex(0);
                        }
                        else
                        {
                            events.push_back(disp->wsaEvent());
                            disp->setEventIndex(i);
                        }
                    }
                }

                // Which is shorter, the delay wait or the asked wait?
                int cmsNext;
                if (cmsWait == kForever) {
                    cmsNext = cmsWait;
                }
                else {
                    cmsNext = cmsTotal - cmsElapsed;
                    if (cmsNext < 0)
                        cmsNext = 0;
                }

                // Wait for one of the events to signal
                DWORD dw = WSAWaitForMultipleEvents(static_cast<DWORD>(events.size()), &events[0], false, cmsNext, false);

                // todo: need a better strategy than this!
                if (dw == WSA_WAIT_FAILED) {
                    WSACloseEvent(socket_ev);
                    return false;
                }

                // Timeout
                if (dw == WSA_WAIT_TIMEOUT) {
                    WSACloseEvent(socket_ev);
                    return true;
                }

                // Figure out which one it is and call it
                int indexEvent = dw - WSA_WAIT_EVENT_0;
                {
                    QyCritScope cr(&mCrit);
                    for (size_t i = 0; i < mDispatchers.size(); ++i)
                    {
                        uint32 ff = 0;
                        int errcode = 0;

                        QyDispatcher * disp = mDispatchers[i];
                        int dispIndexEvent = disp->indexOfEvent();
                        disp->setEventIndex(-1);

                        if (dispIndexEvent == 0)
                        {
                            SOCKET s = disp->sock();
                            if (s == INVALID_SOCKET)
                                continue;

                            WSANETWORKEVENTS wsaEvents;
                            int err = WSAEnumNetworkEvents(s, events[0], &wsaEvents);
                            if (err == 0)
                            {
                                if (wsaEvents.lNetworkEvents & FD_READ)
                                    ff |= kfRead;
                                if (wsaEvents.lNetworkEvents & FD_WRITE)
                                    ff |= kfWrite;

                                if (wsaEvents.lNetworkEvents & FD_CONNECT) {
                                    if (wsaEvents.iErrorCode[FD_CONNECT_BIT] == 0) {
                                        ff |= kfConnect;
                                    }
                                    else {
                                        ff |= kfClose;
                                        errcode = wsaEvents.iErrorCode[FD_CONNECT_BIT];
                                    }
                                }
                                if (wsaEvents.lNetworkEvents & FD_ACCEPT)
                                    ff |= kfRead;

                                if (wsaEvents.lNetworkEvents & FD_CLOSE) {
                                    ff |= kfClose;
                                    errcode = wsaEvents.iErrorCode[FD_CLOSE_BIT];
                                }
                            }
                            if (ff==0) continue;
                        }
                        else if (dispIndexEvent != indexEvent)
                        {
                            continue;
                        }
                        disp->onPreEvent(ff);
                        disp->onEvent(ff, errcode);

                    }
                    // Reset the network event until new activity occurs
                    WSAResetEvent(socket_ev);
                }

                if (!mfWait)
                    break;

                cmsElapsed = GetMillisecondCount() - msStart;
                if ((cmsWait != kForever) && (cmsElapsed >= cmsWait)) {
                    break;
                }
            }
            WSACloseEvent(socket_ev);
            return true;
        }
#else
        bool _platform(unsigned long cmsWait, bool threadrun)
        {
            HUNUSED(threadrun);
            struct timeval *ptvWait = NULL;
            struct timeval tvWait;
            struct timeval tvStop;

            if (cmsWait != kForever) {
                // Calculate wait timeval
                tvWait.tv_sec = cmsWait / 1000;
                tvWait.tv_usec = (cmsWait % 1000) * 1000;
                ptvWait = &tvWait;

                // Calculate when to return in a timeval
                gettimeofday(&tvStop, NULL);
                tvStop.tv_sec += tvWait.tv_sec;
                tvStop.tv_usec += tvWait.tv_usec;
                if (tvStop.tv_usec >= 1000000) {
                    tvStop.tv_usec -= 1000000;
                    tvStop.tv_sec += 1;
                }
            }

            // Zero all fd_sets. Don't need to do this inside the loop since
            // select() zeros the descriptors not signaled

            fd_set fdsRead;
            FD_ZERO(&fdsRead);
            fd_set fdsWrite;
            FD_ZERO(&fdsWrite);

            mfWait = true;
            while (mfWait) {

                int fdmax = -1;
                {
                    QyCritScope cr(&mCrit);
                    for (unsigned i = 0; i < mDispatchers.size(); i++) {
                        // Query dispatchers for read and write wait state

                        QyDispatcher *disp = mDispatchers[i];

                        int fd = disp->wsaEvent();
                        if (fd > fdmax)
                            fdmax = fd;

                        uint32 ff = disp->requestedEvents();
                        if (ff & kfRead)
                            FD_SET(fd, &fdsRead);

                        if (ff & (kfWrite | kfConnect))
                            FD_SET(fd, &fdsWrite);
                    }
                }

                // Wait then call handlers as appropriate
                // < 0 means error
                // 0 means timeout
                // > 0 means count of descriptors ready
                int n = select(fdmax + 1, &fdsRead, &fdsWrite, NULL, ptvWait);
                // If error, return error
                // todo: do something intelligent
                if (n < 0)
                {
                    return false;
                }
                // If timeout, return success
                if (n == 0)
                {
                    return true;
                }

                {
                    QyCritScope cr(&mCrit);
                    for (size_t i = 0; i < mDispatchers.size(); i++) {
                        QyDispatcher *disp = mDispatchers[i];
                        int fd = disp->wsaEvent();

                        uint32 ff = 0;
                        if (FD_ISSET(fd, &fdsRead)) {
                            FD_CLR(fd, &fdsRead);
                            ff |= kfRead;
                        }
                        if (FD_ISSET(fd, &fdsWrite)) {
                            FD_CLR(fd, &fdsWrite);
                            if (disp->requestedEvents() & kfConnect) {
                                ff |= kfConnect;
                            } else {
                                ff |= kfWrite;
                            }
                        }
                        if (ff != 0) {
                            disp->onPreEvent(ff);
                            disp->onEvent(ff, 0);
                        }
                    }
                }

                // Recalc the time remaining to wait. Doing it here means it doesn't get
                // calced twice the first time through the loop

                if (cmsWait != kForever) {
                    ptvWait->tv_sec = 0;
                    ptvWait->tv_usec = 0;
                    struct timeval tvT;
                    gettimeofday(&tvT, NULL);
                    if (tvStop.tv_sec >= tvT.tv_sec) {
                        ptvWait->tv_sec = tvStop.tv_sec - tvT.tv_sec;
                        ptvWait->tv_usec = tvStop.tv_usec - tvT.tv_usec;
                        if (ptvWait->tv_usec < 0) {
                            ptvWait->tv_usec += 1000000;
                            ptvWait->tv_sec -= 1;
                        }
                    }
                }
            }
            return true;
        }
#endif
        bool mfWait;
        QyCriticalSection mCrit;
        std::vector<QyDispatcher*> mDispatchers;

    };

    QyPhysicalSocketFactory::QyPhysicalSocketFactory(EventMode mode) :
        QySocketFactory(mode),
        d_ptr(new QyPhysicalSocketFactoryPrivate())
    {
	}

    QyPhysicalSocketFactory::~QyPhysicalSocketFactory()
    {
        delete d_ptr;
	}

    QySocket* QyPhysicalSocketFactory::createSocket(int type)
	{
        QyPhysicalSocket* socket = new QyPhysicalSocket();
        if (socket->open(type))
		{
			return socket;
		}
		else
		{
			delete socket;
			return 0;
		}
    }

    QyAsyncSocket* QyPhysicalSocketFactory::createAsyncSocket(int type)
	{
        QySocketDispatcher* socket = new QySocketDispatcher(this);
        if (socket->open(type))
        {
            return socket;
        }
        else
        {
            delete socket;
            return 0;
        }
	}

    bool QyPhysicalSocketFactory::wait(unsigned long cms,bool threadrun)
    {
        return d_ptr->wait(cms,threadrun);
    }

    void QyPhysicalSocketFactory::wake()
    {
        d_ptr->wake();
    }

    void QyPhysicalSocketFactory::add(QyDispatcher* dispatcher)
    {
        QyCritScope scope(&d_ptr->mCrit);
        d_ptr->mDispatchers.push_back(dispatcher);
    }

    void QyPhysicalSocketFactory::remove(QyDispatcher* dispatcher)
    {
        QyCritScope scope(&d_ptr->mCrit);
        d_ptr->mDispatchers.erase(std::remove(d_ptr->mDispatchers.begin(), d_ptr->mDispatchers.end(), dispatcher), d_ptr->mDispatchers.end());
    }
} // namespace qy
