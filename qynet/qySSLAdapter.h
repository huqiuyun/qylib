#ifndef QY_SSLAdapter_H__
#define QY_SSLAdapter_H__

#include "qyAsyncSocket.h"

namespace qy 
{
    class QySSLAdapter : public QyAsyncSocketAdapter
	{
	public:
        QySSLAdapter(QyAsyncSocket* socket)
            : QyAsyncSocketAdapter(socket), ignore_bad_cert_(false)
		{ }

		bool ignore_bad_cert() const { return ignore_bad_cert_; }
		void set_ignore_bad_cert(bool ignore) { ignore_bad_cert_ = ignore; }

		// StartSSL returns 0 if successful.
		// If StartSSL is called while the socket is closed or connecting, the SSL
		// negotiation will begin as soon as the socket connects.
        virtual int startSSL(const char* hostname, bool restartable) = 0;

		// Create the default SSL adapter for this platform
        static QySSLAdapter* create(QyAsyncSocket* socket);

	private:
		// If true, the server certificate need not match the configured hostname.
		bool ignore_bad_cert_;
	};

    // modified by huqiuyun 2008-01-22
#ifdef __cplusplus
    extern "C"
    {
#endif

        // Call this on the main thread, before using SSL.
        // Call CleanupSSLThread when finished with SSL.
        bool InitializeSSL();

        // Call to initialize additional threads.
        bool InitializeSSLThread();

        // Call to cleanup additional threads, and also the main thread.
        bool CleanupSSL();

#ifdef __cplusplus
    }
#endif

}  // namespace qy

#endif // QY_SSLAdapter_H__
