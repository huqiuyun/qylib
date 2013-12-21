#ifndef QY_NETBASICTYPES_H__
#define QY_NETBASICTYPES_H__

#include "qysysconfig.h"
#include "qyctypes.h"

#ifdef H_OS_WIN
typedef int socklen_t;
#endif

template<class T> inline T _min(T a, T b) { return (a > b) ? b : a; }
template<class T> inline T _max(T a, T b) { return (a < b) ? b : a; }

// A macro to disallow the evil copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_EVIL_CONSTRUCTORS(TypeName)    \
  TypeName(const TypeName&);                    \
  void operator=(const TypeName&)

// A macro to disallow all the implicit constructors, namely the
// default constructor, copy constructor and operator= functions.
//
// This should be used in the private: declarations for a class
// that wants to prevent anyone from instantiating it. This is
// especially useful for classes containing only static methods.
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) \
  TypeName();                                    \
  DISALLOW_EVIL_CONSTRUCTORS(TypeName)

#ifndef HUNUSED
#define HUNUSED(x) Unused(static_cast<const void *>(&x))
#define HUNUSED2(x,y) Unused(static_cast<const void *>(&x)); Unused(static_cast<const void *>(&y))
#define HUNUSED3(x,y,z) Unused(static_cast<const void *>(&x)); Unused(static_cast<const void *>(&y)); Unused(static_cast<const void *>(&z))
#define HUNUSED4(x,y,z,a) Unused(static_cast<const void *>(&x)); Unused(static_cast<const void *>(&y)); Unused(static_cast<const void *>(&z)); Unused(static_cast<const void *>(&a))
#define HUNUSED5(x,y,z,a,b) Unused(static_cast<const void *>(&x)); Unused(static_cast<const void *>(&y)); Unused(static_cast<const void *>(&z)); Unused(static_cast<const void *>(&a)); Unused(static_cast<const void *>(&b))
inline void Unused(const void *) { }
#endif // UNUSED

#define ARRAY_SIZE(x) (static_cast<int>((sizeof(x)/sizeof(x[0]))))

#endif // QY_NETBASICTYPES_H__
