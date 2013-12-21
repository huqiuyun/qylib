#ifndef QY_DEBUG_H__
#define QY_DEBUG_H__

#include "qysysconfig.h"

#if defined(H_OS_WIN)
   #include <windows.h>
   #include <cstdio>
#else

#endif
/////////////////////////////////////////////////////////////////////////////
// Assertions
/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// Break causes the debugger to stop executing, or the program to abort
inline void _Break()
{
#ifdef H_OS_WIN
	::DebugBreak();
#else // !H_OS_WIN

#endif // !H_OS_WIN
}

// LogAssert writes information about an assertion to the log

inline void LogAssert(const char * function, const char * file, int line, const char * expression)
{
  // TODO - if we put hooks in here, we can do a lot fancier logging
  fprintf(stderr, "%s(%d): %s @ %s\n", file, line, expression, function);
}

inline void Assert(bool result, const char * function, const char * file, int line, const char * expression)
{
	if (!result)
	{
		LogAssert(function, file, line, expression);
		_Break();
	}
}

#ifndef ASSERT
#define ASSERT(x) Assert((x),__FUNCTION__,__FILE__,__LINE__,#x)
#endif

#ifndef VERIFY
#define VERIFY(x) Assert((x),__FUNCTION__,__FILE__,__LINE__,#x)
#endif

#else // !_DEBUG

#ifndef ASSERT
#define ASSERT(x) (void)0
#endif

#ifndef VERIFY
#define VERIFY(x) (void)(x)
#endif

#endif // !_DEBUG

#endif // __QY_DEBUG_H__
