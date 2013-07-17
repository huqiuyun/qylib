#include "qyosstring.h"

#if !defined (__qyINLINE__)
#include "qyosstring.inl"
#endif /* __qyINLINE__ */

#include <stdlib.h>

DEFINE_NAMESPACE(qy)

const void*
qyos_memchr_emulation (const void *s, int c, size_t len)
{
	const unsigned char *t = (const unsigned char *) s;
	const unsigned char *e = (const unsigned char *) s + len;

	while (t < e)
		if (((int) *t) == c)
			return t;
		else
			++t;

	return 0;
}

char *
qyos_strdup_emulation (const char *s)
{
	char *t = (char *) ::malloc (qyos_strlen (s) + 1);
	if (t == 0)
		return 0;

	return qyos_strcpy (t, s);
}


wchar_t *
qyos_strdup_emulation (const wchar_t *s)
{
	wchar_t *buffer =
		(wchar_t *) ::malloc ((qyos_strlen (s) + 1)
		* sizeof (wchar_t));
	if (buffer == 0)
		return 0;

	return qyos_strcpy (buffer, s);
}

char *
qyos_strecpy (char *s, const char *t)
{
	register char *dscan = s;
	register const char *sscan = t;

	while ((*dscan++ = *sscan++) != '\0')
		continue;

	return dscan;
}

wchar_t *
qyos_strecpy (wchar_t *s, const wchar_t *t)
{
	register wchar_t *dscan = s;
	register const wchar_t *sscan = t;

	while ((*dscan++ = *sscan++) != L'\0')
		continue;

	return dscan;
}

const char *
qyos_strnchr (const char *s, int c, size_t len)
{
	for (size_t i = 0; i < len; ++i)
		if (s[i] == c)
			return s + i;

	return 0;
}

const char *
qyos_strnichr (const char *s, int c, size_t len)
{
	for (size_t i = 0; i < len; ++i)
		if (qyos_lower(s[i]) == qyos_lower(c))
			return s + i;
    
    return 0;
}

const wchar_t *
qyos_strnchr (const wchar_t *s, wchar_t c, size_t len)
{
	for (size_t i = 0; i < len; ++i)
		if (s[i] == c)
			return s + i;

	return 0;
}

const wchar_t *
qyos_strnichr (const wchar_t *s, int c, size_t len)
{
	for (size_t i = 0; i < len; ++i)
		if (qyos_lower(s[i]) == qyos_lower(c))
			return s + i;
    
    return 0;
}

const char *
qyos_strnstr (const char *s1, size_t len1,const char *s2, size_t len2)
{
	// Check if the substring is longer than the string being searched.
	if (len2 > len1)
		return 0;

	// Go upto <len>
	size_t const len = len1 - len2;

	for (size_t i = 0; i <= len; i++)
	{
		if (qyos_memcmp (s1 + i, s2, len2) == 0)
			// Found a match!  Return the index.
			return s1 + i;
	}

	return 0;
}

const char *
qyos_strnistr (const char *s1, size_t len1,const char *s2, size_t len2)
{
	// Check if the substring is longer than the string being searched.
	if (len2 > len1)
		return 0;
    
	// Go upto <len>
	const size_t len = len1 - len2;
    
	for (size_t i = 0; i <= len; i++)
	{
		if (qyos_strnicmp(s1 + i, s2, len2) == 0)
			// Found a match!  Return the index.
			return s1 + i;
	}
	return 0;
}

const wchar_t *
qyos_strnstr (const wchar_t *s1, size_t len1, const wchar_t *s2, size_t len2)
{
	// Check if the substring is longer than the string being searched.
	if (len2 > len1)
		return 0;

	// Go upto <len>
	const size_t len = len1 - len2;

	for (size_t i = 0; i <= len; i++)
	{
		if (qyos_memcmp (s1 + i, s2, len2 * sizeof (wchar_t)) == 0)
			// Found a match!  Return the index.
			return s1 + i;
	}

	return 0;
}

const wchar_t *
qyos_strnistr (const wchar_t *s1, size_t len1,const wchar_t *s2, size_t len2)
{
	// Check if the substring is longer than the string being searched.
	if (len2 > len1)
		return 0;
    
	// Go upto <len>
	const size_t len = len1 - len2;
    
	for (size_t i = 0; i <= len; i++)
	{
		if (qyos_strnicmp(s1 + i, s2, len2) == 0)
			// Found a match!  Return the index.
			return s1 + i;
	}
	return 0;
}

void *
qyos_fast_memcpy (void *t, const void *s, size_t len)
{
	unsigned char* to = static_cast<unsigned char*> (t) ;
	const unsigned char* from = static_cast<const unsigned char*> (s) ;
	// Unroll the loop...
	switch (len)
	{
	case 16: to[15] = from[15];
	case 15: to[14] = from[14];
	case 14: to[13] = from[13];
	case 13: to[12] = from[12];
	case 12: to[11] = from[11];
	case 11: to[10] = from[10];
	case 10: to[9] = from[9];
	case  9: to[8] = from[8];
	case  8: to[7] = from[7];
	case  7: to[6] = from[6];
	case  6: to[5] = from[5];
	case  5: to[4] = from[4];
	case  4: to[3] = from[3];
	case  3: to[2] = from[2];
	case  2: to[1] = from[1];
	case  1: to[0] = from[0];
	case  0: return t;
	default: return ::memcpy (t, s, len);
	}
}


char *
qyos_strrchr_emulation (char *s, int c)
{
	char *p = s + qyos_strlen (s);

	while (*p != c)
		if (p == s)
			return 0;
		else
			--p;

	return p;
}

const char *
qyos_strrchr_emulation (const char *s, int c)
{
	const char *p = s + qyos_strlen (s);

	while (*p != c)
		if (p == s)
			return 0;
		else
			--p;

	return p;
}

char *
qyos_strsncpy (char *dst, const char *src, size_t maxlen)
{
	register char *rdst = dst;
	register const char *rsrc = src;
	register size_t rmaxlen = maxlen;

	if (rmaxlen > 0)
	{
		if (rdst!=rsrc)
		{
			*rdst = '\0';
			if (rsrc != 0)
				strncat (rdst, rsrc, --rmaxlen);
		}
		else
		{
			rdst += (rmaxlen - 1);
			*rdst = '\0';
		}
	}
	return dst;
}

wchar_t *
qyos_strsncpy (wchar_t *dst, const wchar_t *src, size_t maxlen)
{
	register wchar_t *rdst = dst;
	register const wchar_t *rsrc = src;
	register size_t rmaxlen = maxlen;

	if (rmaxlen > 0)
	{
		if (rdst!=rsrc)
		{
			*rdst = L'\0';
			if (rsrc != 0)
				wcsncat (rdst, rsrc, --rmaxlen);
		}
		else
		{
			rdst += (rmaxlen - 1);
			*rdst = L'\0';
		}
	}
	return dst;
}

char *
qyos_strtok_r_emulation (char *s, const char *tokens, char **lasts)
{
	if (s == 0)
		s = *lasts;
	else
		*lasts = s;
	if (*s == 0)                  // We have reached the end
		return 0;
	size_t l_org = qyos_strlen (s);
	s = ::strtok (s, tokens);
	if (s == 0)
		return 0;
	const size_t l_sub = qyos_strlen (s);
	if (s + l_sub < *lasts + l_org)
		*lasts = s + l_sub + 1;
	else
		*lasts = s + l_sub;
	return s ;
}


wchar_t*
qyos_strtok_r_emulation (wchar_t *s,const wchar_t *tokens,wchar_t **lasts)
{
	wchar_t* sbegin = s ? s : *lasts;
	sbegin += qyos_strspn(sbegin, tokens);
	if (*sbegin == 0)
	{
		static wchar_t empty[1] = { 0 };
		*lasts = empty;
		return 0;
	}
	wchar_t*send = sbegin + qyos_strcspn(sbegin, tokens);
	if (*send != 0)
		*send++ = 0;
	*lasts = send;
	return sbegin;
}

int
qyos_strnicmp(const char *s, const char *t, size_t len)
{
    for (size_t i = 0; i < len ; ++i) {
        ushort ut = qyos_lower(t[i]);
        ushort us = qyos_lower(s[i]);
        if (ut>us) return 1;
        if (ut<us) return -1;
    }
    return 0;
}

int
qyos_strnicmp(const wchar_t *s, const wchar_t *t, size_t len)
{
    for (size_t i = 0; i < len ; ++i) {
        ushort ut = qyos_lower(t[i]);
        ushort us = qyos_lower(s[i]);
        if (ut>us) return 1;
        if (ut<us) return -1;
    }
    return 0;
}

END_NAMESPACE(qy)
