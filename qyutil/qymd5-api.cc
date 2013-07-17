#include "qymd5api.h"
#include "qymd5.h"
#include "qycodec.h"

DEFINE_NAMESPACE(qy)

int hex_tolW(const wchar_t* str)
{
	int	res = 0;
	const wchar_t* p = str;	
	for (;*p!=0;p++)
	{
		if (*p >= L'0' && *p <= L'9')
		{
			res = res*16 + *p - L'0';
		}
		else if (*p >= L'a' && *p <= L'z')
		{
			res = res*16 + *p - L'a' + 0xa;
		}
		else if (*p >= L'A' && *p <= L'Z')
		{
			res = res*16 + *p - L'A' + 0xa;
		}
		else
		{
			return res;
		}
	}
	return res;
}

bool qymd5_crypt16(const char *inbuf, size_t buflen, unsigned char byte[16])
{
	byte[15] = 0;
	if (NULL == inbuf || buflen == 0)
	{
		return false;
	}
	QyMD5 data;
	data.update((BYTE *)(inbuf), (unsigned int)buflen);
	data.finalize();
	data.raw_digest (byte);
	return true;
}

bool qymd5_crypt16A(const char *inbuf, size_t buflen, std::string& val)
{
    unsigned char byte[17] = {0};
	if (!qymd5_crypt16(inbuf , buflen , byte))
	{
		return false;
	}
	val.assign((const char*)byte, 17);
	return true;
}

bool qymd5_crypt16W(const char *inbuf, size_t buflen, std::wstring& val)
{
    unsigned char byte[17] = {0};
	if (!qymd5_crypt16(inbuf , buflen , byte))
	{
		return false;
	}
    qycodec_c2WString(val, (const char*)byte, 16);
	return true;
}

bool qymd5_crypt32A(const char *inbuf, size_t buflen, std::string& val)
{
    unsigned char byte[17] = {0};
	if (!qymd5_crypt16(inbuf , buflen , byte))
	{
		return false;
	}
	qymd5_16To32A(byte , val);
	return true;
}

bool qymd5_crypt32W(const char *inbuf, size_t buflen, std::wstring& val)
{
    unsigned char byte[17] = {0};
	if (!qymd5_crypt16(inbuf , buflen , byte))
	{
		return false;
	}
	qymd5_16To32W(byte , val);
	return true;
}

void qymd5_16To32A(const BYTE* pbyMd5, std::string& val)
{
    qycodec_bin2HexA(val,(const char*)pbyMd5,16);
}

void qymd5_16To32W(const BYTE* pbyMd5, std::wstring& val)
{
    qycodec_bin2HexW(val,(const char*)pbyMd5,16);
}

void qymd5_32To16W(const wchar_t* hash32,PBYTE hash16)
{
	wchar_t byte[3] = {0};
	for (int i = 0; i < 16; i++)
	{
		wcsncpy(byte, hash32 + i*2 , 2);
		hash16[i] = hex_tolW(byte);
	}
}

END_NAMESPACE(qy)