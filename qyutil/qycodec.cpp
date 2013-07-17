/**
*  @brief String opeartion.
*  @file  strconv.cpp
*  @author loach 
*  @Email < loachmr@sina.com >
*
* Copyright (C) 1996-2010 SINA Corporation, All Rights Reserved
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*/

#include "qycodec.h"
#include <stdlib.h>
#include <algorithm>

DEFINE_NAMESPACE(qy)

static const char HEX[] = "0123456789abcdef";
char qycodec_hex_encode(unsigned char val)
{
    return (val < 16) ? HEX[val] : '!';
}

unsigned char qycodec_hex_decode(char ch)
{
    char lower = tolower(ch);
    return (ch <= '9') ? (ch - '0') : ((lower - 'a') + 10);
}

bool qycodec_isemail(const char *str)
{
	if (!str)
		return false;
    
	const char *tmpstr = strchr(str, '@');
	if (!tmpstr || tmpstr==str)
		return false;
    
	const char *t = str;
	while(t!=tmpstr)
	{
		if (!::isalnum(*t) && *t!='-' && *t!='_' && *t!='.')
			break;
		t++;
	}
	if (t!=tmpstr)
		return false;
    
	t++;
	while(*t)
	{
		if (!::isalnum(*t) && *t!='-' && *t!='.')
		{
			return false;
		}
		t++;
	}
	return true;
}

bool qycodec_isphone(const char *str)
{
	if (!str)
		return false;
    
	while( ::isdigit(*str) || (*str == '(') || (*str == ')') || (*str == '-'))
		str++;
	return *str=='\0';
}

bool qycodec_isalpha(const char *str)
{
	if (!str)
		return false;
    
	while(::isalpha(*str))str++;
	return *str=='\0';
}

bool qycodec_isalnum(const char* str)
{
	if (!str)
		return false;
    
	while(::isalnum(*str))str++;
	return *str=='\0';
}

bool qycodec_isdigit(const char *str)
{
	if (!str)
		return false;
    
	while(::isdigit(*str))str++;
    
	return *str=='\0';
}

const wchar_t* qycodec_extractStringW( const std::wstring &seperate,const std::wstring &in,	std::wstring &out)
{
	std::wstring::size_type at = in.find(seperate);
	if (at != std::wstring::npos)
	{
		out = in.substr(0,at);
	}
	else
	{
		out = in;
	}
	return out.c_str();
}

const char* qycodec_extractStringA( const std::string &seperate,const std::string &in,	std::string &out)
{
	std::string::size_type at = in.find(seperate);
	if (at != std::string::npos)
	{
		out = in.substr(0,at);
	}
	else
	{
		out = in;
	}
	return out.c_str();
}

void  qycodec_toLower(std::wstring& str)
{
	std::transform(str.begin(), str.end(), str.begin(), towlower);
}

void qycodec_toUpper(std::wstring& str)
{
	std::transform(str.begin(), str.end(), str.begin(), towupper);
}

void qycodec_trim(std::wstring& strReturn,std::wstring const& str,std::wstring const& filter)
{
	strReturn.clear();
	size_t begin = std::wstring::npos;
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (filter.find( str[i]) == std::wstring::npos)
		{
			begin = i;
			break;
		}
	}
    
	if (begin == std::wstring::npos)
	{
		return ;
	}
    
	size_t end = str.size();
	for (size_t i = str.size() - 1; i >= begin; --i)
	{
		if (filter.find( str[i]) == std::wstring::npos)
		{
			end = i + 1;
			break;
		}
	}
    
	strReturn = str.substr(begin, end - begin);
}

void qycodec_replace(std::wstring &replaceStr, std::wstring const &srcStr, std::wstring const &desStr)
{
	if (0 == srcStr.size() || replaceStr.size() < srcStr.size())
	{
		return;
	}
    
	size_t firstPos = replaceStr.find(srcStr);
	while (firstPos != -1)
	{
		replaceStr.replace(firstPos, srcStr.size(), desStr);
		firstPos = replaceStr.find(srcStr);
	}
}

void qycodec_splitWString(const std::wstring &stringOrg,
						  const std::wstring & delimiter, std::list<std::wstring> &result)
{
	size_t last = 0;
	size_t index = stringOrg.find_first_of(delimiter,last);
	while (index != std::wstring::npos)
	{
		result.push_back(stringOrg.substr(last,index-last));
		last = index + 1;
		index = stringOrg.find_first_of(delimiter,last);
	}
	if (index - last > 0)
	{
		result.push_back(stringOrg.substr(last ,index - last));
	}
}

void qycodec_splitCString(const std::string &stringOrg,
						  const std::string & delimiter, std::list<std::string> &result)
{
	size_t last = 0;
	size_t index = stringOrg.find_first_of(delimiter,last);
	while(index != std::string::npos)
	{
		result.push_back(stringOrg.substr(last,index-last));
		last = index + 1;
		index = stringOrg.find_first_of(delimiter,last);
	}
	if(index - last > 0)
	{
		result.push_back(stringOrg.substr(last ,index - last));
	}
}

// hex 2 bin | bin 2 hex

const wchar_t* qycodec_c2HexW(wchar_t outstr[3], unsigned char c)
{
	static wchar_t s_Table[] = {'0', '1', '2', '3',
		'4', '5', '6', '7',
		'8', '9', 'A', 'B',
		'C', 'D', 'E', 'F'
	};
	outstr[0] = s_Table[c/16];
	outstr[1] = s_Table[c%16];
    outstr[2] = '\0';
	return outstr;
}

const char* qycodec_c2HexA(char outstr[3], unsigned char c)
{
	static char s_Table[] = {'0', '1', '2', '3',
		'4', '5', '6', '7',
		'8', '9', 'A', 'B',
		'C', 'D', 'E', 'F'
	};
	outstr[0] = s_Table[c/16];
	outstr[1] = s_Table[c%16];
    outstr[2] = '\0';
	return outstr;
}

const wchar_t* qycodec_bin2HexW(std::wstring& outstr, const char* bin ,size_t len)
{
	wchar_t str[3]={0};
	outstr.reserve(len * 2);
	const unsigned char * bsource =
    reinterpret_cast<const unsigned char *>(bin);
    
	for (size_t i = 0; i < len; ++i)
	{
		unsigned char c = (unsigned char)bsource[i];
		outstr += qycodec_c2HexW(str,c);
	}
	return outstr.c_str();
}

const char* qycodec_bin2HexA(std::string& outstr, const char* bin ,size_t len)
{
	char str[3]={0};
	outstr.reserve(len * 2);
	const unsigned char * bsource =
    reinterpret_cast<const unsigned char *>(bin);
    
	for (size_t i = 0; i < len; ++i)
	{
		unsigned char c = (unsigned char)bsource[i];
		outstr += qycodec_c2HexA(str,c);
	}
	return outstr.c_str();
}

const char* qycodec_hex2BinA(std::string& outstr,const char* source, size_t srclen)
{
    unsigned char buffer;
    size_t srcpos = 0;
    while ((srcpos + 1) < srclen)
    {
        unsigned char v1 = (qycodec_hex_decode(source[srcpos]) << 4);
        unsigned char v2 = qycodec_hex_decode(source[srcpos+1]);
        buffer =  v1 | v2;
        outstr += buffer;
    }
	return outstr.c_str();
}

const wchar_t* qycodec_hex2BinW(std::wstring& outstr , const char* source , int srclen)
{
    std::string bin;
    qycodec_hex2BinA(bin,source,srclen);
    qycodec_c2WString(outstr, bin.c_str(),bin.size());
	return outstr.c_str();
}

//-------------------------------------------------------------------------------------//
// uicode ,utf8 , mbsc...

#if defined(WIN32) || defined(_WIN32)
inline int acp_()
{
    return AreFileApisANSI() ? CP_ACP : CP_OEMCP;
}
#else
inline int acp_()
{
    return CP_ACP;
}
#endif
/*
** Convert an ansi string to microsoft unicode, based on the
** current codepage settings for file apis.
** 
** Space to hold the returned string is obtained
** from malloc.
*/
size_t qycodec_c2W(wchar_t** pout ,const char *zinname,int codepage)
{
    QY_UNUSED(codepage);
	wchar_t *zMbcsname = 0;
	int nByte = 0;

	if (!zinname || * zinname == '\0')
		return 0;

#if defined(WIN32) || defined(_WIN32)

	nByte = MultiByteToWideChar(codepage, 0, zinname, -1, NULL,0);
	zMbcsname = (wchar_t*)malloc( sizeof(wchar_t)*(nByte+1));
	if (zMbcsname == 0)
		return 0;

	nByte = MultiByteToWideChar(codepage, 0, zinname, -1, zMbcsname, nByte);
#else
    
#endif
    
	if (nByte >= 0)
	{
		zMbcsname[nByte] = 0;
        *pout = zMbcsname;
        return (size_t)nByte;
	}
	return 0;
}

/*
** Convert microsoft unicode to multibyte character string, based on the
** user's Ansi codepage.
**
** Space to hold the returned string is obtained from
** malloc().
*/
size_t qycodec_w2C(char** pout ,const wchar_t *zWide, int codepage)
{
    QY_UNUSED(codepage);
	char *zname = 0;
	int  nByte = 0;

	if (!zWide || *zWide == '\0')
		return 0;
    
#if defined(WIN32) || defined(_WIN32)

	nByte = WideCharToMultiByte(codepage, 0, zWide, -1, 0, 0, 0, 0);
	zname = (char*)malloc( nByte + 1);
	if (zname == 0) return 0;

	nByte = WideCharToMultiByte(codepage, 0, zWide, -1, zname, nByte+1, 0, 0);
#else
    
#endif
    
    if (nByte >= 0)
    {
        zname[nByte] = '\0';
        *pout = zname;
        return (size_t)nByte;
    }
    return 0;
}


const wchar_t* qycodec_c2WString(std::wstring& outstr , const char* pStr , size_t len,unsigned int codepage /*=CP_ACP*/)
{
    QY_UNUSED(codepage);
	if (pStr == NULL)
	{
		return outstr.c_str();
	}
    
	if (len == SIZE_MAX)
	{
		return outstr.c_str();
	}
#if defined(WIN32) || defined(_WIN32)

	// figure out how many wide characters we are going to get
	int nChars = MultiByteToWideChar( codepage , 0 , pStr , len , NULL , 0) ;
	if(nChars <= 0)
	{
		outstr = L"";
		return outstr.c_str();
	}
	// convert the narrow string to a wide string
	// nb: slightly naughty to write directly into the string like this
	outstr.resize( nChars) ;
	MultiByteToWideChar( codepage , 0 , pStr , len ,
                        const_cast<wchar_t*>(outstr.c_str()) , nChars) ;
#else
#endif
    
	return outstr.c_str() ;
}

const char* qycodec_w2CString(std::string& outstr ,  const wchar_t* pStr , size_t len, unsigned int codepage)
{
    QY_UNUSED(codepage);
	if (pStr == NULL)
	{
		return outstr.c_str();
	}
    
	if (len == SIZE_MAX)
	{
		return outstr.c_str();
	}
#if defined(WIN32) || defined(_WIN32)  
	// figure out how many narrow characters we are going to get
	int nChars = WideCharToMultiByte( codepage , 0 ,
                                     pStr , len , NULL , 0 , NULL , NULL) ;
	if(nChars <= 0)
	{
		outstr = "";
		return outstr.c_str();
	}
	// convert the wide string to a narrow string
	// nb: slightly naughty to write directly into the string like this
	outstr.resize( nChars) ;
	WideCharToMultiByte( codepage , 0 , pStr , len ,
                        const_cast<char*>(outstr.c_str()) , nChars , NULL , NULL) ;
#else
#endif
    
	return outstr.c_str() ;
}


size_t qycodec_w2Utf8(char** pout , const wchar_t *zWide)
{
	int nByte = 0;
	char *zname = 0;

	if (!zWide || *zWide == '\0')
		return 0;
    
#if defined(WIN32) || defined(_WIN32)
	nByte = WideCharToMultiByte(CP_UTF8, 0, zWide, -1, 0, 0, 0, 0);
	zname = (char*)malloc( nByte +1);
	if (zname == 0)
	{
		return 0;
	}
	nByte = WideCharToMultiByte(CP_UTF8, 0, zWide, -1, zname, nByte+1, 	0, 0);
#else
#endif
    
    if (nByte > 0)
	{
		zname[nByte] = '\0';
        *pout = zname ;
        return nByte;
    }
    return 0;
}

size_t qycodec_utf82W(wchar_t** pout ,const char* utf8)
{
	int nByte = 0;
	wchar_t *zwstr = 0;

	if (!utf8)
		return 0;
#if defined(WIN32) || defined(_WIN32)
	nByte = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	zwstr = (wchar_t*)malloc( (nByte+1)*sizeof(wchar_t));
	if (zwstr == 0){
		return 0;
	}
	nByte = MultiByteToWideChar(CP_UTF8, 0, utf8 , -1 , zwstr, nByte);
#else
#endif
    
    if (nByte > 0)
	{
		zwstr[nByte] = 0;
        *pout = zwstr;
        return nByte;
    }
    return 0;
}

/*
** Convert multibyte character string to UTF-8.  Space to hold the
** returned string is obtained from malloc().
*/
size_t qycodec_c2Utf8(char** pout , const char *zinname)
{
	wchar_t *zTmp = 0;
	size_t outlen = 0;

	outlen = qycodec_c2W(&zTmp , zinname,acp_());
	if (!zTmp || outlen <= 0)
	{
		return 0;
	}

	outlen = qycodec_w2Utf8(pout , zTmp);
	free( zTmp);

	return outlen ;
}

/*
** Convert UTF-8 to multibyte character string.  Space to hold the 
** returned string is obtained from sqliteMalloc().
*/
size_t qycodec_utf82C(char** pout ,const char *zstr)
{
	wchar_t* pwout = 0;
	size_t outlen = 0;
	if (0 == qycodec_utf82W(&pwout , zstr))
		return 0;

	outlen = qycodec_w2C(pout , pwout, acp_());

	free( pwout);

	return outlen;
}


const char* qycodec_wToUtf8( std::string& outstr, const wchar_t* wstr) 
{
	char* utf8 = 0;
	outstr = "";

	if (0 != qycodec_w2Utf8(&utf8 , wstr))
	{
		outstr = utf8;
		free(utf8);
	}
	return outstr.c_str();
}

const wchar_t* qycodec_utf8ToW( std::wstring& woutstr , const char* utf8) 
{
	wchar_t *zwstr = 0;
	woutstr = L"";

	if (0 != qycodec_utf82W(&zwstr , utf8))
	{
		woutstr = zwstr;
		free(zwstr);
	}
	return woutstr.c_str();
}

/*
** Convert an ansi string to microsoft unicode, based on the
** current codepage settings for file apis.
** 
*/
const wchar_t* qycodec_mbcsToW(std::wstring& woutstr , const char *zstr)
{
	wchar_t *zwstr = 0;
	woutstr = L"";

	if (0 != qycodec_c2W(&zwstr , zstr, acp_()))
	{
		woutstr = zwstr;
		free(zwstr);
	}
	return woutstr.c_str();
}

/*
** Convert microsoft unicode to multibyte character string, based on the
** user's Ansi codepage.
**
** Space to hold the returned string is obtained from
** sqliteMalloc().
*/
const char* qycodec_wToMbcs(std::string& outstr , const wchar_t *zwstr)
{
	char *zstr = 0;
	outstr = "";

	if (0 != qycodec_w2C(&zstr , zwstr,acp_()))
	{
		outstr = zstr;
		free(zstr);
	}
	return outstr.c_str();
}

/*
** Convert multibyte character string to UTF-8.  Space to hold the
** returned string is obtained from sqliteMalloc().
*/
const char* qycodec_mbcsToUtf8(std::string& outstr , const char *zstr)
{
	char* zout = 0;
	if (0 != qycodec_c2Utf8(&zout , zstr))
	{
		outstr = zout;
		free( zout);
	}
	return outstr.c_str();
}

/*
** Convert UTF-8 to multibyte character string.  Space to hold the 
** returned string is obtained from sqliteMalloc().
*/
const char* qycodec_utf8ToMbcs(std::string& outstr ,const char *zstr)
{
	char* zout = 0;
	if (0 != qycodec_utf82C(&zout , zstr))
	{
		outstr = zout;
		free( zout);
	}
	return outstr.c_str();
}


//
QyC2W::QyC2W(const char* z)
:QyCodecT<wchar_t>(L"")
{
	length_ = qycodec_c2W(&out_ , z,acp_());
}

//
QyW2C::QyW2C(const wchar_t* z)
:QyCodecT<char>("")
{
	length_ = qycodec_w2C(&out_ , z,acp_());
}

//
QyW2UTF8::QyW2UTF8(const wchar_t* z)
:QyCodecT<char>("")
{
	length_ = qycodec_w2Utf8(&out_ , z);
}

//
QyUTF82W::QyUTF82W(const char* z)
:QyCodecT<wchar_t>(L"")
{
	length_ = qycodec_utf82W(&out_ , z);
}

//
QyC2UTF8::QyC2UTF8(const char* z)
:QyCodecT<char>("")
{
	length_ = qycodec_c2Utf8(&out_ , z);
}

//
QyUTF82C::QyUTF82C(const char* z)
:QyCodecT<char>("")
{
	length_ = qycodec_utf82C(&out_ , z);
}

//-------------------------------------------------------------------------------------//
// url encode ,decocde
const unsigned char URL_UNSAFE  = 0x1; // 0-33 "#$%&+,/:;<=>?@[\]^`{|} 127
const unsigned char XML_UNSAFE  = 0x2; // "&'<>
const unsigned char HTML_UNSAFE = 0x2; // "&'<>

//  ! " # $ % & ' () * + , - . / 0 1 2 3 4 6 5 7 8 9 : ; < = > ?
//@ A B C D E F G H I J K L M N O P Q R S T U V W X Y Z [ \ ] ^ _
//` a b c d e f g h i j k l m n o p q r s t u v w x y z { | } ~

const unsigned char ASCII_CLASS[128] =
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,3,1,1,1,3,2,0,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,1,1,3,1,3,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,
};

size_t url_encode(char** out, const char * source, size_t srclen)
{
	size_t buflen = (srclen * 3) + 2;
	char* buffer = (char*)malloc( buflen);
    if (NULL == buffer)
        return 0;
    
    size_t srcpos = 0, bufpos = 0;
    while ((srcpos < srclen) && (bufpos + 1 < buflen))
    {
        unsigned char ch = source[srcpos++];
        if ((ch < 128) && (ASCII_CLASS[ch] & URL_UNSAFE))
        {
            if (bufpos + 3 >= buflen)
            {
                break;
            }
            buffer[bufpos+0] = '%';
            buffer[bufpos+1] = qycodec_hex_encode((ch >> 4) & 0xF);
            buffer[bufpos+2] = qycodec_hex_encode((ch    ) & 0xF);
            bufpos += 3;
        }
        else
        {
            buffer[bufpos++] = ch;
        }
    }
    buffer[bufpos] = '\0';
    *out = buffer;
    return bufpos;
}

size_t url_decode(char** out,const char * source, size_t srclen)
{
 	size_t buflen = srclen + 1;
	char* buffer = (char*)malloc( buflen);
    if (NULL == buffer)
        return 0;
    
    size_t srcpos = 0, bufpos = 0;
    while ((srcpos < srclen) && (bufpos + 1 < buflen))
    {
        unsigned char ch = source[srcpos++];
        if (ch == '+')
        {
            buffer[bufpos++] = ' ';
        }
        else if ((ch == '%') && (srcpos + 1 < srclen))
        {
            buffer[bufpos++] = (qycodec_hex_decode(source[srcpos]) << 4) | qycodec_hex_decode(source[srcpos+1]);
            srcpos += 2;
        }
        else
        {
            buffer[bufpos++] = ch;
        }
    }
    buffer[bufpos] = '\0';
    *out = buffer;
    return bufpos;
}

size_t qycodec_urlDecodeA(char** pout,const char* pin , size_t len)
{
    return url_decode(pout,pin,len);
}

size_t	qycodec_urlDecodeW(wchar_t** pout,const char* pin,  size_t len)
{		
	char* poutA = 0;
	size_t outlen = 0;
    outlen = url_decode(&poutA,pin,len);
	if (outlen > 0)
	{//
		outlen = qycodec_c2W(pout , poutA,acp_());
	}		
	if (poutA) free(poutA);
	return outlen;
}

size_t qycodec_urlEncodeA(char** pout, const char* pin ,  size_t len)
{
    return url_encode(pout,pin,len);
}

size_t qycodec_urlEncodeW(wchar_t** pout, const char* pin ,  size_t len)
{
	char* poutA = 0;
	size_t outlen = 0;

	outlen = url_encode(&poutA , pin , len);
	if (outlen)
	{
		outlen = qycodec_c2W(pout , poutA,acp_());
	}
	if (poutA) free(poutA);
	return outlen;
}

QyUrlDecodeW::QyUrlDecodeW(const char* pin,  size_t len)
:QyCodecT<wchar_t>(L"")
{
    length_ = qycodec_urlDecodeW(&out_,pin,len);
}


QyUrlDecodeA::QyUrlDecodeA(const char* pin,  size_t len)
:QyCodecT<char>("")
{
	length_ = qycodec_urlDecodeA(&out_,pin,len);
}

//
QyUrlEncodeW::QyUrlEncodeW(const char* pin,  size_t len)
:QyCodecT<wchar_t>(L"")
{
	length_ = qycodec_urlEncodeW(&out_,pin,len);
}

//
QyUrlEncodeA::QyUrlEncodeA(const char* pin,  size_t len)
:QyCodecT<char>("")
{
	length_ = qycodec_urlEncodeA(&out_,pin,len);
}

END_NAMESPACE(qy)