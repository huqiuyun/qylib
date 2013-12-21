#ifndef QY_CTYPES_H__
#define QY_CTYPES_H__

#if !defined(_MSC_VER)
#include <stdint.h>
typedef unsigned long       ulong_t;
#else
typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef __int64             int64_t;

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned __int64    uint64_t;
typedef unsigned long       ulong_t;
#endif//

typedef int64_t             int64;
typedef int32_t             int32;
typedef int16_t             int16;
typedef int8_t              int8;

typedef uint64_t            uint64;

typedef uint32_t            uint;
typedef uint32_t            uint32;

typedef ulong_t             ulong;

typedef uint16_t            ushort;
typedef uint16_t            uint16;

typedef uint8_t             uchar;
typedef uint8_t             uint8;

typedef long long           longlong;
typedef unsigned long long  ulonglong;

//
#define MIN_(a, b) ((a) > (b)) ? (b) : (a)
#define MAX_(a,b)  ((a) < (b)) ? (b) : (a)

#endif //QY_CTYPES_H__
