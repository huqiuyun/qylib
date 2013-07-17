/**
*  @brief String opeartion.
*  @file  tqcodec.h
*  @author loach 
*  @Email < loachmr@sina.com >
*
* Copyright (C) 1996-2010 loach, All Rights Reserved
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

#pragma once

#include "qyutil/qyconfig.h"
#include <string>
#include <list>

DEFINE_NAMESPACE(qy)

#ifndef WIN32

#define CP_ACP        0
/*The system default Windows ANSI code page.
Note  This value can be different on different computers, even on the same network. It can be changed on the same computer, leading to stored data becoming irrecoverably corrupted. This value is only intended for temporary use and permanent storage should use UTF-16 or UTF-8 if possible.
*/


#define CP_MACCP      1
/*The current system Macintosh code page.
Note  This value can be different on different computers, even on the same network. It can be changed on the same computer, leading to stored data becoming irrecoverably corrupted. This value is only intended for temporary use and permanent storage should use UTF-16 or UTF-8 if possible.
Note   This value is used primarily in legacy code and should not generally be needed since modern Macintosh computers use Unicode for encoding.
*/

#define CP_OEMCP      2

/*The current system OEM code page.
Note  This value can be different on different computers, even on the same network. It can be changed on the same computer, leading to stored data becoming irrecoverably corrupted. This value is only intended for temporary use and permanent storage should use UTF-16 or UTF-8 if possible.
*/

#define CP_SYMBOL     3
/*Windows 2000: Symbol code page (42).
CP_THREAD_ACP
Windows 2000: The Windows ANSI code page for the current thread.
Note  This value can be different on different computers, even on the same network. It can be changed on the same computer, leading to stored data becoming irrecoverably corrupted. This value is only intended for temporary use and permanent storage should use UTF-16 or UTF-8 if possible.
*/

#define CP_UTF7       4
/*UTF-7. Use this value only when forced by a 7-bit transport mechanism. Use of UTF-8 is preferred. With this value set, lpDefaultChar and lpUsedDefaultChar must be set to NULL.
*/

#define CP_UTF8       5
/*UTF-8. With this value set, lpDefaultChar and lpUsedDefaultChar must be set to NULL.*/


#endif //!WIN32

template<typename STL , typename C_STL>
class QYUTIL_API QyReadSTL{
public:
	const C_STL* c_str() const{
		return outstr_.c_str();
	}
	size_t length() const{
		return outstr_.length();
	}
	STL data() const{
		return outstr_;
	}
protected:
	STL outstr_;
};

template <typename CHAR>
class QYUTIL_API  QyCodecT{
public:
    QyCodecT(const CHAR* def)
    :out_(0)
    ,length_(0)
    ,def_(def){
    }
    
    ~QyCodecT(){
        free(out_);
    }
 	const CHAR* c_str() const{
		return out_?out_:def_;
	}
	size_t length() const{
		return length_;
	}
protected:
    const CHAR* def_;
    CHAR*  out_;
    size_t length_;
};

QYUTIL_API bool           qycodec_isemail(const char *str);
QYUTIL_API bool           qycodec_isphone(const char *str);
QYUTIL_API bool           qycodec_isalpha(const char *str);
QYUTIL_API bool           qycodec_isalnum(const char* str);
QYUTIL_API bool           qycodec_isdigit(const char *str);

QYUTIL_API const wchar_t* qycodec_extractStringW( const std::wstring &seperate, const std::wstring &in, std::wstring &out);
QYUTIL_API const char*    qycodec_extractStringA( const std::string &seperate, const std::string &in, std::string &out);

QYUTIL_API void           qycodec_toLower(std::wstring& str);
QYUTIL_API void           qycodec_toUpper(std::wstring& str);
QYUTIL_API void           qycodec_trim( std::wstring& strReturn, std::wstring const& str, std::wstring const& filter);
QYUTIL_API void           qycodec_replace(std::wstring &replaceStr, std::wstring const &srcStr, std::wstring const &desStr);
QYUTIL_API void           qycodec_splitWString(const std::wstring &stringOrg, const std::wstring & delimiter, std::list<std::wstring> &result);
QYUTIL_API void           qycodec_splitCString(const std::string &stringOrg, const std::string & delimiter, std::list<std::wstring> &result);

// HEX TO BIN | BIN TO HEX
QYUTIL_API const wchar_t* qycodec_c2HexW(wchar_t outstr[3] , unsigned char c);
QYUTIL_API const char*    qycodec_c2HexA(char outstr[3] , unsigned char c);

QYUTIL_API const char*    qycodec_hex2BinA(std::string & outstr,const char * str, size_t len);
QYUTIL_API const wchar_t* qycodec_hex2BinW(std::wstring & outstr,const char* str, size_t len);

QYUTIL_API const char*    qycodec_bin2HexA(std::string& outstr, const char* bin, size_t len);
QYUTIL_API const wchar_t* qycodec_bin2HexW(std::wstring& outstr, const char* bin, size_t len);

class QYUTIL_API QyHexToBinA : public QyReadSTL<std::string ,char>{
public:
	QyHexToBinA(const char * str , size_t len){
		qycodec_hex2BinA(outstr_, str,len);
	}
};

class QYUTIL_API QyHexToBinW : public QyReadSTL<std::wstring ,wchar_t>{
public:
	QyHexToBinW(const char * str , size_t len){
		qycodec_hex2BinW(outstr_ ,str,len);
	}
};

class QYUTIL_API QyBinToHexW : public QyReadSTL<std::wstring ,wchar_t>{
public:
	QyBinToHexW(const char * str , size_t len){
		qycodec_bin2HexW(outstr_ ,str,len);
	}
};

class QYUTIL_API QyBinToHexA: public QyReadSTL<std::string ,char>{
public:
	QyBinToHexA(const char * str , size_t len){
		qycodec_bin2HexA(outstr_ ,str,len);
	}
};

/*
** Convert an ansi string to microsoft unicode, based on the
** current codepage settings for file apis.
** 
** Space to hold the returned string is obtained
** from malloc. please free
*/
QYUTIL_API size_t         qycodec_c2W(wchar_t** pout, const char *in, int codepage);
QYUTIL_API size_t         qycodec_w2C(char** pout, const wchar_t *in, int codepage);
QYUTIL_API size_t         qycodec_w2Utf8(char** pout, const wchar_t *in);
QYUTIL_API size_t         qycodec_utf82W(wchar_t** pout, const char* utf8);
QYUTIL_API size_t         qycodec_c2Utf8(char** pout, const char *in);
QYUTIL_API size_t         qycodec_utf82C(char** pout, const char *utf8);
QYUTIL_API const wchar_t* qycodec_c2WString(std::wstring & outstr, const char* pStr, size_t len, unsigned int codepage = CP_ACP);
QYUTIL_API const char*    qycodec_w2CString(std::string & outstr, const wchar_t* pStr, size_t len, unsigned int codepage = CP_ACP);

/*
 ** Convert microsoft unicode to UTF-8.  Space to hold the returned string is
 */
QYUTIL_API const char*     qycodec_wToUtf8( std::string& outstr, const wchar_t* wstr);

/** Convert  utf8 to microsoft unicode
 */
QYUTIL_API const wchar_t*  qycodec_utf8ToW( std::wstring& woutstr,const char* utf8);

/*
 ** Convert an ansi string to microsoft unicode, based on the
 ** current codepage settings for file apis.
 **
 ** Space to hold the returned string is obtained
 ** from malloc.
 */
QYUTIL_API const wchar_t*  qycodec_mbcsToW(std::wstring& woutstr , const char *zstr);

/*
 ** Convert microsoft unicode to multibyte character string, based on the
 ** user's Ansi codepage.
 **
 ** Space to hold the returned string is obtained from
 ** malloc().
 */
QYUTIL_API const char*     qycodec_wToMbcs(std::string& outstr ,const wchar_t *zwstr);


/*
 ** Convert multibyte character string to UTF-8.  Space to hold the
 ** returned string is obtained from malloc().
 */
QYUTIL_API const char*     qycodec_mbcsToUtf8(std::string& outstr , const char *zstr);

/*
 ** Convert UTF-8 to multibyte character string.  Space to hold the
 ** returned string is obtained from malloc().
 */
QYUTIL_API const char*     qycodec_utf8ToMbcs(std::string& outstr , const char *zstr);


class QYUTIL_API QyC2W : public QyCodecT<wchar_t>{
public:
	QyC2W(const char* z);
};

/** @param pout free.. */
class QYUTIL_API QyW2C : public QyCodecT<char>{
public:
	QyW2C(const wchar_t* z);
};

/** @param pout free.. */
class QYUTIL_API QyW2UTF8 : public QyCodecT<char>{
public:
	QyW2UTF8(const wchar_t* z);
};

/** @param pout free.. */
class QYUTIL_API QyUTF82W : public QyCodecT<wchar_t>{
public:
	QyUTF82W(const char* z);
};

/*
** Convert multibyte character string to UTF-8.  Space to hold the
** returned string is obtained from malloc().
*/
class QYUTIL_API QyC2UTF8 : public QyCodecT<char>{
public:
	QyC2UTF8(const char* z);
};

/** @param pout free.. */
class QYUTIL_API QyUTF82C : public QyCodecT<char>{
public:
	QyUTF82C(const char* z);
};

//
class QYUTIL_API Qy2WString : public QyReadSTL<std::wstring ,wchar_t>{
public:
	Qy2WString(const char* pStr, size_t len , unsigned int CodePage = CP_ACP){
		qycodec_c2WString(outstr_,pStr , len , CodePage);
	}
};

class QYUTIL_API Qy2CString : public QyReadSTL<std::string ,char>{
public:
	Qy2CString(const wchar_t* pStr, size_t len,unsigned int CodePage = CP_ACP){
		qycodec_w2CString(outstr_,pStr , len , CodePage);
	}
};

// url
QYUTIL_API size_t qycodec_urlEncodeW(wchar_t** pout, const char* pin,  size_t len);
QYUTIL_API size_t qycodec_urlEncodeA(char** pout, const char* pin,  size_t len);
QYUTIL_API size_t qycodec_urlDecodeW(wchar_t** pout,const char* pin,  size_t len);
QYUTIL_API size_t qycodec_urlDecodeA(char** pout,const char* pin, size_t len);

class QYUTIL_API QyUrlEncodeW : public QyCodecT<wchar_t>{
public:
	QyUrlEncodeW(const char* pin,  size_t len);
};

class QYUTIL_API QyUrlEncodeA : public QyCodecT<char>{
public:
	QyUrlEncodeA(const char* pin,  size_t len);
};

class QYUTIL_API QyUrlDecodeW : public QyCodecT<wchar_t>{
public:
	QyUrlDecodeW(const char* pin,  size_t len);
};

class QYUTIL_API QyUrlDecodeA : public QyCodecT<char>{
public:
	QyUrlDecodeA(const char* pin,  size_t len);
};

END_NAMESPACE(qy)