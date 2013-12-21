#ifndef QY_PROXYINFO_H__
#define QY_PROXYINFO_H__

#include <string>
#include "qycryptstring.h"
#include "qySocketAddress.h"

namespace qy
{
	enum ProxyType { PROXY_NONE, PROXY_HTTPS, PROXY_SOCKS5, PROXY_UNKNOWN };
	enum ProtocolType { PROTO_UDP, PROTO_TCP, PROTO_SSLTCP, PROTO_LAST = PROTO_SSLTCP };

    class QyProxyInfo
	{
    public:
        QyProxyInfo()
			: type(PROXY_NONE)
		{ }

		static const char* ProxyToString(ProxyType proxy)
		{
			const char * const PROXY_NAMES[] = { "none", "https", "socks5", "unknown" };
			return PROXY_NAMES[proxy];
        }

        ProxyType type;
        QySocketAddress address;
        std::string   username;
        QyCryptString password;
	};
} // namespace qy

#endif // QY_PROXYINFO_H__
