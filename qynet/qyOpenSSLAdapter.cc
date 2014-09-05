#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>

#include "qydebug.h"
#include "qyOpenSSLAdapter.h"
#include "qystringutils.h"
#include "Equifax_Secure_Global_eBusiness_CA-1.h"
#include <string>

//////////////////////////////////////////////////////////////////////
// SocketBIO
//////////////////////////////////////////////////////////////////////

static int socket_write(BIO* h, const char* buf, int num);
static int socket_read(BIO* h, char* buf, int size);
static int socket_puts(BIO* h, const char* str);
static long socket_ctrl(BIO* h, int cmd, long arg1, void* arg2);
static int socket_new(BIO* h);
static int socket_free(BIO* data);

static BIO_METHOD methods_socket =
{
    BIO_TYPE_BIO,
    "socket",
    socket_write,
    socket_read,
    socket_puts,
    0,
    socket_ctrl,
    socket_new,
    socket_free,
    NULL,
};

BIO_METHOD* BIO_s_socket2() { return(&methods_socket); }

BIO* BIO_new_socket(qy::QyAsyncSocket* socket)
{
    BIO* ret = BIO_new(BIO_s_socket2());
    if (ret == NULL) {
        return NULL;
    }
    ret->ptr = socket;
    return ret;
}

static int socket_new(BIO* b) 
{
    b->shutdown = 0;
    b->init = 1;
    b->num = 0; // 1 means socket closed
    b->ptr = 0;
    return 1;
}

static int socket_free(BIO* b)
{
    if (b == NULL)
        return 0;
    return 1;
}

static int socket_read(BIO* b, char* out, int outl)
{
    if (!out)
        return -1;
    qy::QyAsyncSocket* socket = static_cast<qy::QyAsyncSocket*>(b->ptr);
    BIO_clear_retry_flags(b);
    int result = socket->recv(out, outl);
    if (result > 0) {
        return result;
    } else if (result == 0) {
        b->num = 1;
    } else if (socket->isBlocking()) {
        BIO_set_retry_read(b);
    }
    return -1;
}

static int socket_write(BIO* b, const char* in, int inl)
{
    if (!in)
        return -1;
    qy::QyAsyncSocket* socket = static_cast<qy::QyAsyncSocket*>(b->ptr);
    BIO_clear_retry_flags(b);
    int result = socket->send(in, inl);
    if (result > 0) {
        return result;
    } else if (socket->isBlocking()) {
        BIO_set_retry_write(b);
    }
    return -1;
}

static int socket_puts(BIO* b, const char* str)
{
    return socket_write(b, str, strlen(str));
}

static long socket_ctrl(BIO* b, int cmd, long num, void* ptr)
{
    HUNUSED2(num,ptr);

    switch (cmd) {
    case BIO_CTRL_RESET:
        return 0;
    case BIO_CTRL_EOF:
        return b->num;
    case BIO_CTRL_WPENDING:
    case BIO_CTRL_PENDING:
        return 0;
    case BIO_CTRL_FLUSH:
        return 1;
    default:
        return 0;
    }
}

/////////////////////////////////////////////////////////////////////////////
// QyOpenSSLAdapter
/////////////////////////////////////////////////////////////////////////////

namespace qy {
class QyOpenSSLAdapterPrivate
{
public:
    QyOpenSSLAdapterPrivate() :
        mState(QyOpenSSLAdapter::SSL_NONE),
        mSsl_write_needs_write(false),
        mSsl_write_needs_read(false),
        mRestartable(false),
        mSsl(NULL), mSslCtx(NULL)
    {

    }

    QyOpenSSLAdapter::SSLState mState;
    bool mSsl_write_needs_write;
    bool mSsl_write_needs_read;
    // If true, socket will retain SSL configuration after Close.
    bool mRestartable;

    SSL* mSsl;
    SSL_CTX* mSslCtx;
    std::string mSslHostname;
};

QyOpenSSLAdapter::QyOpenSSLAdapter(QyAsyncSocket* socket):
    QySSLAdapter(socket),
    d_ptr(new QyOpenSSLAdapterPrivate())
{
}

QyOpenSSLAdapter::~QyOpenSSLAdapter() {
    cleanup();
    delete d_ptr;
}

int QyOpenSSLAdapter::startSSL(const char* hostname, bool restartable) {
    if (d_ptr->mState != SSL_NONE)
        return -1;

    d_ptr->mSslHostname = hostname;
    d_ptr->mRestartable = restartable;

    if (mSocket->connState() != QySocket::CS_CONNECTED) {
        d_ptr->mState = SSL_WAIT;
        return 0;
    }

    d_ptr->mState = SSL_CONNECTING;
    if (int err = beginSSL()) {
        setErrorWithContext("BeginSSL", err, false);
        return err;
    }

    return 0;
}

int
QyOpenSSLAdapter::beginSSL() {
    ASSERT(d_ptr->mState == SSL_CONNECTING);

    int err = 0;
    BIO* bio = NULL;

    // First set up the context
    if (!d_ptr->mSslCtx)
        d_ptr->mSslCtx = setupSSLContext();

    if (!d_ptr->mSslCtx) {
        err = -1;
        goto ssl_error;
    }

    bio = BIO_new_socket(static_cast<qy::QyAsyncSocketAdapter*>(mSocket));
    if (!bio) {
        err = -1;
        goto ssl_error;
    }

    d_ptr->mSsl = SSL_new(d_ptr->mSslCtx);
    if (!d_ptr->mSsl) {
        err = -1;
        goto ssl_error;
    }

    SSL_set_app_data(d_ptr->mSsl, this);

    SSL_set_bio(d_ptr->mSsl, bio, bio);
    SSL_set_mode(d_ptr->mSsl, SSL_MODE_ENABLE_PARTIAL_WRITE |
                 SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

    // the SSL object owns the bio now
    bio = NULL;

    // Do the connect
    err = continueSSL();
    if (err != 0)
        goto ssl_error;

    return err;

ssl_error:
    cleanup();
    if (bio)
        BIO_free(bio);

    return err;
}

int QyOpenSSLAdapter::continueSSL() {
    ASSERT(d_ptr->mState == SSL_CONNECTING);

    int code = SSL_connect(d_ptr->mSsl);
    switch (SSL_get_error(d_ptr->mSsl, code)) {
    case SSL_ERROR_NONE:

        if (!SSLPostConnectionCheck(d_ptr->mSsl, d_ptr->mSslHostname.c_str())) {
            // make sure we close the socket
            cleanup();
            // The connect failed so return -1 to shut down the socket
            return -1;
        }

        d_ptr->mState = SSL_CONNECTED;
        QyAsyncSocketAdapter::onConnectEvent(this);

        break;

    case SSL_ERROR_WANT_READ:
        break;

    case SSL_ERROR_WANT_WRITE:
        break;

    case SSL_ERROR_ZERO_RETURN:
    default:
        return (code != 0) ? code : -1;
    }

    return 0;
}

void QyOpenSSLAdapter::setErrorWithContext(const char* context, int err, bool signal) {
    HUNUSED(context);
    d_ptr->mState = SSL_ERROR;
    setError(err);
    if (signal)
        QyAsyncSocketAdapter::onCloseEvent(this, err);
}

void QyOpenSSLAdapter::cleanup() {

    d_ptr->mState = SSL_NONE;
    d_ptr->mSsl_write_needs_write = false;
    d_ptr->mSsl_write_needs_read = false;

    if (d_ptr->mSsl) {
        SSL_free(d_ptr->mSsl);
        d_ptr->mSsl = NULL;
    }

    if (d_ptr->mSslCtx) {
        SSL_CTX_free(d_ptr->mSslCtx);
        d_ptr->mSslCtx = NULL;
    }
}

int QyOpenSSLAdapter::send(const void* pv, size_t cb) {

    switch (d_ptr->mState) {
    case SSL_NONE:
        return QyAsyncSocketAdapter::send(pv, cb);

    case SSL_WAIT:
    case SSL_CONNECTING:
        setError(EWOULDBLOCK);
        return SOCKET_ERROR;

    case SSL_CONNECTED:
        break;

    case SSL_ERROR:
    default:
        return SOCKET_ERROR;
    }

    // OpenSSL will return an error if we try to write zero bytes
    if (cb == 0)
        return 0;

    d_ptr->mSsl_write_needs_read = false;

    int code = SSL_write(d_ptr->mSsl, pv, cb);
    switch (SSL_get_error(d_ptr->mSsl, code)) {
    case SSL_ERROR_NONE:
        return code;
    case SSL_ERROR_WANT_READ:
        d_ptr->mSsl_write_needs_read = true;
        setError(EWOULDBLOCK);
        break;
    case SSL_ERROR_WANT_WRITE:

        setError(EWOULDBLOCK);
        break;
    case SSL_ERROR_ZERO_RETURN:

        setError(EWOULDBLOCK);
        // do we need to signal closure?
        break;
    default:
        setErrorWithContext("SSL_write", (code ? code : -1), false);
        break;
    }

    return SOCKET_ERROR;
}

int QyOpenSSLAdapter::recv(void* pv, size_t cb) {
    switch (d_ptr->mState) {
    case SSL_NONE:
        return QyAsyncSocketAdapter::recv(pv, cb);

    case SSL_WAIT:
    case SSL_CONNECTING:
        setError(EWOULDBLOCK);
        return SOCKET_ERROR;

    case SSL_CONNECTED:
        break;

    case SSL_ERROR:
    default:
        return SOCKET_ERROR;
    }

    // Don't trust OpenSSL with zero byte reads
    if (cb == 0)
        return 0;

    d_ptr->mSsl_write_needs_write = false;

    int code = SSL_read(d_ptr->mSsl, pv, cb);
    switch (SSL_get_error(d_ptr->mSsl, code)) {
    case SSL_ERROR_NONE:
        return code;
    case SSL_ERROR_WANT_READ:
        setError(EWOULDBLOCK);
        break;
    case SSL_ERROR_WANT_WRITE:
        d_ptr->mSsl_write_needs_write = true;
        setError(EWOULDBLOCK);
        break;
    case SSL_ERROR_ZERO_RETURN:
        setError(EWOULDBLOCK);
        // do we need to signal closure?
        break;
    default:
        setErrorWithContext("SSL_read", (code ? code : -1), false);
        break;
    }
    return SOCKET_ERROR;
}

int QyOpenSSLAdapter::close() {
    cleanup();
    d_ptr->mState = d_ptr->mRestartable ? SSL_WAIT : SSL_NONE;
    return QyAsyncSocketAdapter::close();
}

QySocket::ConnState QyOpenSSLAdapter::state() const {

    ConnState state = mSocket->connState();
    if ((state == CS_CONNECTED)
            && ((d_ptr->mState == SSL_WAIT) || (d_ptr->mState == SSL_CONNECTING)))
        state = CS_CONNECTING;
    return state;
}

void QyOpenSSLAdapter::onConnectEvent(QyAsyncSocket* socket) {
    if (d_ptr->mState != SSL_WAIT) {
        ASSERT(d_ptr->mState == SSL_NONE);
        QyAsyncSocketAdapter::onConnectEvent(socket);
        return;
    }
    d_ptr->mState = SSL_CONNECTING;
    if (int err = beginSSL()) {
        QyAsyncSocketAdapter::onCloseEvent(socket, err);
    }
}

void QyOpenSSLAdapter::onReadEvent(QyAsyncSocket* socket) {

    if (d_ptr->mState == SSL_NONE) {
        QyAsyncSocketAdapter::onReadEvent(socket);
        return;
    }

    if (d_ptr->mState == SSL_CONNECTING) {
        if (int err = continueSSL()) {
            setErrorWithContext("ContinueSSL", err);
        }
        return;
    }

    if (d_ptr->mState != SSL_CONNECTED)
        return;

    // Don't let ourselves go away during the callbacks
    if (d_ptr->mSsl_write_needs_read)  {
        QyAsyncSocketAdapter::onWriteEvent(socket);
    }
    QyAsyncSocketAdapter::onReadEvent(socket);
}

void QyOpenSSLAdapter::onWriteEvent(QyAsyncSocket* socket) {
    if (d_ptr->mState == SSL_NONE) {
        QyAsyncSocketAdapter::onWriteEvent(socket);
        return;
    }

    if (d_ptr->mState == SSL_CONNECTING) {
        if (int err = continueSSL()) {
            setErrorWithContext("ContinueSSL", err);
        }
        return;
    }

    if (d_ptr->mState != SSL_CONNECTED)
        return;

    if (d_ptr->mSsl_write_needs_write)  {
        QyAsyncSocketAdapter::onReadEvent(socket);
    }

    QyAsyncSocketAdapter::onWriteEvent(socket);
}

void QyOpenSSLAdapter::onCloseEvent(QyAsyncSocket* socket, int err) {
    QyAsyncSocketAdapter::onCloseEvent(socket, err);
}

// This code is taken from the "Network Security with OpenSSL"
// sample in chapter 5
bool QyOpenSSLAdapter::SSLPostConnectionCheck(SSL* ssl, const char* host) {
    if (!host)
        return false;

    // Checking the return from SSL_get_peer_certificate here is not strictly
    // necessary.  With our setup, it is not possible for it to return
    // NULL.  However, it is good form to check the return.
    X509* certificate = SSL_get_peer_certificate(ssl);
    if (!certificate)
        return false;

    bool ok = false;
    int extension_count = X509_get_ext_count(certificate);
    for (int i = 0; i < extension_count; ++i) {
        X509_EXTENSION* extension = X509_get_ext(certificate, i);
        int extension_nid = OBJ_obj2nid(X509_EXTENSION_get_object(extension));

        if (extension_nid == NID_subject_alt_name) {
            X509V3_EXT_METHOD* meth = X509V3_EXT_get(extension);
            if (!meth)
                break;

            void* ext_str = NULL;

#if OPENSSL_VERSION_NUMBER >= 0x0090800fL
            const unsigned char **ext_value_data = (const_cast<const unsigned char **>
                                                    (&extension->value->data));
#else
            unsigned char **ext_value_data = &extension->value->data;
#endif

            if (meth->it) {
                ext_str = ASN1_item_d2i(NULL, ext_value_data, extension->value->length,
                                        ASN1_ITEM_ptr(meth->it));
            } else {
                ext_str = meth->d2i(NULL, ext_value_data, extension->value->length);
            }

            STACK_OF(CONF_VALUE)* value = meth->i2v(meth, ext_str, NULL);
            for (int j = 0; j < sk_CONF_VALUE_num(value); ++j) {
                CONF_VALUE* nval = sk_CONF_VALUE_value(value, j);
                if (!strcmp(nval->name, "DNS") && !strcmp(nval->value, host)) {
                    ok = true;
                    break;
                }
            }
        }
        if (ok)
            break;
    }

    char data[256];
    X509_name_st* subject;
    if (!ok
            && (subject = X509_get_subject_name(certificate))
            && (X509_NAME_get_text_by_NID(subject, NID_commonName,
                                          data, sizeof(data)) > 0)) {
        data[sizeof(data)-1] = 0;
        if (_stricmp(data, host) == 0)
            ok = true;
    }

    X509_free(certificate);

    if (!ok && ignore_bad_cert()) {
        ok = true;
    }

    if (ok)
        ok = (SSL_get_verify_result(ssl) == X509_V_OK);

    if (!ok && ignore_bad_cert()) {
        ok = true;
    }

    return ok;
}

int QyOpenSSLAdapter::SSLVerifyCallback(int ok, X509_STORE_CTX* store) {

    // Get our stream pointer from the store
    SSL* ssl = reinterpret_cast<SSL*>(
                X509_STORE_CTX_get_ex_data(store,
                                           SSL_get_ex_data_X509_STORE_CTX_idx()));

    QyOpenSSLAdapter* stream =
            reinterpret_cast<QyOpenSSLAdapter*>(SSL_get_app_data(ssl));

    if (!ok && stream->ignore_bad_cert()) {
        ok = 1;
    }

    return ok;
}

SSL_CTX* QyOpenSSLAdapter::setupSSLContext() {
    SSL_CTX* ctx = SSL_CTX_new(TLSv1_client_method());
    if (ctx == NULL)
        return NULL;

    // Add the root cert to the SSL context
#if OPENSSL_VERSION_NUMBER >= 0x0090800fL
    const unsigned char* cert_buffer
        #else
    unsigned char* cert_buffer
        #endif
            = EquifaxSecureGlobalEBusinessCA1_certificate;
    size_t cert_buffer_len = sizeof(EquifaxSecureGlobalEBusinessCA1_certificate);
    X509* cert = d2i_X509(NULL, &cert_buffer, cert_buffer_len);
    if (cert == NULL) {
        SSL_CTX_free(ctx);
        return NULL;
    }
    if (!X509_STORE_add_cert(SSL_CTX_get_cert_store(ctx), cert)) {
        X509_free(cert);
        SSL_CTX_free(ctx);
        return NULL;
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, SSLVerifyCallback);
    SSL_CTX_set_verify_depth(ctx, 4);
    SSL_CTX_set_cipher_list(ctx, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");

    return ctx;
}

} // namespace qy
