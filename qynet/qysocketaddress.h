#ifndef QY_SocketAddress_H__
#define QY_SocketAddress_H__

#include <string>
#include <vector>
#include "qybasictypes.h"
struct sockaddr_in;

namespace qy 
{

	// Records an IP address and port, which are 32 and 16 bit integers,
	// respectively, both in <b>host byte-order</b>.
    class QySocketAddress
	{
	public:
		// Creates a missing / unknown address.
        QySocketAddress();

		// Creates the address with the given host and port.  If use_dns is true,
		// the hostname will be immediately resolved to an IP (which may block for
		// several seconds if DNS is not available).  Alternately, set use_dns to
		// false, and then call Resolve() to complete resolution later, or use
		// SetResolvedIP to set the IP explictly.
        QySocketAddress(const std::string& hostname, int port = 0, bool use_dns = true);

		// Creates the address with the given IP and port.
        QySocketAddress(uint32 ip, int port);

		// Creates a copy of the given address.
        QySocketAddress(const QySocketAddress& addr);

		// Resets to missing / unknown address.
        void clear();

		// Replaces our address with the given one.
        QySocketAddress& operator =(const QySocketAddress& addr);

		// Changes the IP of this address to the given one, and clears the hostname.
        void setIP(uint32 ip);

		// Changes the hostname of this address to the given one.
		// Calls Resolve and returns the result.
        bool setIP(const std::string& hostname, bool use_dns = true);

		// Sets the IP address while retaining the hostname.  Useful for bypassing
		// DNS for a pre-resolved IP.
        void setResolvedIP(uint32 ip);

		// Changes the port of this address to the given one.
        void setSockPort(int port);

		// Returns the IP address.
		uint32 ip() const;

		// Returns the port part of this address.
		uint16 port() const;

		// Returns the hostname
        const std::string& hostname() const { return mHostname; }

		// Returns the IP address in dotted form.
        std::string ipAsString() const;

		// Returns the port as a string
        std::string portAsString() const;

		// Returns a display version of the IP/port.
        std::string toString() const;

		// Determines whether this represents a missing / any address.
        bool isAny() const;

		// Synomym for missing / any.
        bool isNil() const { return isAny(); }

		// Determines whether the IP address refers to the local host, i.e. within
		// the range 127.0.0.0/8.
        bool isLocalIP() const;

		// Determines whether the IP address is in one of the private ranges:
		// 127.0.0.0/8 10.0.0.0/8 192.168.0.0/16 172.16.0.0/12.
        bool isPrivateIP() const;

		// Determines whether the hostname has been resolved to an IP
        bool isUnresolved() const;

		// Attempt to resolve a hostname to IP address.
		// Returns false if resolution is required but failed.
		// 'force' will cause re-resolution of hostname.
		// 
        bool resolve(bool force = false, bool use_dns = true);

		// Determines whether this address is identical to the given one.
        bool operator ==(const QySocketAddress& addr) const;

        inline bool operator !=(const QySocketAddress& addr) const {
			return !this->operator ==(addr);
		}

		// Compares based on IP and then port.
        bool operator <(const QySocketAddress& addr) const;

		// Determines whether this address has the same IP as the one given.
        bool equalIPs(const QySocketAddress& addr) const;

		// Deteremines whether this address has the same port as the one given.
        bool equalPorts(const QySocketAddress& addr) const;

		// Hashes this address into a small number.
        size_t hash() const;

		// Returns the size of this address when written.
        size_t size() const;

		// Writes this address into the given buffer.
        void write(char* buf, int len) const;

		// Reads this address from the given buffer.
        void read(const char* buf, int len);

		// Convert to and from sockaddr_in
        void toSockAddr(sockaddr_in* saddr) const;
        void fromSockAddr(const sockaddr_in& saddr);

		// Converts the IP address given in compact form into dotted form.
        static std::string ipToString(uint32 ip);

		// Converts the IP address given in dotted form into compact form.
		// Without 'use_dns', only dotted names (A.B.C.D) are resolved.
        static uint32 stringToIP(const std::string& str, bool use_dns = true);

		// Get local machine's hostname
        static std::string localHostName();

		// Get a list of the local machine's ip addresses
        static bool localIPs(std::vector<uint32>& ips);

	private:
        std::string mHostname;
        uint32 mIp;
        uint16 mPort;
	};

} // namespace qy

#endif // QY_SocketAddress_H__
