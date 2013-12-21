#ifndef __QY_TIME_H__
#define __QY_TIME_H__

#include <iostream>
#include <cstdlib>
#include <cstring>
#include "qyutil/qyconfig.h"
#include "qyutil/qyctypes.h"

#ifdef POSIX
#include <sys/time.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

DEFINE_NAMESPACE(qy)

class QYUTIL_API QyTimer
{
public:
    QyTimer();
    
    void reset();
    void start();
    uint32 stop();
    uint32 getTime() const {return time_;}
    
protected:
    int64 getCount();
    
private:
    int64 freq_;
    int64 start_;
    uint32  time_;
};


#ifdef POSIX
inline quint32 qytime_get()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
#endif

#ifdef WIN32

inline uint32 qytime_get()
{
    return GetTickCount();
}
#endif

inline uint32 qytime_start()
{
    // Close to program execution time
    static const uint32 g_start = qytime_get();
    return g_start;
}

inline bool qytime_isBetween(uint32 later, uint32 middle, uint32 earlier)
{
    if (earlier <= later) {
        return ((earlier <= middle) && (middle <= later));
    } else {
        return !((later < middle) && (middle < earlier));
    }
}

inline int32 qytime_diff(uint32 later, uint32 earlier)
{
    uint32 LAST = 0xFFFFFFFF;
    uint32 HALF = 0x80000000;
    if (qytime_isBetween(earlier + HALF, later, earlier)) {
        if (earlier <= later) {
            return static_cast<long>(later - earlier);
        } else {
            return static_cast<long>(later + (LAST - earlier) + 1);
        }
    } else {
        if (later <= earlier) {
            return -static_cast<long>(earlier - later);
        } else {
            return -static_cast<long>(earlier + (LAST - later) + 1);
        }
    }
}

inline uint32 qytime_elapsed()
{
    return qytime_diff(qytime_get(), qytime_start());
}

// TODO: Delete this old version.
#define GetMillisecondCount qytime_get

END_NAMESPACE(qy)

#endif /* __QY_TIME_H__ */