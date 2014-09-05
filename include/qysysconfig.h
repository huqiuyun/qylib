#ifndef QY_SYSCONFIG_H__
#define QY_SYSCONFIG_H__


/*
 The operating system, must be one of: (H_OS_x)
 
 DARWIN   - Darwin OS (synonym for H_OS_MAC)
 MAC      - OS X or iOS (synonym for H_OS_DARWIN)
 MACX     - OS X
 IOS      - iOS
 MSDOS    - MS-DOS and Windows
 OS2      - OS/2
 OS2EMX   - XFree86 on OS/2 (not PM)
 WIN32    - Win32 (Windows 2000/XP/Vista/7 and Windows Server 2003/2008)
 WINCE    - WinCE (Windows CE 5.0)
 CYGWIN   - Cygwin
 SOLARIS  - Sun Solaris
 HPUX     - HP-UX
 ULTRIX   - DEC Ultrix
 LINUX    - Linux
 FREEBSD  - FreeBSD
 NETBSD   - NetBSD
 OPENBSD  - OpenBSD
 BSDI     - BSD/OS
 IRIX     - SGI Irix
 OSF      - HP Tru64 UNIX
 SCO      - SCO OpenServer 5
 UNIXWARE - UnixWare 7, Open UNIX 8
 AIX      - AIX
 HURD     - GNU Hurd
 DGUX     - DG/UX
 RELIANT  - Reliant UNIX
 DYNIX    - DYNIX/ptx
 QNX      - QNX
 QNX6     - QNX RTP 6.1
 LYNX     - LynxOS
 BSD4     - Any BSD 4.4 system
 UNIX     - Any UNIX BSD/SYSV system
 ANDROID  - Android platform
 */

#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#  define H_OS_DARWIN
#  define H_OS_BSD4
#  ifdef __LP64__
#    define H_OS_DARWIN64
#  else
#    define H_OS_DARWIN32
#  endif
#elif defined(ANDROID)
#  define H_OS_ANDROID
#  define H_OS_LINUX
#elif defined(__CYGWIN__)
#  define H_OS_CYGWIN
#elif !defined(SAG_COM) && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#  define H_OS_WIN32
#  define H_OS_WIN64
#elif !defined(SAG_COM) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#  if defined(WINCE) || defined(_WIN32_WCE)
#    define H_OS_WINCE
#  else
#    define H_OS_WIN32
#  endif
#elif defined(__sun) || defined(sun)
#  define H_OS_SOLARIS
#elif defined(hpux) || defined(__hpux)
#  define H_OS_HPUX
#elif defined(__ultrix) || defined(ultrix)
#  define H_OS_ULTRIX
#elif defined(sinix)
#  define H_OS_RELIANT
#elif defined(__native_client__)
#  define H_OS_NACL
#elif defined(__linux__) || defined(__linux)
#  define H_OS_LINUX
#elif defined(__FreeBSD__) || defined(__DragonFly__)
#  define H_OS_FREEBSD
#  define H_OS_BSD4
#elif defined(__NetBSD__)
#  define H_OS_NETBSD
#  define H_OS_BSD4
#elif defined(__OpenBSD__)
#  define H_OS_OPENBSD
#  define H_OS_BSD4
#elif defined(__bsdi__)
#  define H_OS_BSDI
#  define H_OS_BSD4
#elif defined(__sgi)
#  define H_OS_IRIX
#elif defined(__osf__)
#  define H_OS_OSF
#elif defined(_AIX)
#  define H_OS_AIX
#elif defined(__Lynx__)
#  define H_OS_LYNX
#elif defined(__GNU__)
#  define H_OS_HURD
#elif defined(__DGUX__)
#  define H_OS_DGUX
#elif defined(__QNXNTO__)
#  define H_OS_QNX
#elif defined(_SEQUENT_)
#  define H_OS_DYNIX
#elif defined(_SCO_DS) /* SCO OpenServer 5 + GCC */
#  define H_OS_SCO
#elif defined(__USLC__) /* all SCO platforms + UDK or OUDK */
#  define H_OS_UNIXWARE
#elif defined(__svr4__) && defined(i386) /* Open UNIX 8 + GCC */
#  define H_OS_UNIXWARE
#elif defined(__INTEGRITY)
#  define H_OS_INTEGRITY
#elif defined(VXWORKS) /* there is no "real" VxWorks define - this has to be set in the mkspec! */
#  define H_OS_VXWORKS
#elif defined(__MAKEDEPEND__)
#else
#  error "Qt has not been ported to this OS - see http://www.qt-project.org/"
#endif

#if defined(H_OS_WIN32) || defined(H_OS_WIN64) || defined(H_OS_WINCE)
#  define H_OS_WIN
#endif

#if defined(H_OS_DARWIN)
#  define H_OS_MAC
#  if defined(H_OS_DARWIN64)
#     define H_OS_MAC64
#  elif defined(H_OS_DARWIN32)
#     define H_OS_MAC32
#  endif
#  include <TargetConditionals.h>
#  if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#     define H_OS_IOS
#  else
#     define H_OS_MACX
#  endif
#endif

#if defined(H_OS_WIN)
#  undef H_OS_UNIX
#elif !defined(H_OS_UNIX)
#  define H_OS_UNIX
#endif

#ifdef H_OS_DARWIN
#  include <Availability.h>
#  if !defined(__MAC_OS_X_VERSION_MIN_REQUIRED) || __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_6
#     undef __MAC_OS_X_VERSION_MIN_REQUIRED
#     define __MAC_OS_X_VERSION_MIN_REQUIRED __MAC_10_6
#  endif
#  include <AvailabilityMacros.h>
#  if !defined(MAC_OS_X_VERSION_MIN_REQUIRED) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_6
#     undef MAC_OS_X_VERSION_MIN_REQUIRED
#     define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_6
#  endif
#
#  // Numerical checks are preferred to named checks, but to be safe
#  // we define the missing version names in case Qt uses them.
#
#  if !defined(__MAC_10_7)
#       define __MAC_10_7 1070
#  endif
#  if !defined(__MAC_10_8)
#       define __MAC_10_8 1080
#  endif
#  if !defined(__MAC_10_9)
#       define __MAC_10_9 1090
#  endif
#  if !defined(MAC_OS_X_VERSION_10_7)
#       define MAC_OS_X_VERSION_10_7 1070
#  endif
#  if !defined(MAC_OS_X_VERSION_10_8)
#       define MAC_OS_X_VERSION_10_8 1080
#  endif
#  if !defined(MAC_OS_X_VERSION_10_9)
#       define MAC_OS_X_VERSION_10_9 1090
#  endif
#
#  if !defined(__IPHONE_4_3)
#       define __IPHONE_4_3 40300
#  endif
#  if !defined(__IPHONE_5_0)
#       define __IPHONE_5_0 50000
#  endif
#  if !defined(__IPHONE_5_1)
#       define __IPHONE_5_1 50100
#  endif
#  if !defined(__IPHONE_6_0)
#       define __IPHONE_6_0 60000
#  endif
#  if !defined(__IPHONE_6_1)
#       define __IPHONE_6_1 60100
#  endif
#  if !defined(__IPHONE_7_0)
#       define __IPHONE_7_0 70000
#  endif
#
#  if (__MAC_OS_X_VERSION_MAX_ALLOWED > __MAC_10_9)
#    warning "This version of OS X is unsupported"
#  endif
#endif

#ifdef __LSB_VERSION__
#  if __LSB_VERSION__ < 40
#    error "This version of the Linux Standard Base is unsupported"
#  endif
#ifndef H_LINUXBASE
#  define H_LINUXBASE
#endif
#endif


#endif  // QY_SYSCONFIG_H__
