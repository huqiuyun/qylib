
#include "qystringutils.h"

namespace qy {

	bool memory_check(const void* memory, int c, size_t count)
	{
		const char* char_memory = static_cast<const char*>(memory);
		char char_c = static_cast<char>(c);
        for (size_t i=0; i<count; ++i) {
            if (char_memory[i] != char_c)
                return false;
		}
		return true;
	}

#ifdef H_OS_WIN
	int ascii__QY_NET_libcompare(const wchar_t* s1, const char* s2, size_t n,CharacterTransformation transformation) 
	{
			wchar_t c1, c2;
			while (true) {
				if (n-- == 0) return 0;
				c1 = transformation(*s1);
				// Double check that characters are not UTF-8
                assert(static_cast<unsigned char>(*s2) < 128);
				// Note: *s2 gets implicitly promoted to wchar_t
				c2 = transformation(*s2);
				if (c1 != c2) return (c1 < c2) ? -1 : 1;
				if (!c1) return 0;
				++s1;
				++s2;
			}
	}

	size_t asccpyn(wchar_t* buffer, size_t buflen,const char* source, size_t srclen) 
	{
			if (buflen <= 0)
				return 0;

            if (srclen == SIZE_UNKNOWN)
				srclen = strlenn(source, buflen - 1);
            else if (srclen >= buflen)
				srclen = buflen - 1;

			std::copy(source, source + srclen, buffer);
			buffer[srclen] = 0;
			return srclen;
	}

#endif  // H_OS_WIN

}  // namespace qy
