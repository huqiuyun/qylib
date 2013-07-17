
#include "qyosstring.h"
#include "qyos.h"

#include <algorithm>
// For std::swap<>

#define UCHAR_MAX 255

DEFINE_NAMESPACE(qy)

QyString_BaseT::uchar_type QyString_BaseT::stringCase(const QyString_BaseT::uchar_type c,QyCaseSensitivity cs)
{
    if (kCaseNotSensitive == cs) return qyos_lower(c);
    else return c;
}

const QyString_BaseT::char_type* QyString_BaseT::findHelper(const char_type *haystack, size_t hlen,
                       const char_type *needle, size_t nlen, QyCaseSensitivity cs)
{
    const size_t BAD_CHAR_MAX = ((UCHAR_MAX << (sizeof(char_type)-1)) | UCHAR_MAX);

    size_t scan = 0;
    size_t bad_char_skip[BAD_CHAR_MAX + 1]; /* Officially called:
                                          * bad character shift */
    
    /* Sanity checks on the parameters */
    if (nlen <= 0 || !haystack || !needle)
        return NULL;
    
    /* ---- Preprocess ---- */
    /* Initialize the table to default value */
    /* When a character is encountered that does not occur
     * in the needle, we can safely skip ahead for the whole
     * length of the needle.
     */
    for (scan = 0; scan <= BAD_CHAR_MAX; scan = scan + 1)
        bad_char_skip[scan] = nlen;
    
    /* C arrays have the first byte at [0], therefore:
     * [nlen - 1] is the last byte of the array. */
    size_t last = nlen - 1;
    
    /* Then populate it with the analysis of the needle 
     * 遍历 needle 的字符（排除最后一个字符），计算该字符到最后一个字符的位移。
     * 此处遍历需要从头开始，因为 needle 中可能会出现重复字符，同一个字符必须使用其最后出现位置的位移。
     
     * 排除 needle 最后一个字符的原因是：如果 needle 的最后一个字符在 needle 中是唯一的，那么其位移对照表中的值是 nlen，
     * 只要当次搜索匹配失败，并且搜索窗口上 haystack 的最后一个字符就是 needle 的最后一个字符，那么就应该将搜索窗口后移 nlen，
     * 因为该字符没有重复出现 needle 的其它位置上，就可以安全的跳过当前窗口。
     */
    for (scan = 0; scan < last; scan = scan + 1)
        bad_char_skip[stringCase(needle[scan],cs)] = last - scan;
    
    /* ---- Do the matching ---- */
    uchar_type bit = 0;
    /* Search the haystack, while the needle can still be within it. */
    while (hlen >= nlen)
    {
        /* scan from the end of the needle */
        for (scan = last; stringCase(haystack[scan],cs) == stringCase(needle[scan],cs); scan = scan - 1)
            if (scan == 0) /* If the first byte matches, we've found it. */
                return haystack;
        
        /* otherwise, we need to skip some bytes and start again.
         Note that here we are getting the skip value based on the last byte
         of needle, no matter where we didn't match. So if needle is: "abcd"
         then we are skipping based on 'd' and that value will be 4, and
         for "abcdd" we again skip on 'd' but the value will be only 1.
         The alternative of pretending that the mismatched character was
         the last character is slower in the normal case (E.g. finding
         "abcd" in "...azcd..." gives 4 by using 'd' but only
         4-2==2 using 'z'. */
        bit = stringCase(haystack[last],cs);
        hlen     -= bad_char_skip[bit];
        haystack += bad_char_skip[bit];
    }
    return NULL;
}

const QyString_BaseT::char_type* QyString_BaseT::rfindHelper(const char_type *haystack, size_t hlen,
                                                             const char_type *needle, size_t nlen, QyCaseSensitivity cs)
{
    const size_t BAD_CHAR_MAX = ((UCHAR_MAX << (sizeof(char_type)-1)) | UCHAR_MAX);
    
    size_t scan = 0;
    size_t bad_char_skip[BAD_CHAR_MAX + 1]; /* Officially called:
                                             * bad character shift */
    
    /* Sanity checks on the parameters */
    if (nlen == 0 || !haystack || !needle)
        return NULL;
    
    /* ---- Preprocess ---- */
    /* Initialize the table to default value */
    /* When a character is encountered that does not occur
     * in the needle, we can safely skip ahead for the whole
     * length of the needle.
     */
    for (scan = 0; scan <= BAD_CHAR_MAX; scan = scan + 1)
        bad_char_skip[scan] = nlen;
    
    /* C arrays have the first byte at [0], therefore:
     * [nlen - 1] is the last byte of the array. */
    size_t last = nlen - 1;
    /* Then populate it with the analysis of the needle
     * 遍历 needle 的字符（排除最后一个字符），计算该字符到最后一个字符的位移。
     * 此处遍历需要从头开始，因为 needle 中可能会出现重复字符，同一个字符必须使用其最后出现位置的位移。
     
     * 排除 needle 最后一个字符的原因是：如果 needle 的最后一个字符在 needle 中是唯一的，那么其位移对照表中的值是 nlen，
     * 只要当次搜索匹配失败，并且搜索窗口上 haystack 的最后一个字符就是 needle 的最后一个字符，那么就应该将搜索窗口后移 nlen，
     * 因为该字符没有重复出现 needle 的其它位置上，就可以安全的跳过当前窗口。
     */
    for (scan = last; scan > 0; scan = scan - 1)
        bad_char_skip[stringCase(needle[scan],cs)] =  scan;
    
    /* ---- Do the matching ---- */
    uchar_type bit = 0;
    /* Search the haystack, while the needle can still be within it. */
    while (hlen >= nlen)
    {
        /* scan from the end of the needle */
        for (scan = last; stringCase(haystack[hlen-scan],cs) == stringCase(needle[last-scan],cs); scan = scan - 1)
            if (scan == 0) /* If the first byte matches, we've found it. */
                return (haystack + hlen - last);
        
        /* otherwise, we need to skip some bytes and start again.
         Note that here we are getting the skip value based on the last byte
         of needle, no matter where we didn't match. So if needle is: "abcd"
         then we are skipping based on 'd' and that value will be 4, and
         for "abcdd" we again skip on 'd' but the value will be only 1.
         The alternative of pretending that the mismatched character was
         the last character is slower in the normal case (E.g. finding
         "abcd" in "...azcd..." gives 4 by using 'd' but only
         4-2==2 using 'z'. */
        bit = stringCase(haystack[hlen-last],cs);
        hlen     -= bad_char_skip[bit];
    }
    return NULL;
}

QyString_BaseT::char_type QyString_BaseT::NULL_String = 0;

QyString_BaseT::size_type const QyString_BaseT::npos = static_cast< QyString_BaseT::size_type >(-1);

QyString_BaseT::QyString_BaseT (QyAllocator *the_allocator)
: allocator_ (the_allocator ? the_allocator : QyAllocator::instance ())
, length_ (0)
, buf_len_ (0)
, rep_ (&QyString_BaseT::NULL_String)
, release_ (false)
{
    
}

// Constructor that actually copies memory.


QyString_BaseT::QyString_BaseT (const char_type *s,QyAllocator *the_allocator,bool release)
: allocator_ (the_allocator ? the_allocator : QyAllocator::instance ())
, length_ (0)
, buf_len_ (0)
, rep_ (0)
, release_ (false)
{
	this->set (s, release);
}


QyString_BaseT::QyString_BaseT (char_type c, QyAllocator *the_allocator)
: allocator_ (the_allocator ? the_allocator : QyAllocator::instance ())
, length_ (0)
, buf_len_ (0)
, rep_ (0)
, release_ (false)
{
	this->set (&c, 1, true);
}

// Constructor that actually copies memory.


QyString_BaseT::QyString_BaseT (const char_type *s,QyString_BaseT::size_type  len,QyAllocator *the_allocator,bool release)
: allocator_ (the_allocator ? the_allocator : QyAllocator::instance ())
, length_ (0)
, buf_len_ (0)
, rep_ (0)
, release_ (false)
{
	this->set (s, len, release);
}

// Copy constructor.


QyString_BaseT::QyString_BaseT (const QyString_BaseT &s)
: allocator_ (s.allocator_ ? s.allocator_ : QyAllocator::instance ())
, length_ (0)
, buf_len_ (0)
, rep_ (0)
, release_ (false)
{
	this->set (s.rep_, s.length_, true);
}


QyString_BaseT::QyString_BaseT (QyString_BaseT::size_type len,char_type c,QyAllocator *the_allocator)
: allocator_ (the_allocator ? the_allocator : QyAllocator::instance ())
, length_ (0)
, buf_len_ (0)
, rep_ (0)
, release_ (false)
{
	this->resize (len, c);
}


QyString_BaseT::~QyString_BaseT (void)
{
	if (this->buf_len_ != 0 && this->release_)
		this->allocator_->free (this->rep_);
}

// this method might benefit from a little restructuring.
void
QyString_BaseT::set (const QyString_BaseT::char_type *s,
                     QyString_BaseT::size_type len,
                     bool release)
{
	// Case 1. Going from memory to more memory
	size_type new_buf_len = len + 1;
	if (s != 0 && len != 0 && release && this->buf_len_ < new_buf_len)
	{
		char_type *temp = 0;
		qyALLOCATOR (temp,(char_type *) this->allocator_->malloc (new_buf_len * sizeof (char_type)));
        
		if (this->buf_len_ != 0 && this->release_)
			this->allocator_->free (this->rep_);
        
		this->rep_ = temp;
		this->buf_len_ = new_buf_len;
		this->release_ = true;
		this->length_ = len;
		qyos_memcpy (this->rep_, s, len * sizeof (char_type));
		this->rep_[len] = 0;
	}
	else // Case 2. No memory allocation is necessary.
	{
		// Free memory if necessary and figure out future ownership
		if (!release || s == 0 || len == 0)
		{
			if (this->buf_len_ != 0 && this->release_)
			{
				this->allocator_->free (this->rep_);
				this->release_ = false;
			}
		}
		// Populate data.
		if (s == 0 || len == 0)
		{
			this->buf_len_ = 0;
			this->length_ = 0;
			this->rep_ = &QyString_BaseT::NULL_String;
			this->release_ = false;
		}
		else if (!release) // Note: No guarantee that rep_ is null terminated.
		{
			this->buf_len_ = len;
			this->length_ = len;
			this->rep_ = const_cast <char_type *> (s);
			this->release_ = false;
		}
		else
		{
			qyos_memcpy (this->rep_, s, len * sizeof (char_type));
			this->rep_[len] = 0;
			this->length_ = len;
		}
	}
}

// Return substring.
QyString_BaseT QyString_BaseT::substring (QyString_BaseT::size_type offset,
                                          QyString_BaseT::size_type length) const
{
	QyString_BaseT nill;
	size_type count = length;
    
	// case 1. empty string
	if (this->length_ == 0)
		return nill;
    
	// case 2. start pos past our end
	if (offset >= this->length_)
		return nill;
	// No length == empty string.
	else if (length == 0)
		return nill;
	// Get all remaining bytes.
	else if (length == npos || count > (this->length_ - offset))
		count = this->length_ - offset;
    
	return QyString_BaseT (&this->rep_[offset], count, this->allocator_);
}

QyString_BaseT &
QyString_BaseT::append (const QyString_BaseT::char_type* s,QyString_BaseT::size_type slen)
{
	return append(s , slen , this->length_ );
}


QyString_BaseT &
QyString_BaseT::append (const QyString_BaseT::char_type s)
{
	*this += s;
	return *this;
}

QyString_BaseT &
QyString_BaseT::append (const QyString_BaseT::char_type* s)
{
	*this += s;
	return *this;
}

QyString_BaseT &
QyString_BaseT::append (const QyString_BaseT::self_type& s)
{
	*this += s;
	return *this;
}

QyString_BaseT &
QyString_BaseT::append (const char_type* s, size_type slen , size_t index)
{
	if( index > this->length_ )
		index = this->length_;
    
    if (slen > 0 && slen != npos)
	{
		// case 1. No memory allocation needed.
		if (this->buf_len_ >= this->length_ + slen + 1)
		{
			char_type* pend = this->rep_ + index;
			if ( index < this->length_ )
			{
				char_type* pto = this->rep_ + index + slen;
				qyos_memcpy ( pto , pend , (this->length_ - index ) * sizeof (char_type) );
			}
			qyos_memcpy (pend, s   , slen * sizeof (char_type) );
		}
		else // case 2. Memory reallocation is needed
		{
			const size_type new_buf_len = qyos_MAX(this->length_ + slen + 1, this->buf_len_ + this->buf_len_ / 2);
            
			char_type *t = 0 ,*pend =0;
            
			qyALLOCATOR_RETURN (t,(char_type *) this->allocator_->malloc (new_buf_len * sizeof (char_type)), *this);
            
			pend = t + index;
            
			// Copy memory from old string into new string.
			qyos_memcpy (t, this->rep_, index * sizeof (char_type));
		    qyos_memcpy (pend, s , slen * sizeof (char_type) );
            
			if( index < this->length_ )
			{
				char_type* pto = pend + slen;
				char_type* psrc = this->rep_ + index;
				qyos_memcpy ( pto , psrc  , (this->length_ - index ) * sizeof (char_type) );
			}
            
			if (this->buf_len_ != 0 && this->release_)
				this->allocator_->free (this->rep_);
            
			this->release_ = true;
			this->rep_ = t;
			this->buf_len_ = new_buf_len;
		}
        
		this->length_ += slen;
		this->rep_[this->length_] = 0;
	}
    
	return *this;
}

void
QyString_BaseT::insert(size_t index , const QyString_BaseT::char_type* s , size_type slen)
{
	append( s , slen , index);
}

void
QyString_BaseT::insert(size_t index , const QyString_BaseT::char_type* s)
{
	insert(index , s , qyos_strlen(s));
}

void
QyString_BaseT::insert(size_t index , const self_type& str)
{
	insert(index , str.c_str() , str.length());
}

void
QyString_BaseT::truncate(size_t pos)
{
    if (pos < this->buf_len_)
        fast_resize(pos);
}

void
QyString_BaseT::chop(size_t n)
{
    if (n > 0 && this->buf_len_ > n)
        fast_resize(this->buf_len_ - n);
}

void
QyString_BaseT::resize (QyString_BaseT::size_type len,char_type c)
{
	fast_resize(len);
    fill(c,this->buf_len_);
}

void
QyString_BaseT::fill(char_type c, size_t len)
{
    if (len>this->buf_len_)
        len = this->buf_len_;
    qyos_strset (this->rep_, c, len);
}

void
QyString_BaseT::remove(size_t from , size_t to)
{
	if( from >= to )
	{
		return;
	}
	if( from >= this->length_ )
	{
		return ;
	}
	if( to >= this->length_ )
	{
		this->length_ = from;
		this->rep_[ this->length_ ] = '\0';
	}
	else
	{
		qyos_strncpy(this->rep_ + from , this->rep_ + to , (this->length_ - to) );
		this->length_ -= (to - from);
		this->rep_[ this->length_ ] = '\0';
	}
}

void
QyString_BaseT::fast_resize (size_t len)
{
	// Only reallocate if we don't have enough space...
	if (this->buf_len_ <= len)
	{
		if (this->buf_len_ != 0 && this->release_)
			this->allocator_->free (this->rep_);
        
		this->rep_ = static_cast<char_type*>(this->allocator_->malloc ((len + 1) * sizeof (char_type)));
		this->buf_len_ = len + 1;
		this->release_ = true;
		this->rep_[len] = 0;
	}
	else if (len > 0)
	{
		this->rep_[len] = 0;
	}
	this->rep_[0] = 0;
	this->length_ = 0;
}

void
QyString_BaseT::clear (bool release)
{
	// This can't use set(), because that would free memory if release=false
	if (release)
	{
		if (this->buf_len_ != 0 && this->release_)
			this->allocator_->free (this->rep_);
        
		this->rep_ = &QyString_BaseT::NULL_String;
		this->length_ = 0;
		this->buf_len_ = 0;
		this->release_ = false;
	}
	else
	{
		this->fast_clear ();
	}
}

// Assignment operator (does copy memory).
QyString_BaseT &
QyString_BaseT::operator= (const char_type *s)
{
	if (s != 0)
		this->set (s, true);
	return *this;
}

// Assignment operator (does copy memory).
QyString_BaseT &
QyString_BaseT::operator= (const QyString_BaseT &s)
{
	// Check for self-assignment.
	if (this != &s)
	{
		this->set (s.rep_, s.length_, true);
	}
    
	return *this;
}

void
QyString_BaseT::set (const QyString_BaseT::char_type *s, bool release)
{
	size_t length = 0;
	if (s != 0)
		length = qyos_strlen (s);
    
	this->set (s, length, release);
}

void
QyString_BaseT::fast_clear (void)
{
	this->length_ = 0;
	if (this->release_)
	{
		// String retains the original buffer.
		if (this->rep_ != &QyString_BaseT::NULL_String)
			this->rep_[0] = 0;
	}
	else
	{
		// External buffer: string relinquishes control of it.
		this->buf_len_ = 0;
		this->rep_ = &QyString_BaseT::NULL_String;
	}
}

// Get a copy of the underlying representation.

QyString_BaseT::char_type *
QyString_BaseT::rep (void) const
{
	char_type *new_string = 0;
	qyNEW_RETURN (new_string, char_type[this->length_ + 1], 0);
	qyos_strsncpy (new_string, this->rep_, this->length_+1);
    
	return new_string;
}

int
QyString_BaseT::compare (const QyString_BaseT &s,QyCaseSensitivity cs) const
{
    return compare(s.rep(),s.length(),cs);
}

int
QyString_BaseT::compare (const QyString_BaseT::char_type* s, QyString_BaseT::size_type len, QyCaseSensitivity cs) const
{
	if (this->rep_ == s)
		return 0;
    
	// Pick smaller of the two lengths and perform the comparison.
	size_type smaller_length = qyos_MIN (this->length_, len);
    
	int result = (kCaseSensitive==cs)?qyos_memcmp (this->rep_,s,smaller_length * sizeof (char_type)):
    qyos_strnicmp (this->rep_,s,smaller_length);
    
	if (result == 0 && this->length_ != len)
		result = this->length_ > len ? 1 : -1;
	return result;
}


// Comparison operator.

bool
QyString_BaseT::operator== (const QyString_BaseT &s) const
{
	return this->length_ == s.length_ &&
    qyos_memcmp (this->rep_,	s.rep_,this->length_ * sizeof (char_type)) == 0;
}

//! Makes the string lower case.
void QyString_BaseT::make_lower()
{
	for( size_type i = 0; i < this->length_ ; i++ )
		this->rep_[i] = qyos_lower( this->rep_[i] );
}


//! Makes the string upper case.
void QyString_BaseT::make_upper()
{
	for( size_type i = 0; i < this->length_ ; i++ )
		this->rep_[i] = qyos_upper( this->rep_[i] );
}

QyString_BaseT QyString_BaseT::toLower() const
{
    QyString_BaseT lower(length());
    char_type* _rep = lower.rep();
	for( size_type i = 0; i < this->length_ ; i++ )
		_rep[i] = qyos_upper( this->rep_[i] );
    return lower;
}

QyString_BaseT QyString_BaseT::toUpper() const
{
    QyString_BaseT upper(length());
    char_type* _rep = upper.rep();
	for( size_type i = 0; i < this->length_ ; i++ )
		_rep[i] = qyos_upper( this->rep_[i] );
    return upper;
}

//! Replaces all characters of a special type with another one
/** \param toReplace Character to replace.
 \param replaceWith Character replacing the old one. */
void QyString_BaseT::replace(char_type toReplace, char_type replaceWith)
{
	for( size_type i = 0; i < this->length_ ; i++ )
		if (this->rep_[i] == toReplace)
			this->rep_[i] = replaceWith;
}

void QyString_BaseT::replace(const char_type* sRelplace , size_type len , const char_type* s , size_type count)
{
	if (!s)
	{
		return ;
	}
	if (npos == len)
	{
		len = qyos_strlen(sRelplace);
	}
	size_type pos = 0;
	do
	{
		pos = find(sRelplace , len ,kCaseSensitive, pos);
		if( npos == pos )
		{// not find
			break;
		}
		replace(pos , len , s , count);
		// next position
		pos += count;
	}while(1);
}

void QyString_BaseT::replace(size_type pos , size_type len , const char_type* s , size_type count)
{
	if (!s || count == npos || count == 0)
		return ;
    
	if (npos == pos || pos > this->length_ || (pos + len )  > this->length_)
		return ;
    
	if (len > count)
	{
		char_type* pdest = this->rep_ + pos;
		qyos_memcpy( pdest , s , sizeof(char_type)*count );
		qyos_memcpy( pdest + count , pdest + len , sizeof(char_type)*(this->length_ - (pos + len ) )  );
		this->length_ -= (len - count);
		this->rep_[ this->length_ ] = 0;
	}
	else if (len == count)
	{
		qyos_memcpy(this->rep_ + pos , s , count*sizeof(char_type) );
	}
	else
	{
		// case 1. No memory allocation needed.
		size_type diff = (count - len);
		if (this->buf_len_ >= this->length_ + diff + 1)
		{
			char_type* pdest = this->rep_ + pos;
			qyos_memcpy( pdest + count , pdest + len , sizeof(char_type)*( this->length_ - (pos + len)) );
			qyos_memcpy( pdest , s , sizeof(char_type)*count );
		}
		else // case 2. Memory reallocation is needed
		{
			char_type *t = 0 , *pdest =0;
			const size_type new_buf_len = qyos_MAX(this->length_ + diff + 1, this->buf_len_ + this->buf_len_ / 2);
            
			qyALLOCATOR(t,(char_type *) this->allocator_->malloc (new_buf_len * sizeof (char_type)));
            
			pdest = t + pos;
            
			// Copy memory from old string into new string.
			qyos_memcpy ( t, this->rep_ , sizeof (char_type)*pos );
			qyos_memcpy ( pdest , s , sizeof (char_type)*count );
			qyos_memcpy ( pdest + count , this->rep_ + pos + len , sizeof (char_type)*(this->length_ - (pos + len) ) );
            
			if (this->buf_len_ != 0 && this->release_)
				this->allocator_->free (this->rep_);
            
			this->release_ = true;
			this->rep_ = t;
			this->buf_len_ = new_buf_len;
		}
		this->length_ += diff;
		this->rep_[this->length_] = 0;
	}
}

void QyString_BaseT::replace(const QyString_BaseT::char_type *sReplace , const QyString_BaseT::char_type* s)
{
	replace(sReplace , qyos_strlen(sReplace) , s , qyos_strlen(s) );
}

void QyString_BaseT::replace(const QyString_BaseT::self_type& sReplace , const QyString_BaseT::self_type& s)
{
	replace(sReplace.c_str() , sReplace.length() , s.c_str() , s.length());
}
//! Erases a character from the string.
/** May be slow, because all elements
 following after the erased element have to be copied.
 \param index: Index of element to be erased. */
void QyString_BaseT::erase(size_type index , size_t len)
{
	if( index >= this->length_ )
		return ;
    
	if( index + len >= this->length_ )
	{
		len = this->length_ - index ;
	}
	else
	{
		char_type * pto = this->rep_ + index;
		char_type * psrc= pto + len;
		qyos_memcpy( pto , psrc , (this->length_ - ( index + len ))*sizeof(char_type) );
	}
	this->length_-= len;
	this->rep_[ this->length_ ] = '\0';
}

//! Trims the string.
/** Removes the specified characters (by default, Latin-1 whitespace)
 from the begining and the end of the string. */
QyString_BaseT::self_type&
QyString_BaseT::trim(const self_type& whitespace)
{
	// find start and end of the substring without the specified characters
	size_t begin = qyos_findFirstCharNotInList<char_type>(this->c_str() , this->length() , whitespace.c_str(), whitespace.length() );
	if (begin == SIZE_MAX)
		return (*this = CharTNil);
    
	int end = qyos_findLastCharNotInList<char_type>(this->c_str() , this->length() , whitespace.c_str(), whitespace.length() );
    
	return (*this = substring(begin, (end +1) - begin) );
}

QyString_BaseT::size_type
QyString_BaseT::find (const QyString_BaseT&str, QyCaseSensitivity cs,QyString_BaseT::size_type pos) const
{
	return this->find (str.rep_, str.length_, cs, pos);
}

bool
QyString_BaseT::startsWith(const QyString_BaseT&str,QyCaseSensitivity cs) const
{
    return (find(str,cs) == 0);
}

bool
QyString_BaseT::startsWith(const QyString_BaseT::char_type *s , size_type len,QyCaseSensitivity cs) const
{
    return (find(s,len,cs) == 0);
}

QyString_BaseT::size_type
QyString_BaseT::find (const QyString_BaseT::char_type *s , size_type len , QyCaseSensitivity cs,QyString_BaseT::size_type pos) const
{
	const char_type *pointer = 0;
    if (len == 1)
        return find(s[0],cs,pos);
    
    if (pos < this->length_){
		if (QyString_BaseT::npos == len) {
			len = qyos_strlen (s);
        }
		const char_type *substr = this->rep_ + pos;
#if 0
		pointer = (kCaseSensitive == cs)?qyos_strnstr(substr, (this->length_ - pos), s, len): qyos_strnistr(substr, (this->length_ - pos), s,len);
#else
        pointer = findHelper(substr, (this->length_ - pos), s, len, cs);
#endif
	}
	return (pointer == 0)? npos: pointer - this->rep_;
}

QyString_BaseT::size_type
QyString_BaseT::find (QyString_BaseT::char_type c,QyCaseSensitivity cs,QyString_BaseT::size_type pos) const
{
    char_type *pointer = 0;
    if (pos < this->length_)
    {
        char_type *substr = this->rep_ + pos;
        pointer = (kCaseSensitive == cs)?qyos_strnchr(substr, c, this->length_ - pos):
        qyos_strnichr(substr, c, this->length_ - pos);
    }
    return (pointer == 0)? npos: pointer - this->rep_;
}

bool
QyString_BaseT::startsWith(QyString_BaseT::char_type c,QyCaseSensitivity cs) const
{
    return (find(c,cs) == 0);
}

QyString_BaseT::size_type
QyString_BaseT::rfind (QyString_BaseT::char_type c,QyCaseSensitivity cs,QyString_BaseT::size_type rpos) const
{
    if (this->length_ == 0)
        return QyString_BaseT::npos;
    
	if (rpos == QyString_BaseT::npos || rpos >= this->length_)
		rpos = this->length_ - 1;
    
	// Do not change to prefix operator!  Proper operation of this loop
	// depends on postfix decrement behavior.
	do
    {
        if (kCaseSensitive == cs){
            if (this->rep_[rpos] == c)
                return rpos;
        }else{
            if (qyos_lower(this->rep_[rpos]) == qyos_lower(c))
                return rpos;
        }
        if (rpos==0){
            break;
        }
        rpos--;
    }while(1);
	return QyString_BaseT::npos;
}

QyString_BaseT::size_type
QyString_BaseT::rfind (const self_type &str, QyCaseSensitivity cs,size_type rpos) const
{
    return rfind(str.c_str(), str.length(), cs, rpos);
}

QyString_BaseT::size_type
QyString_BaseT::rfind (const char_type *s , size_type len , QyCaseSensitivity cs,size_type rpos) const
{
    if (this->length_ == 0)
        return QyString_BaseT::npos;
    
	if (rpos == QyString_BaseT::npos || rpos > this->length_)
		rpos = this->length_ - 1;
    
    if (rpos < len)
        return QyString_BaseT::npos;

	// Do not change to prefix operator!  Proper operation of this loop
	// depends on postfix decrement behavior.
    const char_type* pointer = rfindHelper(this->rep_, rpos, s, len, cs);
    
	return (NULL ==pointer) ? QyString_BaseT::npos:(pointer - this->rep_);
}

void
QyString_BaseT::swap (QyString_BaseT & str)
{
	std::swap (this->allocator_ , str.allocator_);
	std::swap (this->length_       , str.length_);
	std::swap (this->buf_len_   , str.buf_len_);
	std::swap (this->rep_       , str.rep_);
	std::swap (this->release_   , str.release_);
}

qyINLINE void
QyString_BaseT::dump (void) const
{
}

// Assignment method (does not copy memory)
qyINLINE QyString_BaseT &
QyString_BaseT::assign_nocopy (const QyString_BaseT &s)
{
	this->set (s.rep_, s.length_, false);
	return *this;
}

qyINLINE QyString_BaseT::size_type
QyString_BaseT::length (void) const
{
	return this->length_;
}

qyINLINE QyString_BaseT::size_type
QyString_BaseT::size (void) const
{
	return this->length_;
}

qyINLINE size_t
QyString_BaseT::capacity (void) const
{
	return this->buf_len_;
}

qyINLINE bool
QyString_BaseT::empty (void) const
{
	return this->length_ == 0;
}

qyINLINE QyString_BaseT
QyString_BaseT::substr (QyString_BaseT::size_type offset,
                        QyString_BaseT::size_type length) const
{
	return this->substring (offset, length);
}

// Return the <slot'th> character in the string.

qyINLINE const QyString_BaseT::char_type &
QyString_BaseT::operator[] (QyString_BaseT::size_type slot) const
{
	return this->rep_[slot];
}

// Return the <slot'th> character in the string by reference.

qyINLINE QyString_BaseT::char_type &
QyString_BaseT::operator[] (QyString_BaseT::size_type slot)
{
	return this->rep_[slot];
}

qyINLINE const QyString_BaseT::char_type *
QyString_BaseT::fast_rep (void) const
{
	return this->rep_;
}

qyINLINE const QyString_BaseT::char_type *
QyString_BaseT::c_str (void) const
{
	return this->rep_;
}

// Less than comparison operator.

qyINLINE bool
QyString_BaseT::operator < (const QyString_BaseT &s) const
{
	return compare (s) < 0;
}

// Greater than comparison operator.

qyINLINE bool
QyString_BaseT::operator > (const QyString_BaseT &s) const
{
	return compare (s) > 0;
}

// Comparison operator.

qyINLINE bool
QyString_BaseT::operator!= (const QyString_BaseT &s) const
{
	return !(*this == s);
}

qyINLINE bool
QyString_BaseT::operator!= (const char_type *s) const
{
	return !(*this == s);
}

qyINLINE QyString_BaseT::size_type
QyString_BaseT::strstr (const QyString_BaseT &s,QyCaseSensitivity cs) const
{
	return this->find (s.rep_,s.length_,cs);
}

qyINLINE bool
operator== (const QyString_BaseT::char_type *s,const QyString_BaseT &t)
{
	return t == s;
}

qyINLINE bool
operator!= (const QyString_BaseT::char_type *s,const QyString_BaseT &t)
{
	return !(t == s);
}

bool
QyString_BaseT::operator== (const QyString_BaseT::char_type *s) const
{
	size_t len = qyos_strlen (s);
	return this->length_ == len &&
    qyos_memcmp (this->rep_,s,len * sizeof (char_type)) == 0;
}

////////////////////////////////////////////////////////////////////////////////
QyString_BaseT
operator+ (const QyString_BaseT &s, const QyString_BaseT &t)
{
	QyString_BaseT temp (s.length () + t.length ());
	temp += s;
	temp += t;
	return temp;
}

QyString_BaseT
operator+ (const QyString_BaseT::char_type *s, const QyString_BaseT &t)
{
	size_t slen = 0;
	if (s != 0)
		slen = qyos_strlen (s);
	QyString_BaseT temp (slen + t.length ());
	if (slen > 0)
		temp.append (s, slen);
	temp += t;
	return temp;
}

QyString_BaseT
operator+ (const QyString_BaseT &s, const QyString_BaseT::char_type *t)
{
	size_t tlen = 0;
	if (t != 0)
		tlen = qyos_strlen (t);
	QyString_BaseT temp (s.length () + tlen);
	temp += s;
	if (tlen > 0)
		temp.append (t, tlen);
	return temp;
}

QyString_BaseT
operator + (const QyString_BaseT &t,const QyString_BaseT::char_type c)
{
	QyString_BaseT temp (t.length () + 1);
	temp += t;
	temp += c;
	return temp;
}

QyString_BaseT
operator + (const QyString_BaseT::char_type c,const QyString_BaseT &t)
{
	QyString_BaseT temp (t.length () + 1);
	temp += c;
	temp += t;
	return temp;
}


QyString_BaseT &
QyString_BaseT::operator+= (const QyString_BaseT::char_type* s)
{
	size_t slen = 0;
	if (s != 0)
		slen = qyos_strlen (s);
	return this->append (s, slen);
}


QyString_BaseT &
QyString_BaseT::operator+= (const QyString_BaseT &s)
{
	return this->append (s.rep_, s.length_);
}


QyString_BaseT &
QyString_BaseT::operator+= (const QyString_BaseT::char_type c)
{
	const size_type slen = 1;
	return this->append (&c, slen);
}

int  QyString_BaseT::toInt(void) const
{
	return qyos_atoi( c_str() );
}

float QyString_BaseT::toFloat() const
{
    return qyos_atof(c_str());
}

int64 QyString_BaseT::toInt64() const
{
    return qyos_atoi64(c_str());
}

void QyString_BaseT::format(const char_type* fmt , ...)
{
	va_list argp;
	va_start(argp, fmt);
    
	int length = qyos_vsnprintf(NULL, 0, fmt, argp);
	if (-1 == length || length == 0 )
	{
		va_end(argp);
		return ;
	}
	this->fast_resize(length);
	qyos_vsnprintf(this->rep_, length, fmt, argp);
	va_end(argp);
}

void QyString_BaseT::path()
{
	if (length_ <= 0)
	{
		return ;
	}
	qyutil_safePath(rep_,length_);
}

size_t QyString_BaseT::range(const size_t length ,
							 const size_t strLen ,
							 size_t& from , const size_t& cpsize)
{
	if (from == SIZE_MAX)
	{
		from = 0;
	}
	if (from >= length || strLen == 0)
	{
		return 0;
	}
	size_t outsize = cpsize;
	if (outsize > 0)
	{
		if ((from + outsize) > length)
		{
			outsize = length - from;
		}
	}
	else
	{
		outsize = length - from;
	}
	
	if (strLen < outsize)
	{
		outsize = strLen;
	}
	return outsize;
}

size_t  QyString_BaseT::copyTo(char_type* str , const size_t strLen , size_t from , size_t cpsize)
{
	size_t ifrom = from;
	size_t isize = cpsize;
	size_t outsize = range(length_ , strLen ,ifrom , isize );
	if (outsize > 0)
	{
		qyos_memcpy((void*)str , (const void*)(rep_ + ifrom) , sizeof(char_type)*outsize);
	}
	return outsize;
}

size_t  QyString_BaseT::copyTo(self_type& str , size_t from , size_t cpsize)
{
	size_t ifrom = from;
	size_t isize = cpsize;
	size_t outsize = range(length_ , npos , ifrom , isize );
	if (outsize > 0)
	{
		str.fast_resize(outsize);
		str.set( rep_ + ifrom, outsize , true);
	}
	return outsize;
}

QyString_BaseT::char_type* QyString_BaseT::data(void) const
{
	return rep_;
}

END_NAMESPACE(qy)