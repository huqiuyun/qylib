#ifndef __SEC_DES_H__
#define __SEC_DES_H__

#include "qyutil/qyconfig.h"

DEFINE_NAMESPACE(qy)

/********************************************************************
 * Encrypt/decrypt the data in "data", according to the "key". 		*
 * Caller is responsible for confirming the buffer size of "data" 	*
 * points to is 8*"blocks" bytes. The data encrypted/decrypted is 	*
 * stored in data. The return code is 0:success, other:failed. 		*
 ********************************************************************/

class QYUTIL_API QySecDes
{
public:
	int encrypt(const char key[8], char* data, const int size);
	int decrypt(const char key[8], char* data, const int size);

private:
	int encrypt32to32(const char key[8], const char in[32], char out[32]);
	int decrypt32to32(const char key[8], const char in[32], char out[32]);
	unsigned char encrypt32to1(const char key[8], const char in[32]);
};

END_NAMESPACE(qy)

#endif /* #ifndef __SEC_DES_H__ */