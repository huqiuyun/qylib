/*

Copyright (c) 2004-2005, Nash Tsai
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the distribution.
* Neither the name of the author nor the names of its
contributors may be used to endorse or promote products derived
from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef __QY_THREADPOLICY_HXX
#define __QY_THREADPOLICY_HXX

#ifndef _WIN32
#  include <pthread.h>
#  include <cerrno>
#elif defined(HAVE_LIBPTHREAD)
#  include <windows.h>
#  include <pthread.h>
#else
#  include <windows.h>
#endif

#include <cassert>
#include <qyutil/qyconfig.h>

DEFINE_NAMESPACE(qy)

class QyPolicySingleThreaded
{
public:
	QyPolicySingleThreaded()
	{
	}

	~QyPolicySingleThreaded()
	{
	}

	void lock()
	{
	}

	void unlock()
	{
	}
};

class QyPolicyMultiThreaded
{
public:
	QyPolicyMultiThreaded()
	{
#if !defined(_WIN32) || defined(HAVE_LIBPTHREAD)
		int rc = pthread_mutex_init(&mId,0);
		(void)rc;
		assert( rc == 0);
#else
		::InitializeCriticalSection(&mId);
#endif
	}


	~QyPolicyMultiThreaded()
	{
#if !defined(_WIN32) || defined(HAVE_LIBPTHREAD)
		int rc = pthread_mutex_destroy(&mId);
		(void)rc;
		assert( rc != EBUSY);  // currently locked 
		assert( rc == 0);
#else
		::DeleteCriticalSection(&mId);
#endif
	}

	void lock()
	{
#if !defined(_WIN32) || defined(HAVE_LIBPTHREAD)
		int  rc = pthread_mutex_lock(&mId);
		(void)rc;
		assert( rc != EINVAL);
		assert( rc != EDEADLK);
		assert( rc == 0);
#else
		::EnterCriticalSection(&mId);
#endif
	}

	void unlock()
	{
#if !defined(_WIN32) || defined(HAVE_LIBPTHREAD)
		int rc = pthread_mutex_unlock(&mId);
		(void)rc;
		assert( rc != EINVAL);
		assert( rc != EPERM);
		assert( rc == 0);
#else
		::LeaveCriticalSection(&mId);
#endif
	}

private:
#if !defined(_WIN32) || defined(HAVE_LIBPTHREAD)
	mutable pthread_mutex_t mId;
#else
	CRITICAL_SECTION mId;
#endif
};

END_NAMESPACE(qy)

#endif // #ifndef __QY_THREADPOLICY_HXX

