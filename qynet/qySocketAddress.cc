#include "qybyteorder.h"
#include "qySocketAddress.h"
#include "qydebug.h"

#ifndef H_OS_WIN
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <cstring>
#include <sstream>

#ifdef H_OS_WIN
int inet_aton(const char * cp, struct in_addr * inp)
{
	inp->s_addr = inet_addr(cp);
	return (inp->s_addr == INADDR_NONE) ? 0 : 1;
}
#endif // H_OS_WIN

#ifdef _DEBUG
#define DISABLE_DNS 0
#else // !_DEBUG
#define DISABLE_DNS 0
#endif // !_DEBUG

namespace qy 
{

    QySocketAddress::QySocketAddress()
	{
        clear();
	}

    QySocketAddress::QySocketAddress(const std::string& hostname, int port, bool use_dns)
	{
        setIP(hostname, use_dns);
        setSockPort(port);
	}

    QySocketAddress::QySocketAddress(uint32 ip, int port)
	{
        setIP(ip);
        setSockPort(port);
	}

    QySocketAddress::QySocketAddress(const QySocketAddress& addr)
	{
		this->operator=(addr);
	}

    void QySocketAddress::clear()
	{
        mHostname.clear();
        mIp = 0;
        mPort = 0;
	}

    QySocketAddress& QySocketAddress::operator=(const QySocketAddress& addr)
	{
        mHostname = addr.mHostname;
        mIp = addr.mIp;
        mPort = addr.mPort;
		return *this;
	}

    void QySocketAddress::setIP(uint32 ip)
	{
        mHostname.clear();
        mIp = ip;
	}

    bool QySocketAddress::setIP(const std::string& hostname, bool use_dns)
	{
        mHostname = hostname;
        mIp = 0;
        return resolve(true, use_dns);
	}

    void QySocketAddress::setResolvedIP(uint32 ip)
	{
        mIp = ip;
	}

    void QySocketAddress::setSockPort(int port)
	{
		ASSERT((0 <= port) && (port < 65536));
        mPort = port;
	}

    uint32 QySocketAddress::ip() const
	{
        return mIp;
	}

    uint16 QySocketAddress::port() const
	{
        return mPort;
	}

    std::string QySocketAddress::ipAsString() const
	{
        if (!mHostname.empty())
            return mHostname;
        return ipToString(mIp);
	}

    std::string QySocketAddress::portAsString() const
	{
        std::ostringstream ost;
        ost << mPort;
		return ost.str();
	}

    std::string QySocketAddress::toString() const
	{
        std::ostringstream ost;
        ost << ipAsString();
		ost << ":";
		ost << port();
		return ost.str();
	}

    bool QySocketAddress::isAny() const
	{
        return (mIp == 0);
	}

    bool QySocketAddress::isLocalIP() const
	{
        return (mIp >> 24) == 127;
	}

    bool QySocketAddress::isPrivateIP() const
	{
        return ((mIp >> 24) == 127) ||
            ((mIp >> 24) == 10) ||
            ((mIp >> 20) == ((172 << 4) | 1)) ||
            ((mIp >> 16) == ((192 << 8) | 168));
	}

    bool QySocketAddress::isUnresolved() const
	{
        return isAny() && !mHostname.empty();
	}

    bool QySocketAddress::resolve(bool force, bool use_dns)
	{
        if (mHostname.empty()) {
			// nothing to resolve
        } else if (!force && !isAny()) {
			// already resolved
        } else if (uint32 ip = stringToIP(mHostname, use_dns)) {
            mIp = ip;
		} else {
			return false;
		}
		return true;
	}

    bool QySocketAddress::operator ==(const QySocketAddress& addr) const
	{
        return equalIPs(addr) && equalPorts(addr);
	}

    bool QySocketAddress::operator <(const QySocketAddress& addr) const
	{
        if (mIp < addr.mIp)
			return true;
        else if (addr.mIp < mIp)
			return false;

		// We only check hostnames if both IPs are zero.  This matches EqualIPs()
        if (addr.mIp == 0) {
            if (mHostname < addr.mHostname)
				return true;
            else if (addr.mHostname < mHostname)
				return false;
		}

        return mPort < addr.mPort;
	}

    bool QySocketAddress::equalIPs(const QySocketAddress& addr) const
	{
        return (mIp == addr.mIp) && ((mIp != 0) || (mHostname == addr.mHostname));
	}

    bool QySocketAddress::equalPorts(const QySocketAddress& addr) const
	{
        return (mPort == addr.mPort);
	}

    size_t QySocketAddress::hash() const
	{
		size_t h = 0;
        h ^= mIp;
        h ^= mPort | (mPort << 16);
		return h;
	}

    size_t QySocketAddress::size() const
	{
        return sizeof(mIp) + sizeof(mPort);
	}

    void QySocketAddress::write(char* buf, int len) const
	{
		// TODO: Depending on how this is used, we may want/need to write hostname
        HUNUSED(len);
		ASSERT((size_t)len >= Size_());
        reinterpret_cast<uint32*>(buf)[0] = mIp;
        buf += sizeof(mIp);
        reinterpret_cast<uint16*>(buf)[0] = mPort;
	}

    void QySocketAddress::read(const char* buf, int len)
	{
        HUNUSED(len);
		ASSERT((size_t)len >= Size_());
        mIp = reinterpret_cast<const uint32*>(buf)[0];
        buf += sizeof(mIp);
        mPort = reinterpret_cast<const uint16*>(buf)[0];
	}

    void QySocketAddress::toSockAddr(sockaddr_in* saddr) const
	{
		memset(saddr, 0, sizeof(*saddr));
		saddr->sin_family = AF_INET;
        saddr->sin_port = HostToNetwork16(mPort);
        if (0 == mIp) {
			saddr->sin_addr.s_addr = INADDR_ANY;
		} else {
            saddr->sin_addr.s_addr = HostToNetwork32(mIp);
		}
	}

    void QySocketAddress::fromSockAddr(const sockaddr_in& saddr)
	{
        setIP(NetworkToHost32(saddr.sin_addr.s_addr));
        setSockPort(NetworkToHost16(saddr.sin_port));
	}

    std::string QySocketAddress::ipToString(uint32 ip)
	{
        std::ostringstream ost;
		ost << ((ip >> 24) & 0xff);
		ost << '.';
		ost << ((ip >> 16) & 0xff);
		ost << '.';
		ost << ((ip >> 8) & 0xff);
		ost << '.';
		ost << ((ip >> 0) & 0xff);
		return ost.str();
	}

    uint32 QySocketAddress::stringToIP(const std::string& hostname, bool use_dns)
	{
		uint32 ip = 0;
		in_addr addr;
		if (inet_aton(hostname.c_str(), &addr) != 0) {
			ip = NetworkToHost32(addr.s_addr);
        } else if (use_dns) {
            // Note: this is here so we can spot spurious DNS resolutions for a while

            if (hostent * pHost = gethostbyname(hostname.c_str())) {
                ip = NetworkToHost32(*reinterpret_cast<uint32 *>(pHost->h_addr_list[0]));
            } else {
            }
		}
		return ip;
	}

    std::string QySocketAddress::localHostName()
	{
		char hostname[256];
		if (gethostname(hostname, ARRAY_SIZE(hostname)) == 0)
			return hostname;
		return "";
	}

    bool QySocketAddress::localIPs(std::vector<uint32>& ips)
	{
		ips.clear();

        const std::string name = localHostName();
        if (name.empty())
			return false;

        if (hostent * pHost = gethostbyname(name.c_str())) {
			for (size_t i=0; pHost->h_addr_list[i]; ++i) {
				uint32 ip =
					NetworkToHost32(*reinterpret_cast<uint32 *>(pHost->h_addr_list[i]));
				ips.push_back(ip);
			}
			return !ips.empty();
        }
		return false;
	}

} // namespace qy
