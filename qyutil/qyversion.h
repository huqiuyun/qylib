/**
*  @brief String opeartion.
*  @file  qyversion.h
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
#ifndef __QY_VERSION_H__
#define __QY_VERSION_H__

#include "qyutil/qyconfig.h"

DEFINE_NAMESPACE(qy)

class QyVersionPrivate;
/** 版本 
 * 
 * @author by loach(胡秋云)
 * @date 2009-09-08 
 *
*/
class QYUTIL_API QyVersion
{
public:
	QyVersion();
	~QyVersion();

	/**
	* @brief 读取资源的版本信息
	*/
	BOOL read(HINSTANCE hResource);

	/**
	* @brief 读取一个文件夹的版本信息
	*/
	BOOL read(const wchar_t* sFile);
	/**
	* @param nVer   == 1  高版本号
	*               == 2  低版本号
	*               == -1 获取所有
	*/
	const wchar_t* getVersionString(int nVer=-1);

	const wchar_t* getLegalCopyright(void) const;
	const wchar_t* getPrivateBuild(void) const;
	const wchar_t* getSpecialBuild(void) const;

	unsigned long getMS(void) const;
	unsigned long getLS(void) const;

	static BOOL string2Word(const wchar_t* lpszVer,unsigned long& dwMS,unsigned long& dwLS);

	enum Win32SysType
	{  
		WindowsNo = -1,///< 无效
		Windows32s,  
		WindowsNT3,  
		Windows95,  
		Windows98,  
		WindowsME,  
		WindowsNT4,  
		Windows2000,  
		WindowsXP,
		WindowsVista,
	};
	static Win32SysType getShellType();

private:
	friend class QyVersionPrivate;
	QyVersionPrivate* d_ptr_;
};

END_NAMESPACE(qy)

#endif //__QY_VERSION_H__