#ifndef __QY_OS_H__
#define __QY_OS_H__

#include "qyutil/qyconfig.h"

DEFINE_NAMESPACE(qy)

template <class T>
qyINLINE const T &
qyos_MIN (const T &t1, const T &t2)
{
    return t2 > t1 ? t1 : t2;
}

template <class T>
qyINLINE const T &
qyos_MAX (const T &t1, const T &t2)
{
    return t1 > t2 ? t1 : t2;
}

template <class T>
qyINLINE const T &
qyos_MIN (const T &t1, const T &t2, const T &t3)
{
    return qyos_MIN(qyos_MIN (t1, t2), t3);
}

template <class T>
qyINLINE const T &
qyos_MAX (const T &t1, const T &t2, const T &t3)
{
    return qyos_MAX(qyos_MAX (t1, t2), t3);
}

template <class T>
qyINLINE const T &
qyos_RANGE (const T &min, const T &max, const T &val)
{
    return qyos_MIN (qyos_MAX (min, val), max);
}

qyINLINE int qyos_hiword(unsigned long d)
{
    return (int)((d >> 16) & 0xFFFF);
}

qyINLINE int qyos_loword(unsigned long d)
{
    return (int)(d & 0xFFFF);
}

extern QYUTIL_API
void qyos_sleep(int msecs);

END_NAMESPACE(qy)

#endif  /* __QY_OS_H__ */
