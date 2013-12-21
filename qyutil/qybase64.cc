/*
Copyright (c) 2005-2007 by Jakob Schroeter <js@camaya.net>
This file is part of the gloox library. http://camaya.net/gloox

This software is distributed under a license. The full license
agreement can be found in the file LICENSE in this distribution.
This software may not be copied, modified, sold or distributed
other than expressed in the named license agreement.

This software is distributed without any warranty.
*/

#include "qybase64.h"
#include <stdlib.h>

DEFINE_NAMESPACE(qy)

const char QyBase64::alphabet64[] ="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char QyBase64::Base64Table[65]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"; 
const char QyBase64::pad = '=';

const std::string::size_type QyBase64::np = std::string::npos;
const std::string::size_type QyBase64::table64[] =
{
	np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np,
	np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np,
	np, np, np, 62, np, np, np, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, np, np,
	np, np, np, np, np,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, np, np, np, np, np, np, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
	49, 50, 51, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np,
	np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np,
	np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np,
	np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np,
	np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np,
	np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np,
	np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np
};

const std::string QyBase64::encode64( const std::string& input)
{
	std::string encoded;
	char c;
	const std::string::size_type length = input.length();

	encoded.reserve( length * 2);

	for( std::string::size_type i = 0; i < length; ++i)
	{
		c =(input[i] >> 2) & 0x3f;
		encoded.append( 1, alphabet64[c]);

		c =(input[i] << 4) & 0x3f;
		if (++i < length)
			c |=(( input[i] >> 4) & 0x0f);
		encoded.append( 1, alphabet64[c]);

		if (i < length)
		{
			c =(input[i] << 2) & 0x3c;
			if (++i < length)
				c |=(input[i] >> 6) & 0x03;
			encoded.append( 1, alphabet64[c]);
		}
		else
		{
			++i;
			encoded.append( 1, pad);
		}

		if (i < length)
		{
			c = input[i] & 0x3f;
			encoded.append( 1, alphabet64[c]);
		}
		else
		{
			encoded.append( 1, pad);
		}
	}

	return encoded;
}

const std::string QyBase64::decode64( const std::string& input)
{
	char c, d;
	const std::string::size_type length = input.length();
	std::string decoded;

	decoded.reserve( length);

	for( std::string::size_type i = 0; i < length; ++i)
	{
		c = (char)table64[(unsigned char)input[i]];
		++i;
		d = (char)table64[(unsigned char)input[i]];
		c =(c << 2) |(( d >> 4) & 0x3);
		decoded.append( 1, c);
		if (++i < length)
		{
			c = input[i];
			if (pad == c)
				break;

			c = (char)table64[(unsigned char)input[i]];
			d =(( d << 4) & 0xf0) |(( c >> 2) & 0xf);
			decoded.append( 1, d);
		}

		if (++i < length)
		{
			d = input[i];
			if (pad == d)
				break;

			d = (char)table64[(unsigned char)input[i]];
			c =(( c << 6) & 0xc0) | d;
			decoded.append( 1, c);
		}
	}

	return decoded;
}

int  QyBase64::Base64Encod(const char *buftoenc,int bufsize,char *encbuf) 
{ 
	int i=0; 
	int b64byte[5]; 
	unsigned char *buftemp; 

	//Allocate space for the temporary string 
	buftemp=(unsigned char *)malloc(bufsize+10); 
	memcpy(buftemp,buftoenc,bufsize); 
	if (bufsize%3==1) 
	{ 
		buftemp[bufsize]='\0'; 
		buftemp[bufsize+1]='\0'; 
	} 
	if (bufsize%3==2)buftemp[bufsize]='\0'; 
	while (i<bufsize) 
	{ 
		b64byte[0]=buftemp[i]>>2; 
		b64byte[1]=((buftemp[i]&3)<<4)|(buftemp[i+1]>>4); 
		b64byte[2]=((buftemp[i+1]&0x0F)<<2)|(buftemp[i+2]>>6); 
		b64byte[3]=buftemp[i+2]&0x3F; 
		encbuf[i+(i/3)]=Base64Table[b64byte[0]]; 
		encbuf[i+(i/3)+1]=Base64Table[b64byte[1]]; 
		encbuf[i+(i/3)+2]=Base64Table[b64byte[2]]; 
		encbuf[i+(i/3)+3]=Base64Table[b64byte[3]]; 
		i+=3; 
	} 
	free(buftemp); 
	if (bufsize%3==0)return bufsize*8/6; 
	if (bufsize%3==1)return((bufsize+2)*8/6)-2; 
	if (bufsize%3==2)return((bufsize+1)*8/6)-1; 
	return -1; 
} 

//Base64Decod
//Encode string
//Param: buftodec,input string to decode
//       bufsize,input string length
//       decbug, output string decoded
//Return value:Decoded string length??? if fail return -1???
int QyBase64::Base64Decod(const char *buftodec,int bufsize,char *decbuf) 
{ 
	int i=0; 
	unsigned char binbyte[4]; 
	long cpos[5];
	unsigned char *buftemp; 

	//Allocate space for the temporary string 
	buftemp=(unsigned char *)malloc(bufsize+10); 
	memcpy(buftemp,buftodec,bufsize); 
	if (bufsize%4==1) 
	{ 
		buftemp[bufsize]='\0'; 
		buftemp[bufsize+1]='\0'; 
		buftemp[bufsize+2]='\0'; 
	} 
	if (bufsize%4==2) 
	{ 
		buftemp[bufsize]='\0'; 
		buftemp[bufsize+1]='\0'; 
	} 
	if (bufsize%4==3)buftemp[bufsize]='\0'; 
	while (i<bufsize) 
	{ 
		cpos[0]=strchr(Base64Table,buftemp[i])-Base64Table; 
		cpos[1]=strchr(Base64Table,buftemp[i+1])-Base64Table; 
		cpos[2]=strchr(Base64Table,buftemp[i+2])-Base64Table; 
		cpos[3]=strchr(Base64Table,buftemp[i+3])-Base64Table; 
		binbyte[0]=((cpos[0]<<2)|(cpos[1]>>4)); 
		binbyte[1]=((cpos[1]<<4)|(cpos[2]>>2)); 
		binbyte[2]=(((cpos[2]&0x03)<<6)|(cpos[3]&0x3f)); 
		decbuf[i-(i/4)]=binbyte[0]; 
		decbuf[i-(i/4)+1]=binbyte[1]; 
		decbuf[i-(i/4)+2]=binbyte[2]; 
		i+=4; 
	} 
	free(buftemp); 
	if (bufsize%4==0)return bufsize*6/8; 
	if (bufsize%4==1)return((bufsize+3)*6/8)-3; 
	if (bufsize%4==2)return((bufsize+2)*6/8)-2; 
	if (bufsize%4==3)return((bufsize+1)*6/8)-1; 
	return -1; 
} 

END_NAMESPACE(qy)
