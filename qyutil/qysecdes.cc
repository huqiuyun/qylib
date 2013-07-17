#include <stdlib.h>
#include "qysecdes.h"

DEFINE_NAMESPACE(qy)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define int8    char
#define quint8   unsigned char 
#define quint16  unsigned short 
#define quint32  unsigned long 

////////////////////////////////////////////////////////////

#define ENCRYPT  0
#define DECRYPT  1

////////////////////////////////////////////////////////////

static quint32 KnL[32];

static quint16 bytebit[8] = {
	0200, 0100, 040, 020, 010, 04, 02, 01 
};

static quint32 bigbyte[24] = {
	0x800000L, 0x400000L, 0x200000L, 0x100000L,
	0x80000L,  0x40000L,  0x20000L,  0x10000L,
	0x8000L,   0x4000L,   0x2000L,   0x1000L,
	0x800L,    0x400L,    0x200L,    0x100L,
	0x80L,     0x40L,     0x20L,     0x10L,
	0x8L,      0x4L,      0x2L,      0x1L        
};

static quint8 pc1[56] = {
	56, 48, 40, 32, 24, 16,  8,  0, 57, 49, 41, 33, 25, 17,
	9,  1, 58, 50, 42, 34, 26, 18, 10,  2, 59, 51, 43, 35,
	62, 54, 46, 38, 30, 22, 14,  6, 61, 53, 45, 37, 29, 21,
	13,  5, 60, 52, 44, 36, 28, 20, 12,  4, 27, 19, 11, 3   
};

static quint8 totrot[16] = {
	1, 2, 4, 6, 8, 10, 12, 14, 15, 17, 19, 21, 23, 25, 27, 28 
};

static quint8 pc2[48] = {
	13, 16, 10, 23,  0,  4,      2, 27, 14,  5, 20,  9,
	22, 18, 11,  3, 25,  7,     15,  6, 26, 19, 12,  1,
	40, 51, 30, 36, 46, 54,     29, 39, 50, 44, 32, 47,
	43, 48, 38, 55, 33, 52,     45, 41, 49, 35, 28, 31   
};

static quint32 SP1[64] = {
	0x01010400L, 0x00000000L, 0x00010000L, 0x01010404L,
	0x01010004L, 0x00010404L, 0x00000004L, 0x00010000L,
	0x00000400L, 0x01010400L, 0x01010404L, 0x00000400L,
	0x01000404L, 0x01010004L, 0x01000000L, 0x00000004L,
	0x00000404L, 0x01000400L, 0x01000400L, 0x00010400L,
	0x00010400L, 0x01010000L, 0x01010000L, 0x01000404L,
	0x00010004L, 0x01000004L, 0x01000004L, 0x00010004L,
	0x00000000L, 0x00000404L, 0x00010404L, 0x01000000L,
	0x00010000L, 0x01010404L, 0x00000004L, 0x01010000L,
	0x01010400L, 0x01000000L, 0x01000000L, 0x00000400L,
	0x01010004L, 0x00010000L, 0x00010400L, 0x01000004L,
	0x00000400L, 0x00000004L, 0x01000404L, 0x00010404L,
	0x01010404L, 0x00010004L, 0x01010000L, 0x01000404L,
	0x01000004L, 0x00000404L, 0x00010404L, 0x01010400L,
	0x00000404L, 0x01000400L, 0x01000400L, 0x00000000L,
	0x00010004L, 0x00010400L, 0x00000000L, 0x01010004L  
};

static quint32 SP2[64] = {
	0x80108020L, 0x80008000L, 0x00008000L, 0x00108020L,
	0x00100000L, 0x00000020L, 0x80100020L, 0x80008020L,
	0x80000020L, 0x80108020L, 0x80108000L, 0x80000000L,
	0x80008000L, 0x00100000L, 0x00000020L, 0x80100020L,
	0x00108000L, 0x00100020L, 0x80008020L, 0x00000000L,
	0x80000000L, 0x00008000L, 0x00108020L, 0x80100000L,
	0x00100020L, 0x80000020L, 0x00000000L, 0x00108000L,
	0x00008020L, 0x80108000L, 0x80100000L, 0x00008020L,
	0x00000000L, 0x00108020L, 0x80100020L, 0x00100000L,
	0x80008020L, 0x80100000L, 0x80108000L, 0x00008000L,
	0x80100000L, 0x80008000L, 0x00000020L, 0x80108020L,
	0x00108020L, 0x00000020L, 0x00008000L, 0x80000000L,
	0x00008020L, 0x80108000L, 0x00100000L, 0x80000020L,
	0x00100020L, 0x80008020L, 0x80000020L, 0x00100020L,
	0x00108000L, 0x00000000L, 0x80008000L, 0x00008020L,
	0x80000000L, 0x80100020L, 0x80108020L, 0x00108000L  
};

static quint32 SP3[64] = {
	0x00000208L, 0x08020200L, 0x00000000L, 0x08020008L,
	0x08000200L, 0x00000000L, 0x00020208L, 0x08000200L,
	0x00020008L, 0x08000008L, 0x08000008L, 0x00020000L,
	0x08020208L, 0x00020008L, 0x08020000L, 0x00000208L,
	0x08000000L, 0x00000008L, 0x08020200L, 0x00000200L,
	0x00020200L, 0x08020000L, 0x08020008L, 0x00020208L,
	0x08000208L, 0x00020200L, 0x00020000L, 0x08000208L,
	0x00000008L, 0x08020208L, 0x00000200L, 0x08000000L,
	0x08020200L, 0x08000000L, 0x00020008L, 0x00000208L,
	0x00020000L, 0x08020200L, 0x08000200L, 0x00000000L,
	0x00000200L, 0x00020008L, 0x08020208L, 0x08000200L,
	0x08000008L, 0x00000200L, 0x00000000L, 0x08020008L,
	0x08000208L, 0x00020000L, 0x08000000L, 0x08020208L,
	0x00000008L, 0x00020208L, 0x00020200L, 0x08000008L,
	0x08020000L, 0x08000208L, 0x00000208L, 0x08020000L,
	0x00020208L, 0x00000008L, 0x08020008L, 0x00020200L  
};

static quint32 SP4[64] = {
	0x00802001L, 0x00002081L, 0x00002081L, 0x00000080L,
	0x00802080L, 0x00800081L, 0x00800001L, 0x00002001L,
	0x00000000L, 0x00802000L, 0x00802000L, 0x00802081L,
	0x00000081L, 0x00000000L, 0x00800080L, 0x00800001L,
	0x00000001L, 0x00002000L, 0x00800000L, 0x00802001L,
	0x00000080L, 0x00800000L, 0x00002001L, 0x00002080L,
	0x00800081L, 0x00000001L, 0x00002080L, 0x00800080L,
	0x00002000L, 0x00802080L, 0x00802081L, 0x00000081L,
	0x00800080L, 0x00800001L, 0x00802000L, 0x00802081L,
	0x00000081L, 0x00000000L, 0x00000000L, 0x00802000L,
	0x00002080L, 0x00800080L, 0x00800081L, 0x00000001L,
	0x00802001L, 0x00002081L, 0x00002081L, 0x00000080L,
	0x00802081L, 0x00000081L, 0x00000001L, 0x00002000L,
	0x00800001L, 0x00002001L, 0x00802080L, 0x00800081L,
	0x00002001L, 0x00002080L, 0x00800000L, 0x00802001L,
	0x00000080L, 0x00800000L, 0x00002000L, 0x00802080L  
};

static quint32 SP5[64] = {
	0x00000100L, 0x02080100L, 0x02080000L, 0x42000100L,
	0x00080000L, 0x00000100L, 0x40000000L, 0x02080000L,
	0x40080100L, 0x00080000L, 0x02000100L, 0x40080100L,
	0x42000100L, 0x42080000L, 0x00080100L, 0x40000000L,
	0x02000000L, 0x40080000L, 0x40080000L, 0x00000000L,
	0x40000100L, 0x42080100L, 0x42080100L, 0x02000100L,
	0x42080000L, 0x40000100L, 0x00000000L, 0x42000000L,
	0x02080100L, 0x02000000L, 0x42000000L, 0x00080100L,
	0x00080000L, 0x42000100L, 0x00000100L, 0x02000000L,
	0x40000000L, 0x02080000L, 0x42000100L, 0x40080100L,
	0x02000100L, 0x40000000L, 0x42080000L, 0x02080100L,
	0x40080100L, 0x00000100L, 0x02000000L, 0x42080000L,
	0x42080100L, 0x00080100L, 0x42000000L, 0x42080100L,
	0x02080000L, 0x00000000L, 0x40080000L, 0x42000000L,
	0x00080100L, 0x02000100L, 0x40000100L, 0x00080000L,
	0x00000000L, 0x40080000L, 0x02080100L, 0x40000100L  
};

static quint32 SP6[64] = {
	0x20000010L, 0x20400000L, 0x00004000L, 0x20404010L,
	0x20400000L, 0x00000010L, 0x20404010L, 0x00400000L,
	0x20004000L, 0x00404010L, 0x00400000L, 0x20000010L,
	0x00400010L, 0x20004000L, 0x20000000L, 0x00004010L,
	0x00000000L, 0x00400010L, 0x20004010L, 0x00004000L,
	0x00404000L, 0x20004010L, 0x00000010L, 0x20400010L,
	0x20400010L, 0x00000000L, 0x00404010L, 0x20404000L,
	0x00004010L, 0x00404000L, 0x20404000L, 0x20000000L,
	0x20004000L, 0x00000010L, 0x20400010L, 0x00404000L,
	0x20404010L, 0x00400000L, 0x00004010L, 0x20000010L,
	0x00400000L, 0x20004000L, 0x20000000L, 0x00004010L,
	0x20000010L, 0x20404010L, 0x00404000L, 0x20400000L,
	0x00404010L, 0x20404000L, 0x00000000L, 0x20400010L,
	0x00000010L, 0x00004000L, 0x20400000L, 0x00404010L,
	0x00004000L, 0x00400010L, 0x20004010L, 0x00000000L,
	0x20404000L, 0x20000000L, 0x00400010L, 0x20004010L  
};

static quint32 SP7[64] = {
	0x00200000L, 0x04200002L, 0x04000802L, 0x00000000L,
	0x00000800L, 0x04000802L, 0x00200802L, 0x04200800L,
	0x04200802L, 0x00200000L, 0x00000000L, 0x04000002L,
	0x00000002L, 0x04000000L, 0x04200002L, 0x00000802L,
	0x04000800L, 0x00200802L, 0x00200002L, 0x04000800L,
	0x04000002L, 0x04200000L, 0x04200800L, 0x00200002L,
	0x04200000L, 0x00000800L, 0x00000802L, 0x04200802L,
	0x00200800L, 0x00000002L, 0x04000000L, 0x00200800L,
	0x04000000L, 0x00200800L, 0x00200000L, 0x04000802L,
	0x04000802L, 0x04200002L, 0x04200002L, 0x00000002L,
	0x00200002L, 0x04000000L, 0x04000800L, 0x00200000L,
	0x04200800L, 0x00000802L, 0x00200802L, 0x04200800L,
	0x00000802L, 0x04000002L, 0x04200802L, 0x04200000L,
	0x00200800L, 0x00000000L, 0x00000002L, 0x04200802L,
	0x00000000L, 0x00200802L, 0x04200000L, 0x00000800L,
	0x04000002L, 0x04000800L, 0x00000800L, 0x00200002L  
};

static quint32 SP8[64] = {
	0x10001040L, 0x00001000L, 0x00040000L, 0x10041040L,
	0x10000000L, 0x10001040L, 0x00000040L, 0x10000000L,
	0x00040040L, 0x10040000L, 0x10041040L, 0x00041000L,
	0x10041000L, 0x00041040L, 0x00001000L, 0x00000040L,
	0x10040000L, 0x10000040L, 0x10001000L, 0x00001040L,
	0x00041000L, 0x00040040L, 0x10040040L, 0x10041000L,
	0x00001040L, 0x00000000L, 0x00000000L, 0x10040040L,
	0x10000040L, 0x10001000L, 0x00041040L, 0x00040000L,
	0x00041040L, 0x00040000L, 0x10041000L, 0x00001000L,
	0x00000040L, 0x10040040L, 0x00001000L, 0x00041040L,
	0x10001000L, 0x00000040L, 0x10000040L, 0x10040000L,
	0x10040040L, 0x10000000L, 0x00040000L, 0x10001040L,
	0x00000000L, 0x10041040L, 0x00040040L, 0x10000040L,
	0x10040000L, 0x10001000L, 0x10001040L, 0x00000000L,
	0x10041040L, 0x00041000L, 0x00041000L, 0x00001040L,
	0x00001040L, 0x00040040L, 0x10000000L, 0x10041000L  
};

////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------
// usekey
//       Loads the interal key register with the data in cookedkey.
// -----------------------------------------------------------------------
static void usekey(register quint32 *from)
{
	register quint32 *to, *endp;

	to = KnL, endp = &KnL[32];
	while (to < endp) 
		*to++ = *from++;
}

// ----------------------------------------------------------------------
// cookey
//       Only called by deskey.
// -----------------------------------------------------------------------
static void cookey(register quint32 *raw1)
{
	register quint32 *cook, *raw0;
	quint32 dough[32];
	register int i;

	cook = dough;
	for (i = 0; i < 16; i++, raw1++) {
		raw0 = raw1++;
		*cook = (*raw0 & 0x00fc0000L) << 6;
		*cook |= (*raw0 & 0x00000fc0L) << 10;
		*cook |= (*raw1 & 0x00fc0000L) >> 10;
		*cook++ |= (*raw1 & 0x00000fc0L) >> 6;
		*cook = (*raw0 & 0x0003f000L) << 12;
		*cook |= (*raw0 & 0x0000003fL) << 16;
		*cook |= (*raw1 & 0x0003f000L) >> 4;
		*cook++ |= (*raw1 & 0x0000003fL);
	}
	usekey(dough);
}

// ----------------------------------------------------------------------
// deskey
//       Sets the internal key register (KnR) according to the hexadecimal
//       key contained in the 8 bytes of hexkey, according to the DES,
//       for encryption or decrytion according to MODE
//
//       "key" is the 64 bits key.
//       "mode"  means encryption or decryption.
//       Thanks to James Gillogly &amp; Phil Karn! 
// ----------------------------------------------------------------------

static void deskey(quint8 key[8], int mode)  
{
	register int i, j, l, m, n;
	quint8 pc1m[56], pcr[56];
	quint32 kn[32];

	for (j = 0; j < 56; j++) 
	{
		l = pc1[j];
		m = l & 07;
		pc1m[j] = (key[l >> 3] & bytebit[m]) ? 1:0;
	}

	for (i = 0; i < 16; i++) 
	{
		if (mode == DECRYPT) 
			m = (15 - i) << 1;
		else 
			m = i << 1;
		n = m + 1;
		kn[m] = kn[n] = 0L;

		for (j = 0; j < 28; j++) 
		{
			l = j + totrot[i];
			if (l < 28) 
				pcr[j] = pc1m[l];
			else 
				pcr[j] = pc1m[l - 28];

		}

		for (j = 28; j < 56; j++) 
		{
			l = j + totrot[i];
			if (l < 56) 
				pcr[j] = pc1m[l];
			else 
				pcr[j] = pc1m[l - 28];
		}

		for (j = 0; j < 24; j++) 
		{
			if (pcr[ pc2[j] ]) 
				kn[m] |= bigbyte[j];
			if (pcr[ pc2[j+24] ]) 
				kn[n] |= bigbyte[j];
		}
	}

	cookey(kn);
}

static void scrunch(register quint8 *outof, register quint32 *into)
{
	*into = (*outof++ & 0xffL) << 24;
	*into |= (*outof++ & 0xffL) << 16;
	*into |= (*outof++ & 0xffL) << 8;
	*into++ |= (*outof++ & 0xffL);
	*into = (*outof++ & 0xffL) << 24;
	*into |= (*outof++ & 0xffL) << 16;
	*into |= (*outof++ & 0xffL) << 8;
	*into |= (*outof & 0xffL);
}

static void unscrun(register quint32 *outof, register quint8 *into)
{
	*into++ = (quint8)( (*outof >> 24) & 0xffL);
	*into++ = (quint8)( (*outof >> 16) & 0xffL);
	*into++ = (quint8)( (*outof >> 8) & 0xffL);
	*into++ = (quint8)( *outof++ & 0xffL);
	*into++ = (quint8)( (*outof >> 24) & 0xffL);
	*into++ = (quint8)( (*outof >> 16) & 0xffL);
	*into++ = (quint8)( (*outof >> 8) & 0xffL);
	*into =   (quint8) (*outof & 0xffL);
}

static void desfunc(register quint32 *block,register quint32 *keys)
{
	register quint32 fval, work, right, leftt;
	register int round;

	leftt = block[0]; right = block[1];
	work = ((leftt >> 4) ^ right) & 0x0f0f0f0fL;

	right ^= work;    leftt ^= (work << 4);
	work = ((leftt >> 16) ^ right) & 0x0000ffffL;

	right ^= work;    leftt ^= (work << 16);
	work = ((right >> 2) ^ leftt) & 0x33333333L;

	leftt ^= work;	right ^= (work << 2);
	work = ((right >> 8) ^ leftt) & 0x00ff00ffL;

	leftt ^= work;	right ^= (work << 8);
	right = ((right << 1) | ((right >> 31) & 1L)) & 0xffffffffL;
	work = (leftt ^ right) & 0xaaaaaaaaL;

	leftt ^= work;    right ^= work;
	leftt = ((leftt << 1) | ((leftt >> 31) & 1L)) & 0xffffffffL;

	for (round = 0; round < 8; round++) 
	{
		work = (right << 28) | (right >> 4);
		work ^= *keys++;
		fval  = SP7[work         & 0x3fL];
		fval |= SP5[(work >>  8) & 0x3fL];
		fval |= SP3[(work >> 16) & 0x3fL];
		fval |= SP1[(work >> 24) & 0x3fL];
		work = right ^ *keys++;
		fval |= SP8[work         & 0x3fL];
		fval |= SP6[(work >>  8) & 0x3fL];
		fval |= SP4[(work >> 16) & 0x3fL];
		fval |= SP2[(work >> 24) & 0x3fL];
		leftt ^= fval;
		work = (leftt << 28) | (leftt >> 4);
		work ^= *keys++;
		fval  = SP7[work         & 0x3fL];
		fval |= SP5[(work >>  8) & 0x3fL];
		fval |= SP3[(work >> 16) & 0x3fL];
		fval |= SP1[(work >> 24) & 0x3fL];
		work = leftt ^ *keys++;
		fval |= SP8[work         & 0x3fL];
		fval |= SP6[(work >>  8) & 0x3fL];
		fval |= SP4[(work >> 16) & 0x3fL];
		fval |= SP2[(work >> 24) & 0x3fL];
		right ^= fval;
	}

	right = (right << 31) | (right >> 1);
	work = (leftt ^ right) & 0xaaaaaaaaL;
	leftt ^= work;  right ^= work;
	leftt = (leftt << 31) |(leftt >> 1);
	work = ((leftt >> 8) ^ right) & 0x00ff00ffL;
	right ^= work;  leftt ^= (work << 8);
	work = ((leftt >> 2) ^ right) & 0x33333333L;
	right ^= work;  leftt ^= (work << 2);
	work = ((right >> 16) ^ leftt) & 0x0000ffffL;
	leftt ^= work;  right ^= (work << 16);
	work = ((right >> 4) ^ leftt) & 0x0f0f0f0fL;
	leftt ^= work;  right ^= (work << 4);
	*block++ = right;
	*block = leftt;
}


// -----------------------------------------------------------------------
// des_block
//      Encrpts/Decrypts(according to the key currently loaded int the
//      internal key register) one block of eight bytes at address 'in'
//      into the block at address 'out'. They can be the same.
//
//      "in"
//      "out"
// -----------------------------------------------------------------------
static void des_block(quint8 *in, quint8 *out)
{
	quint32 work[2];

	scrunch(in, work);
	desfunc(work, KnL);
	unscrun(work, out);
}

// -----------------------------------------------------------------------
// des
//      Encrpts/Decrypts(according to the key currently loaded int the
//      internal key register) SOME blocks of eight bytes at address 'in'
//      into the block at address 'out'. They can be the same.
//
//      "in"
//      "out"
//      "block"  Number of blocks.
// -----------------------------------------------------------------------
static int des(quint8* in, quint8* out, int blocks)
{
	for (int i = 0; i < blocks; i++,in+=8,out+=8)
		des_block(in,out);
	return 0;
};



int QySecDes::encrypt (const char key[8], char* data, const int size)
{
	int blocks = 0;

	if ((!data) || (size < 8))
		return -1;

	if ((size % 8) == 0)
		blocks = size / 8;
	else 
		return -1;

	deskey ((quint8 *)key, ENCRYPT);
	des((quint8 *)data, (quint8 *)data, blocks);
	return 0;
};

int QySecDes::decrypt (const char key[8], char* data, const int size)
{
	int blocks = 0;

	if ((!data) || (size < 8))
		return -1;

	if ((size % 8) == 0)
		blocks = size / 8;
	else 
		return -1;

	deskey((quint8 *)key, DECRYPT);
	des((quint8 *)data, (quint8 *)data, blocks);

	return 0;
};

const quint8 confuse32[32] = {
	41,  35, 190, 132, 225, 108, 214, 174,
	82, 144,  73, 241, 241, 187, 233, 235,
	179, 166, 219,  60, 135,  12,  62, 153,
	36,  94,  13,  28,   6, 183,  71, 222
};

int QySecDes::encrypt32to32(const char key[8], const char in[32], char out[32])
{
	quint8 * u_in =  (quint8 *) in;
	quint8 * u_out = (quint8 *) out;
	quint8 * u_key = (quint8 *) key;

	quint8 i = 0;
	for (i=0; i<32; ++i) {
		u_out[i] = u_in[i];
		u_out[i] ^= confuse32[i];
		u_out[i] ^= u_key[i%8];
	}

	encrypt(key, out, 32);

	return 0;
}

int QySecDes::decrypt32to32(const char key[8], const char in[32], char out[32])
{
	quint8 * u_in =  (quint8 *) in;
	quint8 * u_out = (quint8 *) out;
	quint8 * u_key = (quint8 *) key;

	quint8 i = 0;
	for (i=0; i<32; ++i) {
		u_out[i] = u_in[i];
	}

	decrypt(key, out, 32);

	for (i=0; i<32; ++i) {
		u_out[i] ^= u_key[i%8];
		u_out[i] ^= confuse32[i];
	}

	return 0;
}


unsigned char QySecDes::encrypt32to1(const char key[8], const char in[32])
{
	quint8 * u_in =  (quint8 *) in;
	quint8 u_out[32];
	quint8 * u_key = (quint8 *) key;

	quint8 i = 0;
	for (i=0; i<32; ++i) {
		u_out[i] = u_in[i];
		u_out[i] ^= confuse32[i];
		u_out[i] ^= u_key[i%8];
	}

	encrypt(key, (char *) u_out, 32);

	quint8 r = 0;
	for (i=0; i<32; ++i) {
		r ^= u_out[i];
	}

	return r;
}

END_NAMESPACE(qy)
