#include "qySSLAdapter.h"
#include "qysysconfig.h"

#if !defined(SSL_USE_SCHANNEL) && !defined(SSL_USE_OPENSSL)
   #ifdef H_OS_WIN
      #define SSL_USE_SCHANNEL 1
   #else
      #define SSL_USE_OPENSSL 1
   #endif
#endif

#if SSL_USE_SCHANNEL
#include "qyschanneladapter.h"
#endif  // SSL_USE_SCHANNEL

#if SSL_USE_OPENSSL
#include <openssl/crypto.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include "qyopenSSLAdapter.h"

#ifdef H_OS_WIN
#define MUTEX_TYPE HANDLE
#define MUTEX_SETUP(x) (x) = CreateMutex(NULL, FALSE, NULL)
#define MUTEX_CLEANUP(x) CloseHandle(x)
#define MUTEX_LOCK(x) WaitForSingleObject((x), INFINITE)
#define MUTEX_UNLOCK(x) ReleaseMutex(x)
#define THREAD_ID GetCurrentThreadId()
#else
#define MUTEX_TYPE pthread_mutex_t
#define MUTEX_SETUP(x) pthread_mutex_init(&(x), NULL)
#define MUTEX_CLEANUP(x) pthread_mutex_destroy(&(x))
#define MUTEX_LOCK(x) pthread_mutex_lock(&(x))
#define MUTEX_UNLOCK(x) pthread_mutex_unlock(&(x))
#define THREAD_ID pthread_self()
#endif

struct CRYPTO_dynlock_value
{
	MUTEX_TYPE mutex;
};

#endif  // SSL_USE_OPENSSL


namespace qy 
{
#if SSL_USE_SCHANNEL
    typedef SChannelAdapter DefaultQySSLAdapter;
#endif

#if SSL_USE_OPENSSL
    typedef QyOpenSSLAdapter DefaultQySSLAdapter;
#endif

    QySSLAdapter* QySSLAdapter::create(QyAsyncSocket* socket)
	{
            return new DefaultQySSLAdapter(socket);
	}

#if SSL_USE_OPENSSL

	// This array will store all of the mutexes available to OpenSSL.
	static MUTEX_TYPE* mutex_buf = NULL;

	static void locking_function(int mode, int n, const char * file, int line) {
        HUNUSED2(file,line);
		if (mode & CRYPTO_LOCK) {
			MUTEX_LOCK(mutex_buf[n]);
		} else {
			MUTEX_UNLOCK(mutex_buf[n]);
		}
	}

	static pthread_t id_function() {
		return THREAD_ID;
	}

	static CRYPTO_dynlock_value* dyn_create_function(const char* file, int line) {
        HUNUSED2(file,line);
		CRYPTO_dynlock_value* value = new CRYPTO_dynlock_value;
		if (!value)
			return NULL;
		MUTEX_SETUP(value->mutex);
		return value;
	}

	static void dyn_lock_function(int mode, CRYPTO_dynlock_value* l,
		const char* file, int line) {
        HUNUSED2(file,line);
        if (mode & CRYPTO_LOCK) {
            MUTEX_LOCK(l->mutex);
        } else {
            MUTEX_UNLOCK(l->mutex);
        }
	}

	static void dyn_destroy_function(CRYPTO_dynlock_value* l,
		const char* file, int line) {
        HUNUSED2(file,line);
        MUTEX_CLEANUP(l->mutex);
        delete l;
	}

	extern "C"
	{
		bool InitializeSSL() {
			if (!InitializeSSLThread() || !SSL_library_init())
				return false;
			SSL_load_error_strings();
			ERR_load_BIO_strings();
			OpenSSL_add_all_algorithms();
			RAND_poll();
			return true;
		}

		bool InitializeSSLThread() {
			mutex_buf = new MUTEX_TYPE[CRYPTO_num_locks()];
			if (!mutex_buf)
				return false;
			for (int i = 0; i < CRYPTO_num_locks(); ++i)
				MUTEX_SETUP(mutex_buf[i]);

			// we need to cast our id_function to return an unsigned long -- pthread_t is a pointer
			CRYPTO_set_id_callback((unsigned long (*)())id_function);
			CRYPTO_set_locking_callback(locking_function);
			CRYPTO_set_dynlock_create_callback(dyn_create_function);
			CRYPTO_set_dynlock_lock_callback(dyn_lock_function);
			CRYPTO_set_dynlock_destroy_callback(dyn_destroy_function);
			return true;
		}

		bool CleanupSSL() {
			if (!mutex_buf)
				return false;
			CRYPTO_set_id_callback(NULL);
			CRYPTO_set_locking_callback(NULL);
			CRYPTO_set_dynlock_create_callback(NULL);
			CRYPTO_set_dynlock_lock_callback(NULL);
			CRYPTO_set_dynlock_destroy_callback(NULL);
			for (int i = 0; i < CRYPTO_num_locks(); ++i)
				MUTEX_CLEANUP(mutex_buf[i]);
			delete [] mutex_buf;
			mutex_buf = NULL;
			return true;
		}
	}

#else  // !SSL_USE_OPENSSL

    extern "C"
    {
        bool InitializeSSL()
        {
            return true;
        }

        bool InitializeSSLThread()
        {
            return true;
        }

        bool CleanupSSL()
        {
            return true;
        }
    }

#endif  // !SSL_USE_OPENSSL

}  // namespace qy
