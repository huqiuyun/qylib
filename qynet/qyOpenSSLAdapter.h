#ifndef QY_OPENSSLAdapter_H__
#define QY_OPENSSLAdapter_H__

#include "qySSLAdapter.h"

typedef struct ssl_st SSL;
typedef struct ssl_ctx_st SSL_CTX;
typedef struct x509_store_ctx_st X509_STORE_CTX;

namespace qy {

class QyOpenSSLAdapterPrivate;
class QyOpenSSLAdapter : public QySSLAdapter
{
public:
    QyOpenSSLAdapter(QyAsyncSocket* socket);
    virtual ~QyOpenSSLAdapter();

    virtual int startSSL(const char* hostname, bool restartable);
    virtual int send(const void* pv, size_t cb);
    virtual int recv(void* pv, size_t cb);
    virtual int close();

    // Note that the socket returns ST_CONNECTING while SSL is being negotiated.
    virtual ConnState state() const;

protected:
    virtual void onConnectEvent(QyAsyncSocket* socket);
    virtual void onReadEvent(QyAsyncSocket* socket);
    virtual void onWriteEvent(QyAsyncSocket* socket);
    virtual void onCloseEvent(QyAsyncSocket* socket, int err);

private:
    enum SSLState {
        SSL_NONE, SSL_WAIT, SSL_CONNECTING, SSL_CONNECTED, SSL_ERROR
    };

    int  beginSSL();
    int  continueSSL();
    void setErrorWithContext(const char* context, int err, bool signal = true);
    void cleanup();

    bool SSLPostConnectionCheck(SSL* ssl, const char* host);
    static int SSLVerifyCallback(int ok, X509_STORE_CTX* store);

    static SSL_CTX* setupSSLContext();

private:
    friend class QyOpenSSLAdapterPrivate;
    QyOpenSSLAdapterPrivate* d_ptr;
};

} // namespace qy

#endif // QY_OPENSSLAdapter_H__
