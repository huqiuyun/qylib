/**
* 功能: 以QyMD5算法加密数据，加密后的数据可以为二进制，也可以为文本
* 整理: 陈中华
* 日期: 2001.10.3
*/

// QyMD5.CC - source code for the C++/object oriented translation and 
//          modification of QyMD5.

// Translation and modification (c) 1995 by Mordechai T. Abzug 

// This translation/ modification is provided "as is," without express or 
// implied warranty of any kind.

// The translator/ modifier does not claim (1) that QyMD5 will do what you think 
// it does; (2) that this translation/ modification is accurate; or (3) that 
// this software is "merchantible."  (Language for this disclaimer partially 
// copied from the disclaimer below).

/* based on:

QyMD5.H - header file for MD5C.C
MDDRIVER.C - test driver for MD2, MD4 and QyMD5

Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. QyMD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. QyMD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.

*/

#ifndef __QY_MD5_H__
#define __QY_MD5_H__

#include "qyutil/qyconfig.h"

class QYUTIL_API QyMD5
{
public:
	// methods for controlled operation:
	QyMD5              ();  // simple initializer
	void  update     (unsigned char *input, unsigned int input_length);
	void  finalize   ();

	// constructors for special circumstances.  All these constructors finalize
	// the QyMD5 context.
	QyMD5              (unsigned char *string); // digest string, finalize

	// methods to acquire finalized result
	unsigned char    *raw_digest (unsigned char rawDigiest[16]);	//	digest as a 16-byte binary array(没有'\0'结束!!!)
	char *            hex_digest (char hexDigiest[33]);			//	digest as a 33-byte ascii-hex string('\0'结束)

private:

	// first, some types:
	typedef unsigned       int quint4; // assumes integer is 4 words long
	typedef unsigned short int quint2; // assumes short integer is 2 words long
	typedef unsigned      char quint1; // assumes char is 1 word long

	// next, the private data:
	quint4 state[4];
	quint4 count[2];     // number of *bits*, mod 2^64
	quint1 buffer[64];   // input buffer
	quint1 digest[16];
	quint1 finalized;

	// last, the private methods, mostly static:
	void init             ();               // called by all constructors
	void transform        (quint1 *buffer);  // does the real update work.  Note 
	// that length is implied to be 64.

	static void encode    (quint1 *dest, quint4 *src, quint4 length);
	static void decode    (quint4 *dest, quint1 *src, quint4 length);
	static void memcpy    (quint1 *dest, quint1 *src, quint4 length);
	static void memset    (quint1 *start, quint1 val, quint4 length);

	static inline quint4  rotate_left (quint4 x, quint4 n);
	static inline quint4  F           (quint4 x, quint4 y, quint4 z);
	static inline quint4  G           (quint4 x, quint4 y, quint4 z);
	static inline quint4  H           (quint4 x, quint4 y, quint4 z);
	static inline quint4  I           (quint4 x, quint4 y, quint4 z);
	static inline void   FF  (quint4& a, quint4 b, quint4 c, quint4 d, quint4 x, 
		quint4 s, quint4 ac);
	static inline void   GG  (quint4& a, quint4 b, quint4 c, quint4 d, quint4 x, 
		quint4 s, quint4 ac);
	static inline void   HH  (quint4& a, quint4 b, quint4 c, quint4 d, quint4 x, 
		quint4 s, quint4 ac);
	static inline void   II  (quint4& a, quint4 b, quint4 c, quint4 d, quint4 x, 
		quint4 s, quint4 ac);

};

#endif	//	__QY_MD5_H__
