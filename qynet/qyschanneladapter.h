#ifndef __QY_SCHANNELADAPTER_H__
#define __QY_SCHANNELADAPTER_H__

#include <string>
#include "qyssladapter.h"
#include "qymessagequeue.h"
struct _SecBufferDesc;

namespace qy
{
	class SChannelAdapter : public SSLAdapter , public CMessageHandler
	{
	public:
		SChannelAdapter(AsyncSocket* socket);
		virtual ~SChannelAdapter();

		virtual int StartSSL(const char* hostname, bool restartable);
		virtual int Send(const void* pv, size_t cb);
		virtual int Recv(void* pv, size_t cb);
		virtual int Close();

		// Note that the socket returns ST_CONNECTING while SSL is being negotiated.
		virtual ConnState GetState() const;

	protected:
		enum SSLState
		{
			SSL_NONE, SSL_WAIT, SSL_CONNECTING, SSL_CONNECTED, SSL_ERROR
		};
		struct SSLImpl;

		virtual void OnConnectEvent(AsyncSocket* socket);
		virtual void OnReadEvent(AsyncSocket* socket);
		virtual void OnWriteEvent(AsyncSocket* socket);
		virtual void OnCloseEvent(AsyncSocket* socket, int err);
		virtual void OnMessage(Message* pmsg);

		int BeginSSL();
		int ContinueSSL();
		int ProcessContext(long int status, _SecBufferDesc* sbd_in,	_SecBufferDesc* sbd_out);
		int DecryptData();

		int  Read();
		int  Flush();
		void Error(const char* context, int err, bool signal = true);
		void Cleanup();

		void PostEvent();

	private:
		SSLState state_;
		std::string ssl_host_name_;
		// If true, socket will retain SSL configuration after Close.
		bool restartable_; 
		// If true, we are delaying signalling close until all data is read.
		bool signal_close_;
		// If true, we are waiting to be woken up to signal readability or closure.
		bool message_pending_;
		SSLImpl* impl_;
	};

} // namespace qy

#endif // __QY_SCHANNELADAPTER_H__
