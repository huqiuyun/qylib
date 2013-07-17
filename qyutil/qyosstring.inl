
DEFINE_NAMESPACE(qy)

qyINLINE const void *
qyos_memchr (const void *s, int c, size_t len)
{
return ::memchr (s, c, len);
}

qyINLINE void *
qyos_memchr (void *s, int c, size_t len)
{
return const_cast<void *> (qyos_memchr (static_cast<const void *> (s),c,len));
}

qyINLINE int
qyos_memcmp (const void *t, const void *s, size_t len)
{
return ::memcmp (t, s, len);
}

qyINLINE void *
qyos_memcpy (void *t, const void *s, size_t len)
{
return ::memcpy (t, s, len);
}

qyINLINE void *
qyos_memmove (void *t, const void *s, size_t len)
{
return ::memmove (t, s, len);
}

qyINLINE void *
qyos_memset (void *s, int c, size_t len)
{
return ::memset (s, c, len);
}

qyINLINE int
qyos_atoi(const char* s)
{
return ::atoi(s);
}

qyINLINE int
qyos_atoi(const wchar_t*s)
{
return (int)::wcstol(s,NULL,10);
}

///
qyINLINE
double qyos_atof(const char* s)
{
return ::strtod(s,NULL);
}

qyINLINE
double qyos_atof(const wchar_t*s)
{
return ::wcstod(s,NULL);
}

qyINLINE
int64 qyos_atoi64(const char* s)
{
return 0;
}

qyINLINE
int64 qyos_atoi64(const wchar_t* s)
{
return 0;
}

qyINLINE
longlong  qyos_strtoll(const char* s, char** end, int base)
{
return ::strtoll(s,end,base);
}

qyINLINE
ulonglong  qyos_strtoull(const char* s, char** end, int base)
{
return ::strtoull(s,end,base);
}

qyINLINE
longlong  qyos_strtoll(const wchar_t* s,wchar_t** end,int base)
{
return ::wcstoll(s,end,base);
}

qyINLINE
ulonglong  qyos_strtoull(const wchar_t* s,wchar_t** end,int base)
{
return ::wcstoull(s,end,base);
}

qyINLINE char *
qyos_strcat (char *s, const char *t)
{
return ::strcat(s, t);
}

qyINLINE wchar_t *
qyos_strcat (wchar_t *s, const wchar_t *t)
{
return ::wcscat(s, t);
}

qyINLINE const char *
qyos_strchr (const char *s, int c)
{
return const_cast <const char *> (::strchr (s, c));
}


qyINLINE const wchar_t *
qyos_strchr (const wchar_t *s, wchar_t c)
{
return wcschr(s,c);
}


qyINLINE char *
qyos_strchr (char *s, int c)
{
return ::strchr(s, c);
}

qyINLINE wchar_t *
qyos_strchr (wchar_t *s, wchar_t c)
{
return const_cast<wchar_t *> (qyos_strchr (const_cast<const wchar_t *> (s),c));
}

qyINLINE int
qyos_strcmp (const char *s, const char *t)
{
return ::strcmp(s, t);
}

qyINLINE int
qyos_strcmp (const wchar_t *s, const wchar_t *t)
{
return wcscmp(s,t);
}

qyINLINE void
qyos_strset (char *s, const char t, size_t length)
{
::memset(s, t , length);
}

qyINLINE void
qyos_strset (wchar_t *s, const wchar_t t,size_t length)
{
size_t i = 0;
while( i < length)
{
*s = t;
s++;
i++;
}
return;
}

qyINLINE char *
qyos_strcpy (char *s, const char *t)
{
return ::strcpy(s, t);
}

qyINLINE wchar_t *
qyos_strcpy (wchar_t *s, const wchar_t *t)
{
return wcscpy(s, t);
}

qyINLINE size_t
qyos_strcspn (const char *s, const char *reject)
{
return ::strcspn(s, reject);
}

qyINLINE size_t
qyos_strcspn (const wchar_t *s, const wchar_t *reject)
{
return wcscspn(s, reject);
}

qyINLINE char *
qyos_strdup (const char *s)
{
return ::strdup (s);
}

qyINLINE wchar_t *
qyos_strdup (const wchar_t *s)
{
return wcsdup(s);

}

qyINLINE size_t
qyos_strlen (const char *s)
{
return ::strlen(s);
}

qyINLINE size_t
qyos_strlen (const wchar_t *s)
{
return wcslen(s);
}

qyINLINE char *
qyos_strncat (char *s, const char *t, size_t len)
{
return ::strncat(s, t, len);

}

qyINLINE wchar_t *
qyos_strncat (wchar_t *s, const wchar_t *t, size_t len)
{
return wcsncat(s, t, len);
}

qyINLINE char *
qyos_strnchr(char *s, int c, size_t len)
{
return const_cast<char *> (qyos_strnchr (static_cast<const char *> (s),c,len));
}

qyINLINE char *
qyos_strnichr(char *s, int c, size_t len)
{
return const_cast<char *> (qyos_strnichr (static_cast<const char *> (s),c,len));
}

qyINLINE wchar_t *
qyos_strnchr(wchar_t *s, wchar_t c, size_t len)
{
return
const_cast<wchar_t *> (qyos_strnchr (const_cast<const wchar_t *> (s),c,len));
}

qyINLINE wchar_t *
qyos_strnichr(wchar_t *s, wchar_t c, size_t len)
{
return
const_cast<wchar_t *> (qyos_strnichr (const_cast<const wchar_t *> (s),c,len));
}

qyINLINE int
qyos_strncmp (const char *s, const char *t, size_t len)
{
return ::strncmp(s, t, len);
}

qyINLINE int
qyos_strncmp (const wchar_t *s, const wchar_t *t, size_t len)
{
return wcsncmp(s, t, len);
}

qyINLINE char *
qyos_strncpy (char *s, const char *t, size_t len)
{
return ::strncpy(s, t, len);
}

qyINLINE wchar_t *
qyos_strncpy (wchar_t *s, const wchar_t *t, size_t len)
{
return wcsncpy(s, t, len);
}

qyINLINE size_t
qyos_strnlen (const char *s, size_t maxlen)
{
return ::strnlen(s, maxlen);
}

qyINLINE size_t
qyos_strnlen (const wchar_t *s, size_t maxlen)
{
return wcsnlen(s, maxlen);
}

qyINLINE char *
qyos_strnstr (char *s, size_t slen,const char *t, size_t tlen)
{
return
const_cast <char *> (qyos_strnstr (const_cast <const char *> (s), slen, t, tlen));
}

qyINLINE char *
qyos_strnistr (char *s, size_t slen,const char *t, size_t tlen)
{
return
const_cast <char *> (qyos_strnistr (const_cast <const char *> (s), slen, t, tlen));
}

qyINLINE wchar_t *
qyos_strnstr (wchar_t *s, size_t slen,const wchar_t *t, size_t tlen)
{
return
const_cast<wchar_t *> (qyos_strnstr (static_cast<const wchar_t *> (s), slen, t, tlen));
}

qyINLINE wchar_t *
qyos_strnistr (wchar_t *s, size_t slen,const wchar_t *t, size_t tlen)
{
return
const_cast<wchar_t *> (qyos_strnistr (static_cast<const wchar_t *> (s), slen, t, tlen));
}

qyINLINE const char *
qyos_strpbrk (const char *s1, const char *s2)
{
return const_cast <const char *> (::strpbrk (s1, s2));
}

qyINLINE const wchar_t *
qyos_strpbrk (const wchar_t *s, const wchar_t *t)
{
return wcspbrk(s, t);
}

qyINLINE char *
qyos_strpbrk (char *s1, const char *s2)
{
return ::strpbrk(s1, s2);
}

qyINLINE wchar_t *
qyos_strpbrk (wchar_t *s, const wchar_t *t)
{
return const_cast<wchar_t *> (qyos_strpbrk (const_cast<const wchar_t *> (s), t));
}

qyINLINE const char *
qyos_strrchr (const char *s, int c)
{
return (const char *) ::strrchr (s, c);
}

qyINLINE const wchar_t *
qyos_strrchr (const wchar_t *s, wchar_t c)
{
return wcsrchr(s, c);
}

qyINLINE char *
qyos_strrchr (char *s, int c)
{
return ::strrchr (s, c);
}

qyINLINE wchar_t *
qyos_strrchr (wchar_t *s, wchar_t c)
{
return const_cast<wchar_t *> (qyos_strrchr (const_cast<const wchar_t *> (s), c));
}

qyINLINE size_t
qyos_strspn (const char *s, const char *t)
{
return ::strspn (s, t);
}

qyINLINE size_t
qyos_strspn (const wchar_t *s, const wchar_t *t)
{
return wcsspn(s, t);

}

qyINLINE const char *
qyos_strstr (const char *s, const char *t)
{
return (const char *) ::strstr (s, t);
}

qyINLINE const wchar_t *
qyos_strstr (const wchar_t *s, const wchar_t *t)
{
return wcsstr(s,t);
}

qyINLINE char *
qyos_strstr (char *s, const char *t)
{
return ::strstr (s, t);
}

qyINLINE wchar_t *
qyos_strstr (wchar_t *s, const wchar_t *t)
{
return wcsstr(s, t);

}

qyINLINE char *
qyos_strtok (char *s, const char *tokens)
{
return ::strtok (s, tokens);
}

qyINLINE wchar_t *
qyos_strtok (wchar_t *s, const wchar_t *tokens)
{
return ::wcstok(s, tokens,NULL);
}

qyINLINE char *
qyos_strtok_r (char *s, const char *tokens, char **lasts)
{
return ::strtok_r(s, tokens, lasts);
}

qyINLINE wchar_t*
qyos_strtok_r (wchar_t *s, const wchar_t *tokens, wchar_t **lasts)
{
return wcstok(s, tokens, lasts);
}

qyINLINE int
qyos_vsnprintf(char * out, size_t l, const char * in, va_list v)
{
return ::vsnprintf(out, l,in,v);
}

qyINLINE int
qyos_vsnprintf(wchar_t * out, size_t l, const wchar_t * in, va_list v)
{
return vswprintf(out,l,in,v);
}

END_NAMESPACE(qy)

