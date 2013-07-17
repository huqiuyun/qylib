#pragma  once

#include "qyutil/qyconfig.h"
#include <string>

DEFINE_NAMESPACE(qy)
/** 
 * @param byte = 16bytes binary data
*/
QYUTIL_API bool qymd5_crypt16(const char *inbuf, size_t buflen, unsigned char byte[16]);

/** 
 * @param val = 16bytes string data
*/
QYUTIL_API bool qymd5_crypt16A(const char *inbuf, size_t buflen, std::string& val);
QYUTIL_API bool qymd5_crypt16W(const char *inbuf, size_t buflen, std::wstring& val);

/**
 * @param val = 32bytes hex string
*/
QYUTIL_API bool qymd5_crypt32A(const char *inbuf, size_t buflen, std::string& val);
QYUTIL_API bool qymd5_crypt32W(const char *inbuf, size_t buflen, std::wstring& val);

/**
 * @param md5 = 16bytes binary data
 * @param val = 32bytes hex string
 *
*/
QYUTIL_API void qymd5_16To32A(const BYTE* md5, std::string& val);
QYUTIL_API void qymd5_16To32W(const BYTE* md5, std::wstring& val);

/** 
 * @param hash32 = 32bytes hex string
 * @param hash16 = 16bytes binary data
 *
*/
QYUTIL_API void qymd5_32To16W(const wchar_t* hash32 , PBYTE hash16);

END_NAMESPACE(qy)