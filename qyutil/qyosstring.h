#ifndef __qyOS_STRING_H__
#define __qyOS_STRING_H__

#include "qyutil/qyconfig.h"
#include <string.h>
#ifndef WIN32
#include <wchar.h>
#endif //!WIN32
#pragma warning(disable:4996)

DEFINE_NAMESPACE(qy)

/** @name Functions from <cstring>
 *
 *  Included are the functions defined in <cstring> and their <cwchar>
 *  equivalents.
 *
 *  @todo To be complete, we should add strcoll, and strxfrm.
 */

/// Finds characters in a buffer (const void version).
qyINLINE
const void *qyos_memchr (const void *s, int c, size_t len);

/// Finds characters in a buffer (void version).
qyINLINE
void *qyos_memchr (void *s, int c, size_t len);

/// Emulated memchr - Finds a character in a buffer.
extern QYUTIL_API
const void *qyos_memchr_emulation (const void *s, int c, size_t len);

/// Compares two buffers.
qyINLINE
int qyos_memcmp (const void *t, const void *s, size_t len);

/// Copies one buffer to another.
qyINLINE
void *qyos_memcpy (void *t, const void *s, size_t len);

/*
 * Version of memcpy where the copy loop is unrolled.
 * On certain platforms this results in better performance.
 * This is determined and set via autoconf.
 */
extern QYUTIL_API
void *qyos_fast_memcpy (void *t, const void *s, size_t len);

/// Moves one buffer to another.
qyINLINE
void *qyos_memmove (void *t, const void *s, size_t len);

/// Fills a buffer with a character value.
qyINLINE
void *qyos_memset (void *s, int c, size_t len);

///
qyINLINE
int   qyos_atoi(const char* s);

qyINLINE
int   qyos_atoi(const wchar_t*s);

qyINLINE
int64 qyos_atoi64(const char* s);

qyINLINE
int64 qyos_atoi64(const wchar_t*s);

///
qyINLINE
double qyos_atof(const char* s);

qyINLINE
double qyos_atof(const wchar_t*s);

qyINLINE
longlong  qyos_strtoll(const char* s, char** , int base);

qyINLINE
ulonglong  qyos_strtoull(const char* s, char** , int base);

qyINLINE
longlong  qyos_strtoll(const wchar_t* s,wchar_t** ,int base);

qyINLINE
ulonglong  qyos_strtoull(const wchar_t* s,wchar_t** ,int base);

/// Appends a string to another string (char version).
qyINLINE
char *qyos_strcat (char *s, const char *t);


/// Appends a string to another string (wchar_t version).
qyINLINE
wchar_t *qyos_strcat (wchar_t *s, const wchar_t *t);

/// Finds the first occurance of a character in a string (const char
/// version).
qyINLINE
const char *qyos_strchr (const char *s, int c);


/// Finds the first occurance of a character in a string (const wchar_t
/// version).
qyINLINE
const wchar_t *qyos_strchr (const wchar_t *s, wchar_t c);

/// Finds the first occurance of a character in a string (char version).
qyINLINE
char *qyos_strchr (char *s, int c);


/// Finds the first occurance of a character in a string (wchar_t version).
qyINLINE
wchar_t *qyos_strchr (wchar_t *s, wchar_t c);

/// Compares two strings (char version).
qyINLINE
int qyos_strcmp (const char *s, const char *t);

/// Compares two strings (wchar_t version).
qyINLINE
int qyos_strcmp (const wchar_t *s, const wchar_t *t);

qyINLINE
void qyos_strset (char *s, const char t, size_t length);

qyINLINE
void qyos_strset (wchar_t *s, const wchar_t t,size_t length);

/// Copies a string (char version).
qyINLINE
char *qyos_strcpy (char *s, const char *t);

/// Copies a string (wchar_t version).
qyINLINE
wchar_t *qyos_strcpy (wchar_t *s, const wchar_t *t);

/// Searches for the first substring without any of the specified
/// characters and returns the size of the substring (char version).
qyINLINE
size_t qyos_strcspn (const char *s, const char *reject);

/// Searches for the first substring without any of the specified
/// characters and returns the size of the substring (wchar_t version).
qyINLINE
size_t qyos_strcspn (const wchar_t *s, const wchar_t *reject);

/// Returns a malloced duplicated string (char version).
qyINLINE
char *qyos_strdup (const char *s);

extern QYUTIL_API
char *qyos_strdup_emulation (const char *s);

/// Returns a malloced duplicated string (wchar_t version).
qyINLINE
wchar_t *qyos_strdup (const wchar_t *s);

extern QYUTIL_API
wchar_t *qyos_strdup_emulation (const wchar_t *s);


/// Copies a string, but returns a pointer to the end of the
/// copied region (char version).
extern QYUTIL_API
char *qyos_strecpy (char *des, const char *src);


/// Copies a string, but returns a pointer to the end of the
/// copied region (wchar_t version).
extern QYUTIL_API
wchar_t *qyos_strecpy (wchar_t *s, const wchar_t *t);

/// Finds the length of a string (char version).
qyINLINE
size_t qyos_strlen (const char *s);

/// Finds the length of a string (wchar_t version).
qyINLINE
size_t qyos_strlen (const wchar_t *s);

/// Appends part of a string to another string (char version).
qyINLINE
char *qyos_strncat (char *s, const char *t, size_t len);

/// Appends part of a string to another string (wchar_t version).
qyINLINE
wchar_t *qyos_strncat (wchar_t *s, const wchar_t *t, size_t len);

/// Finds the first occurance of a character in an array (const char
/// version).
extern QYUTIL_API
const char *qyos_strnchr (const char *s, int c, size_t len);

extern QYUTIL_API
const char *qyos_strnichr (const char *s, int c, size_t len);

/// Finds the first occurance of a character in an array (const wchar_t
/// version).
extern QYUTIL_API
const wchar_t *qyos_strnchr (const wchar_t *s, wchar_t c, size_t len);

extern QYUTIL_API
const wchar_t *qyos_strnichr (const wchar_t *s, int c, size_t len);

/// Finds the first occurance of a character in an array (char version).
qyINLINE
char *qyos_strnchr (char *s, int c, size_t len);

qyINLINE
char *qyos_strnichr (char *s, int c, size_t len);

/// Finds the first occurance of a character in an array (wchar_t version).
qyINLINE
wchar_t *qyos_strnchr (wchar_t *s, wchar_t c, size_t len);

qyINLINE
wchar_t *qyos_strnichr (wchar_t *s, wchar_t c, size_t len);

/// Compares two arrays (char version).
qyINLINE
int qyos_strncmp (const char *s, const char *t, size_t len);

/// Compares two arrays (wchar_t version).
qyINLINE
int qyos_strncmp (const wchar_t *s, const wchar_t *t, size_t len);

// Not case sensitive
extern QYUTIL_API
int qyos_strnicmp(const char *s, const char *t, size_t len);

extern QYUTIL_API
int qyos_strnicmp(const wchar_t *s, const wchar_t *t, size_t len);

/// Copies an array (char version)
qyINLINE
char *qyos_strncpy (char *s, const char *t, size_t len);

/// Copies an array (wchar_t version)
qyINLINE
wchar_t *qyos_strncpy (wchar_t *s, const wchar_t *t, size_t len);

/// Finds the length of a limited-length string (char version).
/**
 * @param s       The character string to find the length of.
 * @param maxlen  The maximum number of characters that will be
 *                scanned for the terminating nul character.
 *
 * @return The length of @arg s, if the terminating nul character
 *         is located, else @arg maxlen.
 */
qyINLINE
size_t qyos_strnlen (const char *s, size_t maxlen);

/// Finds the length of a limited-length string (wchar_t version).
/**
 * @param s       The character string to find the length of.
 * @param maxlen  The maximum number of characters that will be
 *                scanned for the terminating nul character.
 *
 * @return The length of @arg s, if the terminating nul character
 *         is located, else @arg maxlen.
 */
qyINLINE
size_t qyos_strnlen (const wchar_t *s, size_t maxlen);

/// Finds the first occurance of a substring in an array (const char
/// version).
extern QYUTIL_API
const char *qyos_strnstr (const char *s, size_t slen,const char *t, size_t tlen);

extern QYUTIL_API
const char *qyos_strnistr (const char *s, size_t slen,const char *t, size_t tlen);

/// Finds the first occurance of a substring in an array (const wchar_t
/// version).
extern QYUTIL_API
const wchar_t *qyos_strnstr (const wchar_t *s,size_t slen,const wchar_t *t,size_t tlen);

extern QYUTIL_API
const wchar_t *qyos_strnistr (const wchar_t *s, size_t slen,const wchar_t *t, size_t tlen);

/// Finds the first occurance of a substring in an array (char version).
qyINLINE
char *qyos_strnstr (char *s, size_t slen,const char *t, size_t tlen);

qyINLINE
char *qyos_strnistr (char *s, size_t slen,const char *t, size_t tlen);

/// Finds the first occurance of a substring in an array (wchar_t version).
qyINLINE
wchar_t *qyos_strnstr (wchar_t *s, size_t slen,const wchar_t *t, size_t tlen);

qyINLINE
wchar_t *qyos_strnistr (wchar_t *s, size_t slen,const wchar_t *t, size_t tlen);

/// Searches for characters in a string (const char version).
qyINLINE
const char *qyos_strpbrk (const char *s1, const char *s2);

/// Searches for characters in a string (const wchar_t version).
qyINLINE
const wchar_t *qyos_strpbrk (const wchar_t *s1, const wchar_t *s2);

/// Searches for characters in a string (char version).
qyINLINE
char *qyos_strpbrk (char *s1, const char *s2);

/// Searches for characters in a string (wchar_t version).
qyINLINE
wchar_t *qyos_strpbrk (wchar_t *s1, const wchar_t *s2);

/// Finds the last occurance of a character in a string (const char
/// version).
qyINLINE
const char *qyos_strrchr (const char *s, int c);

/// Finds the last occurance of a character in a string (const wchar_t
/// version).
qyINLINE
const wchar_t *qyos_strrchr (const wchar_t *s, wchar_t c);

/// Finds the last occurance of a character in a string (char version).
qyINLINE
char *qyos_strrchr (char *s, int c);

/// Finds the last occurance of a character in a string (wchar_t version).
qyINLINE
wchar_t *qyos_strrchr (wchar_t *s, wchar_t c);

/// Emulated strrchr (char version) - Finds the last occurance of a
/// character in a string.
extern QYUTIL_API
char *qyos_strrchr_emulation (char *s, int c);

/// Emulated strrchr (const char version) - Finds the last occurance of a
/// character in a string.
extern QYUTIL_API
const char *qyos_strrchr_emulation (const char *s, int c);

/// This is a "safe" c string copy function (char version).
/**
 * Unlike strncpy() this function will always add a terminating '\0'
 * char if maxlen > 0.  So the user doesn't has to provide an extra
 * '\0' if the user wants a '\0' terminated dst.  The function
 * doesn't check for a 0 @a dst, because this will give problems
 * anyway.  When @a src is 0 an empty string is made.  We do not
 * "touch" * @a dst if maxlen is 0.  Returns @a dst.  Care should be
 * taken when replacing strncpy() calls, because in some cases a
 * strncpy() user is using the "not '\0' terminating" feature from
 * strncpy().  This happens most when the call to strncpy() was
 * optimized by using a maxlen which is 1 smaller than the size
 * because there's always written a '\0' inside this last position.
 * Very seldom it's possible that the '\0' padding feature from
 * strncpy() is needed.
 */
extern QYUTIL_API
char *qyos_strsncpy (char *dst,const char *src,size_t maxlen);

/// This is a "safe" c string copy function (wchar_t version).
/**
 * Unlike strncpy() this function will always add a terminating '\0'
 * char if maxlen > 0.  So the user doesn't has to provide an extra
 * '\0' if the user wants a '\0' terminated dst.  The function
 * doesn't check for a 0 @a dst, because this will give problems
 * anyway.  When @a src is 0 an empty string is made.  We do not
 * "touch" * @a dst if maxlen is 0.  Returns @a dst.  Care should be
 * taken when replacing strncpy() calls, because in some cases a
 * strncpy() user is using the "not '\0' terminating" feature from
 * strncpy().  This happens most when the call to strncpy() was
 * optimized by using a maxlen which is 1 smaller than the size
 * because there's always written a '\0' inside this last position.
 * Very seldom it's possible that the '\0' padding feature from
 * strncpy() is needed.
 */
extern QYUTIL_API
wchar_t *qyos_strsncpy (wchar_t *dst,const wchar_t *src,size_t maxlen);

/// Searches for the first substring containing only the specified
/// characters and returns the size of the substring (char version).
qyINLINE
size_t qyos_strspn (const char *s1, const char *s2);


/// Searches for the first substring containing only the specified
/// characters and returns the size of the substring (wchar_t version).
qyINLINE
size_t qyos_strspn (const wchar_t *s1, const wchar_t *s2);

/// Finds the first occurance of a substring in a string (const char
/// version).
qyINLINE
const char *qyos_strstr (const char *s, const char *t);

/// Finds the first occurance of a substring in a string (const wchar_t
/// version).
qyINLINE
const wchar_t *qyos_strstr (const wchar_t *s, const wchar_t *t);

/// Finds the first occurance of a substring in a string (char version).
qyINLINE
char *qyos_strstr (char *s, const char *t);

/// Finds the first occurance of a substring in a string (wchar_t version).
qyINLINE
wchar_t *qyos_strstr (wchar_t *s, const wchar_t *t);

/// Finds the next token in a string (char version).
qyINLINE
char *qyos_strtok (char *s, const char *tokens);

/// Finds the next token in a string (wchar_t version).
qyINLINE
wchar_t *qyos_strtok (wchar_t *s, const wchar_t *tokens);

//@}

/// Finds the next token in a string (safe char version).
qyINLINE
char *qyos_strtok_r (char *s, const char *tokens, char **lasts);

/// Finds the next token in a string (wchar_t version).
qyINLINE
wchar_t *qyos_strtok_r (wchar_t *s, const wchar_t *tokens, wchar_t **lasts);

/// Emulated strtok_r.
extern QYUTIL_API
char *qyos_strtok_r_emulation (char *s, const char *tokens, char **lasts);

/// Emulated strtok_r (wchar_t version).
extern QYUTIL_API
wchar_t *qyos_strtok_r_emulation (wchar_t *s, const wchar_t *tokens, wchar_t **lasts);


//! Returns a character converted to lower case
qyINLINE
unsigned short qyos_lower(unsigned short x)
{
    // ansi
    return x >= 'A' && x <= 'Z' ? x + 0x20 : x;
}

//! Returns a character converted to upper case
qyINLINE
unsigned short qyos_upper(unsigned short x)
{
    // ansi
    return x >= 'a' && x <= 'z' ? x +('A' - 'a') : x;
}

qyINLINE int qyos_vsnprintf(char * out, size_t, const char * in, va_list);

qyINLINE int qyos_vsnprintf(wchar_t * out, size_t, const wchar_t * in, va_list);

/**
 * finds first occurrence of character in string
 @param c: Character to search for.
 @return Position where the character has been found,
 or -1 if not found. */
template<typename T>
size_t qyos_findFirst(const T* str , size_t len , T c , size_t start = 0)
{
    if (start >= len)
        return -1;
    for(size_t i=start; i<len; ++i)
        if (str[i] == c)
            return i;
    
    return -1;
}

/**
 * finds last occurrence of character in string
 @param c: Character to search for.
 @param start: start to search reverse(default = -1, on end)
 @return Position where the character has been found,
 or -1 if not found. */
template<typename T>
size_t qyos_findLast(const T* str , size_t len , T c , size_t start = 0)
{
    if (start >= len)
        return (int)len - 1;
    
    for(size_t i=start; i!= -1; --i)
        if (str[i] == c)
            return (int)i;
    
    return -1;
}

/**
 * finds first occurrence of a character of a list in string
 @param c: List of characters to find. For example if the method
 should find the first occurrence of 'a' or 'b', this parameter should be "ab".
 @param count: Amount of characters in the list. Usually,
 this should be strlen(c)
 @return Position where one of the characters has been found,
 or -1 if not found. */
template<typename T>
size_t qyos_findFirstChar(const T* str , size_t len , const T* const c, size_t count)
{
    if (!c)
        return -1;
    
    for (size_t i=0; i<len; ++i)
        for (size_t j=0; j<count; ++j)
            if (str[i] == c[j])
                return i;
    return -1;
}

//! Finds first position of a character not in a given list.
/** \param c: List of characters not to find. For example if the method
 should find the first occurrence of a character not 'a' or 'b', this parameter should be "ab".
 \param count: Amount of characters in the list. Usually,
 this should be strlen(c)
 \return Position where the character has been found,
 or -1 if not found. */
template <typename T>
size_t qyos_findFirstCharNotInList(const T* str , size_t len , const T* const c, size_t count)
{
    for(size_t i=0; i<len-1; ++i)
    {
        size_t j;
        for (j=0; j<count; ++j)
            if (str[i] == c[j])
                break;
        
        if (j==count)
            return i;
    }
    
    return -1;
}

//! Finds last position of a character not in a given list.
/** \param c: List of characters not to find. For example if the method
 should find the first occurrence of a character not 'a' or 'b', this parameter should be "ab".
 \param count: Amount of characters in the list. Usually,
 this should be strlen(c)
 \return Position where the character has been found,
 or -1 if not found. */
template <typename T>
int qyos_findLastCharNotInList(const T* str , size_t len , const T* const c, size_t count)
{
    for(int i= (int)len-2; i >= 0; --i)
    {
        size_t j;
        for (j=0; j<count; ++j)
            if (str[i] == c[j])
                break;
        
        if (j==count)
            return i;
    }
    return -1;
}


END_NAMESPACE(qy)

# if defined (__qyINLINE__)
#include "qyosstring.inl"
# endif /* __qyINLINE__ */

#endif /* __qyOS_STRING_H__ */
