#ifndef QY_BYTEORDER_H__
#define QY_BYTEORDER_H__

#include "qybasictypes.h"

#ifdef H_OS_WIN
  #include <winsock2.h>
#else
extern "C" {
  #include <arpa/inet.h>
}
#endif

namespace qy 
{
	inline uint16 HostToNetwork16(uint16 n) {
		return htons(n);
	}

	inline uint32 HostToNetwork32(uint32 n) {
		return htonl(n);
	}

	inline uint16 NetworkToHost16(uint16 n) {
		return ntohs(n);
	}

	inline uint32 NetworkToHost32(uint32 n) {
		return ntohl(n);
	}
} // namespace qy

#endif // QY_BYTEORDER_H__
