#ifndef QY_TIME_H__
#define QY_TIME_H__

#include "qysysconfig.h"
#include "qyctypes.h"

#ifdef H_OS_WIN
#include <windows.h>
#else
#include <sys/time.h>
#endif

namespace qy {

#ifndef H_OS_WIN
inline uint32 qytime_get()
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
#else

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

inline long qytime_diff(uint32 later, uint32 earlier)
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

} // namespace qy

#endif //QY_TIME_H__
