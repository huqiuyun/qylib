DEFINE_NAMESPACE(qy)

class QYUTIL_API QyString_BaseT
{
public:
    typedef size_t             size_type;
    typedef CharT              char_type;
    typedef unsigned CharT     uchar_type;
	typedef QyString_BaseT     self_type;
	static size_type const     npos;
	/**
	*  Default constructor.
	*
	*  @param the_allocator QyAllocator associated with string
	*  @return Default QyString_BaseT string.
	*/
	QyString_BaseT (QyAllocator *the_allocator = 0);

	/**
	* Constructor that copies @a s into dynamically allocated memory.
	*
	* if release == true then a new buffer is allocated internally, and
	*   s is copied to the internal buffer.
	* if release == false then the s buffer is used directly. If s == 0
	*   then it will _not_ be used, and instead the internal buffer
	*   is set to NULL_String.
	*
	* @param s Zero terminated input string
	* @param the_allocator QyAllocator associated with string
	* @param release Allocator responsible(true)/not reponsible(false) for
	*    freeing memory.
	* @return QyString_BaseT containing const char_type *s
	*/
	QyString_BaseT (const char_type *s,QyAllocator *the_allocator = 0,bool release = true);

	/**
	* Constructor that copies @a len CHARs of @a s into dynamically
	* allocated memory (will zero terminate the result).
	*
	* if release == true then a new buffer is allocated internally.
	*   s is copied to the internal buffer.
	* if release == false then the s buffer is used directly. If s == 0
	*   then it will _not_ be used, and instead the internal buffer
	*   is set to NULL_String.
	*
	* @param s Non-zero terminated input string
	* @param len Length of non-zero terminated input string
	* @param the_allocator QyAllocator associated with string
	* @param release Allocator responsible(true)/not reponsible(false) for
	*    freeing memory.
	* @return QyString_BaseT containing const char_type *s
	*/
	QyString_BaseT (const char_type *s,size_type len,QyAllocator *the_allocator = 0,	bool release = true);

	/**
	*  Copy constructor.
	*
	*  @param s Input QyString_BaseT string to copy
	*  @return Copy of input string @a s
	*/
	QyString_BaseT (const self_type &s);

	/**
	*  Constructor that copies @a c into dynamically allocated memory.
	*
	*  @param c Single input character.
	*  @param the_allocator QyAllocator associated with string
	*  @return QyString_BaseT containing char_type 'c'
	*/
	QyString_BaseT (char_type c, QyAllocator *the_allocator = 0);

	/**
	*  Constructor that allocates a len long string.
	*
	*  Warning : This constructor was incorrectly documented in the past.
	*  It simply calls resize(len, c).
	*  It is probably not advisable to use the second parameter. See
	*  resize() for more information.
	*
	*  @param len Amount of space to reserve for the string.
	*  @param c The array is filled with c's
	*  @param the_allocator QyAllocator associated with string
	*  @return Empty QyString_BaseT with room for len CHARs
	*/
	QyString_BaseT (size_type len,	char_type c = 0,QyAllocator *the_allocator = 0);

	/**
	*  Deletes the memory...
	*/
	virtual ~QyString_BaseT (void);

	/**
	* Return the <slot'th> character in the string (doesn't perform
	* bounds checking).
	*
	* @param slot Index of the desired character
	* @return The character at index @a slot
	*/
	const char_type & operator[] (size_type slot) const;

	/**
	* Return the <slot'th> character by reference in the string
	* (doesn't perform bounds checking).
	*
	* @param slot Index of the desired character
	* @return The character at index @a slot
	*/
	char_type & operator[] (size_type slot);

	/**
	*  Assignment operator (does copy memory).
	*
	*  @param s Input null-terminated char_type string to assign to this object.
	*  @return Return a copy of the this string.
	*/
	QyString_BaseT &operator = (const char_type * s);

	/**
	*  Assignment operator (does copy memory).
	*
	*  @param s Input QyString_BaseT string to assign to this object.
	*  @return Return a copy of the this string.
	*/
	QyString_BaseT &operator = (const self_type &s);

	/**
	*  Assignment alternative method (does not copy memory).
	*
	*  @param s Input QyString_BaseT string to assign to this object.
	*  @return Return this string.
	*/
	QyString_BaseT  &assign_nocopy (const self_type &s);

	/**
	* Copy @a s into this @a QyString_BaseT.
	*
	* If release == true then a new buffer is allocated internally if the
	*   existing one is not big enough to hold s. If the existing
	*   buffer is big enough, then it will be used. This means that
	*   set(*, 1) can be illegal when the string is constructed with a
	*   const char*. (e.g. QyString_BaseT("test", 0, false)).
	*
	* if release == false then the s buffer is used directly, and any
	*   existing buffer is destroyed. If s == 0 then it will _not_ be
	*   used, and instead the internal buffer is set to NULL_String.
	*
	* @param s Null terminated input string
	* @param release Allocator responsible(true)/not reponsible(false) for
	*    freeing memory.
	*/
	void set (const char_type * s, bool release = true);

	/**
	*  Copy @a len bytes of @a s (will zero terminate the result).
	*
	* If release == true then a new buffer is allocated internally if the
	*   existing one is not big enough to hold s. If the existing
	*   buffer is big enough, then it will be used. This means that
	*   set(*, *, 1) is illegal when the string is constructed with a
	*   non-owned const char*. (e.g. QyString_BaseT("test", 0, 0))
	*
	* If release == false then the s buffer is used directly, and any
	*   existing buffer is destroyed. If s == 0 then it will _not_ be
	*   used, and instead the internal buffer is set to NULL_String.
	*
	*  @param s Non-zero terminated input string
	*  @param len Length of input string 's'
	*  @param release Allocator responsible(true)/not reponsible(false) for
	*    freeing memory.
	*/
	void set (const char_type * s, size_type len, bool release);

	/**
	* Clear this string. Memory is _not_ freed if @a release is false.
	*
	* Warning: This method was incorrectly documented in the past, but
	* the current implementation has been changed to match the documented
	* behavior.
	*
	* Warning: clear(false) behaves like fast_clear() below.
	*
	* @param release Memory is freed if true, and not freed if false.
	*/
	void clear (bool release = false);

	/**
	* A more specialized version of clear(): "fast clear". fast_clear()
	* resets the string to 0 length. If the string owns the buffer
	* (@arg release_== true):
	*  - the string buffer is not freed
	*  - the first character of the buffer is set to 0.
	*
	* If @arg release_ is false (this object does not own the buffer):
	*  - the buffer pointer is reset to the NULL_String and does not
	*    maintain a pointer to the caller-supplied buffer on return
	*  - the maximum string length is reset to 0.
	*
	* Warning : Calling clear(false) or fast_clear() can have unintended
	*   side-effects if the string was constructed (or set()) with an
	*   external buffer. The string will be disassociated with the buffer
	*   and the next append() or +=() will cause a new buffer to be
	*   allocated internally.
	*/
	void fast_clear (void);

	/**
	* Return a substring given an offset and length.
	* If length == @c npos use rest of str.  Return empty substring if
	* offset or offset/length are invalid.
	*
	* @param offset Index of first desired character of the substring.
	* @param length How many characters to return starting at the offset.
	* @return The string containing the desired substring
	*/
	QyString_BaseT substring (size_type offset,size_type length = npos) const;

	/**
	*  Same as <substring>.
	*
	* @param offset Index of first desired character of the substring.
	* @param length How many characters to return starting at the offset.
	* @return The string containing the desired substring
	*/
	QyString_BaseT substr (size_type offset,size_type length = npos) const;

	/**
	*  Concat operator (copies memory).
	*
	*  @param s Input QyString_BaseT string to concatenate to this string.
	*  @return The combined string (input append to the end of the old). New
	*    string is zero terminated.
	*/
	QyString_BaseT  &operator += (const self_type &s);

	/**
	*  Concat operator (copies memory).
	*
	*  @param s Input C string to concatenate to this string.
	*  @return The combined string (input append to the end of the old). New
	*    string is zero terminated.
	*/
	QyString_BaseT & operator += (const char_type* s);

	/**
	*  Concat operator (copies memory).
	*
	*  @param c Input char_type to concatenate to this string.
	*  @return The combined string (input append to the end of the old). New
	*    string is zero terminated.
	*/
	QyString_BaseT & operator += (const char_type c);

	/**
	*  Append function (copies memory).
	*
	*  @param s Input char_type array to concatenate to this string.
	*  @param slen The length of the array.
	*  @return The combined string (input append to the end of the old). New
	*    string is zero terminated.
	*/
	QyString_BaseT & append (const char_type s);
	QyString_BaseT & append (const char_type* s);
	QyString_BaseT & append (const self_type& s);
	QyString_BaseT & append (const char_type* s, size_type slen);
	QyString_BaseT & append (const char_type* s, size_type slen , size_t index);
    //
    void prepend(const self_type& str) { this->insert(0, str); }
    void prepend(const char_type* text) { this->insert(0, text); }
    void prepend(const char_type* text, size_t len) { this->insert(0, text, len); }
	//
	void insert(size_t index , const char_type* s , size_type slen);
	void insert(size_t index , const char_type* s);
	void insert(size_t index , const self_type& str);

	/** delete index from To to string ,BUT not free alloced memory */
	void remove(size_t from , size_t to);
	/**
	*  Return the length of the string.
	*
	*  @return Length of stored string
	*/
	size_type length (void) const;
	size_type size (void) const;

	/**
	*  Return the number of allocated CHARs in the string object.
	*  This may be greater than the current length of the string.
	*
	*  @return Maximum number of char_type units that can be stored, including
	*          any terminating nul that may be needed.
	*/
	size_t capacity (void) const;

	/**
	* Return @c true if the length of the string is zero, else @c false.
	*/
	bool empty (void) const;

	/**
	* Get a copy of the underlying representation.
	*
	* This method allocates memory for a copy of the string and returns
	* a pointer to the new area. The caller is responsible for freeing
	* the memory when finished; use delete []
	*
	* @return Pointer reference to the string data. Returned string is
	*    zero terminated.
	*/
	char_type *rep (void) const;

	/**
	* Get at the underlying representation directly!
	* _Don't_ even think about casting the result to (char *) and modifying it,
	* if it has length 0!
	*
	* @return Pointer reference to the stored string data. No guarantee is
	*    that the string is zero terminated.
	*
	*/
	const char_type *fast_rep (void) const;

	/**
	*  Same as STL String's <c_str> and <fast_rep>.
	*/
	const char_type *c_str (void) const;
	operator const char_type* () const
	{
		return rep_;
	}
	char_type* data(void) const;

	/**
	*  Comparison operator that will match substrings.  Returns the
	*  slot of the first location that matches, else @c npos.
	*
	*  @param s Input QyString_BaseT string
	*  @return Integer index value of the first location of string @a s or
	*          @c npos (not found).
	*/
	size_type strstr (const self_type &s,QyCaseSensitivity cs = kCaseSensitive) const;

	//! Makes the string lower case.
	void make_lower();


	//! Makes the string upper case.
	void make_upper();

	//! Replaces all characters of a special type with another one
	/** \param toReplace Character to replace.
	\param replaceWith Character replacing the old one. */
	void replace(char_type toReplace, char_type replaceWith);
	void replace(const char_type *sRelplace , size_type len , const char_type* s , size_type count);
	void replace(size_type beginpos , size_type len , const char_type* s , size_type count); 
	void replace(const char_type *sRelplace , const char_type* s);
    void replace(const self_type& sRelplace , const self_type& s);
	/**
	*  Find <str> starting at pos.  Returns the slot of the first
	*  location that matches (will be >= pos), else @c npos.
	*
	*  @param str Input QyString_BaseT string to search for in stored string.
	*  @param pos Starting index position to start searching for string @a str.
	*  @return Index value of the first location of string @a str else
	*          @c npos.
	*/
	size_type find (const self_type &str, QyCaseSensitivity cs = kCaseSensitive,size_type pos = 0) const;
	bool startsWith(const self_type &str, QyCaseSensitivity cs = kCaseSensitive) const;
    /**
	*  Find @a s starting at pos.  Returns the slot of the first
	*  location that matches (will be >= pos), else @c npos.
	*
	*  @param s non-zero input string to search for in stored string.
	*  @param len s length, can is -1
	*  @param pos Starting index position to start searching for string @a str.
	*  @return Index value of the first location of string @a str else
	*          @c npos.
	*/
	size_type find (const char_type *s , size_type len , QyCaseSensitivity cs = kCaseSensitive,size_type pos = 0) const;
	bool startsWith(const char_type* s, size_type len, QyCaseSensitivity cs = kCaseSensitive) const;

	/**
	*  Find @a c starting at pos.  Returns the slot of the first
	*  location that matches (will be >= pos), else @c npos.
	*
	*  @param c Input character to search for in stored string.
	*  @param pos Starting index position to start searching for string @a str.
	*  @return Index value of the first location of string @a str else
	*          @c npos.
	*/
	size_type find (char_type c, QyCaseSensitivity cs = kCaseSensitive, size_type pos = 0) const;
	bool startsWith(char_type c, QyCaseSensitivity cs = kCaseSensitive) const;

	/**
	*  Find @a c starting at pos (counting from the end).  Returns the
	*  slot of the first location that matches, else @c npos.
	*
	*  @param c Input character to search for in stored string.
	*  @param rpos Starting index position to start searching for string @a str.
	*  @return Index value of the first location of string @a str else
	*          @c npos.
	*/
	size_type rfind (char_type c, QyCaseSensitivity cs = kCaseSensitive, size_type rpos = npos) const;
    size_type rfind (const self_type &str, QyCaseSensitivity cs = kCaseSensitive,size_type rpos = npos) const;
    size_type rfind (const char_type *s , size_type len , QyCaseSensitivity cs = kCaseSensitive,size_type rpos = npos) const;
	//! Erases a character from the string.
	/** May be slow, because all elements
	following after the erased element have to be copied.
	\param index: Index of element to be erased. */
	void erase(size_type index , size_type len = 1);

	//! Trims the string.
	/** Removes the specified characters (by default, Latin-1 whitespace)
	from the begining and the end of the string. */
    self_type& trim(const self_type& whitespace);
	/**
	*  Equality comparison operator (must match entire string).
	*
	* @param s Input QyString_BaseT string to compare against stored string.
	* @return @c true if equal, @c false otherwise.
	*/
	bool operator == (const self_type &s) const;

	/**
	*  Equality comparison operator (must match entire string).
	*
	* @param s Null terminated string to compare against stored string.
	* @return @c true if equal, @c false otherwise.
	*/
	bool operator == (const char_type *s) const;

	/**
	*  Less than comparison operator.
	*
	*  @param s Input QyString_BaseT string to compare against stored string.
	*  @return @c true if less than, @c false otherwise.
	*/
	bool operator < (const self_type &s) const;

	/**
	*  Greater than comparison operator.
	*
	*  @param s Input QyString_BaseT string to compare against stored string.
	*  @return @c true if greater than, @c false otherwise.
	*/
	bool operator > (const self_type &s) const;

	/**
	*  Inequality comparison operator.
	*
	*  @param s String to compare against stored string.
	*  @return @c true if not equal, @c false otherwise.
	*/
	bool operator != (const self_type &s) const;

	/**
	*  Inequality comparison operator.
	*
	*  @param s Null terminated string to compare against stored string.
	*  @return @c true if not equal, @c false otherwise.
	*/
	bool operator != (const char_type *s) const;

	/**
	*  Performs a strncmp comparison.
	*
	*  @param s Input QyString_BaseT string to compare against stored string.
	*  @return Integer value of result (less than 0, 0, greater than 0)
	*    depending on how input string @a s is to the stored string.
	*/
	int compare (const self_type &s,QyCaseSensitivity cs = kCaseSensitive) const;
    int compare (const char_type* s, size_type len, QyCaseSensitivity cs = kCaseSensitive) const;

	/**
	*  Dump the state of an object.
	*/
	void dump (void) const;

    /** Truncates the string at the given  pos index. */
    void truncate(size_t pos);
    
    /**
     Removes n characters from the end of the string.
     */
    void chop(size_t n);
	/**
	* This method is designed for high-performance. Please use with
	* care ;-)
	*
	* Warning : This method was documented incorrectly in the past.
	* The original intention was to change the length of the string to
	* len, and to fill the whole thing with c CHARs.
	* However, what was actually done was to set the length of the
	* string to zero, and fill the buffer with c's. The buffer was
	* also not null-terminated unless c happened to be zero.
	* Rather than fix the method to work as documented, the code is
	* left as is, but the second parameter should probably not be used.
	*
	* fast_resize just adjusts the buffer if needed and sets the length,
	* it doesn't fill the buffer, so is much faster.
	* 
	* @param len The number of CHARs to reserve
	* @param c The char_type to use when filling the string.
	*/
	void resize (size_type len, char_type c = 0);
	void fast_resize (size_t len);
    void fill(char_type c, size_t len);
    
	/// Swap the contents of this @c QyString_BaseT with @a str.
	/**
	* @note This is non-throwing operation.
	*/
	void swap (self_type & str);

	/** convert to number 
	 * @example:
	 * str ="1234";
	 * int i = str.toNumber();
	 *
	*/
	int  toInt() const;
    float toFloat() const;
    int64 toInt64() const;
    QyString_BaseT toLower() const;
    QyString_BaseT toUpper() const;

	/** format chars 
	 * @example:
	 *  str.format("%s,%s","2334","hello");
	 *  OR 
	 *  str.format(L"%s,%s,%d",L"2334",L"hello",123);
	*/
	void format(const char_type* fmt ,...);

	/**  if WIN32 '/' convert '\\' else '\\' convert '/' */
	void path();

	size_t  copyTo(char_type* str , const size_t strLen , size_t from , size_t cpsize);
	size_t  copyTo(self_type& str , size_t from , size_t cpSize);

	static size_t range(const size_t length ,const size_t strLen ,size_t& from/*[in|out]*/ , const size_t& cpsize);
    
    static uchar_type stringCase(const uchar_type c,QyCaseSensitivity cs);
    
    static const char_type*  findHelper(const char_type *haystack, size_t hlen,
                                        const char_type *needle, size_t nlen, QyCaseSensitivity cs);
    
    static const char_type* rfindHelper(const char_type *haystack, size_t hlen,
                                        const char_type *needle, size_t nlen, QyCaseSensitivity cs);
protected:
	/**
	*  Pointer to a memory allocator.
	*/
	QyAllocator* allocator_;

	/**
	*  Length of the QyString_BaseT data (not counting the trailing '\0').
	*/
	size_type length_;

	/**
	*  Length of the QyString_BaseT data buffer.  Keeping track of the
	*  length allows to avoid unnecessary dynamic allocations.
	*/
	size_type buf_len_;

	/**
	*  Pointer to data.
	*/
	char_type *rep_;

	/**
	*  Flag that indicates if we own the memory
	*/
	bool release_;

	/**
	*  Represents the "NULL" string to simplify the internal logic.
	*/
	static char_type NULL_String;
};

QYUTIL_API 
QyString_BaseT operator+ (const QyString_BaseT &s, const QyString_BaseT &t);

QYUTIL_API 
QyString_BaseT operator+ (const QyString_BaseT::char_type *s, const QyString_BaseT &t);

QYUTIL_API 
QyString_BaseT operator+ (const QyString_BaseT &s, const QyString_BaseT::char_type *t);

QYUTIL_API 
QyString_BaseT operator + (const QyString_BaseT &t,const QyString_BaseT::char_type c);

QYUTIL_API 
QyString_BaseT operator + (const QyString_BaseT::char_type c,const QyString_BaseT &t);

END_NAMESPACE(qy)