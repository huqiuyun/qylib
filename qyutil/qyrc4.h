#pragma  once

#include "qyutil/qyconfig.h"

BEGIN_EXTERN_C
/*
 * Encrypt some data using the supplied key..
 * The input and output buffers may be the same buffer.
 * Since RC4 is a stream cypher, this function is used
 * for both encryption and decryption.
 */
void QYUTIL_API qyrc4_crypt(const char *inbuf, char *outbuf, size_t buflen, const char *key, size_t keylen);

END_EXTERN_C
