#ifndef QY_STRINGENCODE_H__
#define QY_STRINGENCODE_H__

#include <string>
#include <sstream>

namespace qy
{
	//////////////////////////////////////////////////////////////////////
	// String Encoding Utilities
	//////////////////////////////////////////////////////////////////////

	// Convert an unsigned value from 0 to 15 to the hex character equivalent...
	char hex_encode(unsigned char val);
	// ...and vice-versa.
	unsigned char hex_decode(char ch);

	// Convert an unsigned value to it's utf8 representation.  Returns the length
	// of the encoded string, or 0 if the encoding is longer than buflen - 1.
	size_t utf8_encode(char* buffer, size_t buflen, unsigned long value);
	// Decode the utf8 encoded value pointed to by source.  Returns the number of
	// bytes used by the encoding, or 0 if the encoding is invalid.
	size_t utf8_decode(const char* source, size_t srclen, unsigned long* value);

	// Escaping prefixes illegal characters with the escape character.  Compact, but
	// illegal characters still appear in the string.
	size_t escape(char * buffer, size_t buflen,
		const char * source, size_t srclen,
		const char * illegal, char escape);
	// Note: in-place unescaping (buffer == source) is allowed.
	size_t unescape(char * buffer, size_t buflen,
		const char * source, size_t srclen,
		char escape); 

	// Encoding replaces illegal characters with the escape character and 2 hex
	// chars, so it's a little less compact than escape, but completely removes
	// illegal characters.  note that hex digits should not be used as illegal
	// characters.
	size_t encode(char * buffer, size_t buflen,
		const char * source, size_t srclen,
		const char * illegal, char escape);
	// Note: in-place decoding (buffer == source) is allowed.
	size_t decode(char * buffer, size_t buflen,
		const char * source, size_t srclen,
		char escape);

	// Returns a list of characters that may be unsafe for use in the name of a
	// file, suitable for passing to the 'illegal' member of escape or encode.
	const char* unsafe_filename_characters();

	// url_encode is an encode operation with a predefined set of illegal characters
	// and escape character (for use in URLs, obviously).
	size_t url_encode(char * buffer, size_t buflen,
		const char * source, size_t srclen);
	// Note: in-place decoding (buffer == source) is allowed.
	size_t url_decode(char * buffer, size_t buflen,
		const char * source, size_t srclen);

	// html_encode prevents data embedded in html from containing markup.
	size_t html_encode(char * buffer, size_t buflen,
		const char * source, size_t srclen);
	// Note: in-place decoding (buffer == source) is allowed.
	size_t html_decode(char * buffer, size_t buflen,
		const char * source, size_t srclen);

	// xml_encode makes data suitable for inside xml attributes and values.
	size_t xml_encode(char * buffer, size_t buflen,
		const char * source, size_t srclen);
	// Note: in-place decoding (buffer == source) is allowed.
	size_t xml_decode(char * buffer, size_t buflen,
		const char * source, size_t srclen);

	// hex_encode shows the hex representation of binary data in ascii.
	size_t hex_encode(char * buffer, size_t buflen,
		const char * source, size_t srclen);
	size_t hex_decode(char * buffer, size_t buflen,
		const char * source, size_t srclen);

	// Apply any suitable string transform (including the ones above) to an STL
	// string.  Stack-allocated temporary space is used for the transformation,
	// so value and source may refer to the same string.
	typedef size_t (*Transform)(char * buffer, size_t buflen,
		const char * source, size_t srclen);
	void transform(std::string& value, size_t maxlen, const std::string& source,
		Transform t);

	// Return the result of applying transform t to source.
	std::string s_transform(const std::string& source, Transform t);

	// Convenience wrappers
	inline std::string s_url_encode(const std::string& source) {
		return s_transform(source, url_encode);
	}
	inline std::string s_url_decode(const std::string& source) {
		return s_transform(source, url_decode);
	}

	// Convert arbitrary values to/from a string.

	template <class T>
    static bool toString(const T &t, std::string* s) {
        std::ostringstream oss;
		oss << t;
		*s = oss.str();
		return !oss.fail();
	}

	template <class T>
    static bool fromString(const std::string& s, T* t) {
        std::ostringstream iss(s);
		iss >> *t;
		return !iss.fail();
	}

	// Inline versions of the string conversion routines.

	template<typename T>
    static inline std::string toString(T val) {
        std::string str; toString(val, &str); return str;
	}

	template<typename T>
    static inline T fromString(const std::string& str) {
        T val; fromString(str, &val); return val;
	}

	// simple function to strip out characters which shouldn't be
	// used in filenames
	char make_char_safe_for_filename(char c);

	//////////////////////////////////////////////////////////////////////

}  // namespace qy

#endif  // QY_STRINGENCODE_H__
