#pragma once

#include <assert.h>

#if defined(_DEBUG)
	#define QY_ASSERT(exp) assert(exp)
	#define QY_ASSERT_X(exp, where, what) assert(exp)
#else
	#define QY_ASSERT(exp) ;
	#define QY_ASSERT_X(exp, where, what) ;
#endif //!_DEBUG

