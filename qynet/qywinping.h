#ifndef __QY_NET_WINPING_H__
#define __QY_NET_WINPING_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <winsock2.h>
#define _WINSOCKAPI_
#include <windows.h>


#include "qyqybasictypes.h"

namespace qy
{

	// This class wraps a Win32 API for doing ICMP pinging.  This API, unlike the
	// the normal socket APIs (as implemented on Win9x), will return an error if
	// an ICMP packet with the dont-fragment bit set is too large.  This means this
	// class can be used to detect the MTU to a given address.

	typedef struct ip_option_information
	{
		UCHAR   Ttl;                // Time To Live
		UCHAR   Tos;                // Type Of Service
		UCHAR   Flags;              // IP header flags
		UCHAR   OptionsSize;        // Size in bytes of options data
		PUCHAR  OptionsData;        // Pointer to options data
	} IP_OPTION_INFORMATION, * PIP_OPTION_INFORMATION;

	typedef HANDLE (WINAPI *PIcmpCreateFile)();

	typedef BOOL (WINAPI *PIcmpCloseHandle)(HANDLE icmp_handle);

	typedef DWORD (WINAPI *PIcmpSendEcho)(
		HANDLE                   IcmpHandle,
		ULONG                    DestinationAddress,
		LPVOID                   RequestData,
		WORD                     RequestSize,
		PIP_OPTION_INFORMATION   RequestOptions,
		LPVOID                   ReplyBuffer,
		DWORD                    ReplySize,
		DWORD                    Timeout);

	class WinPing 
	{
	public:
		WinPing();
		~WinPing();

		// Determines whether the class was initialized correctly.
		bool IsValid() { return valid_; }

		// Attempts to send a ping with the given parameters.
		enum PingResult { PING_FAIL, PING_TOO_LARGE, PING_TIMEOUT, PING_SUCCESS };

		PingResult Ping(uint32 ip, uint32 data_size, uint32 timeout_millis, uint8 ttl,bool allow_fragments);

	private:
		HMODULE dll_;
		HANDLE hping_;
		PIcmpCreateFile create_;
		PIcmpCloseHandle close_;
		PIcmpSendEcho send_;
		char* data_;
		uint32 dlen_;
		char* reply_;
		uint32 rlen_;
		bool valid_;
	};
} // namespace qy

#endif // __QY_NET_WINPING_H__

