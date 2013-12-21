#ifndef __QY_WIN32_H__
#define __QY_WIN32_H__

#include <winsock2.h>
#include <windows.h>
#include <malloc.h>
#include <string>

namespace qy
{
	inline std::wstring ToUtf16(const std::string& str) {
		size_t len16 = ::MultiByteToWideChar(CP_UTF8, 0, str.data(), str.length(),NULL, 0);
		wchar_t *ws = static_cast<wchar_t*>(_alloca(len16 * sizeof(wchar_t)));
		::MultiByteToWideChar(CP_UTF8, 0, str.data(), str.length(), ws, len16);
		std::wstring result(ws, len16);
		return result;
	}

	inline std::string ToUtf8(const std::wstring& wstr) {
		size_t len8 = ::WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.length(),NULL, 0, NULL, NULL);
		char* ns = static_cast<char*>(_alloca(len8));
		::WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.length(),
			ns, len8, NULL, NULL);
		std::string result(ns, len8);
		return result;
	}
}

#endif  // __QY_NET_WIN32_H__
