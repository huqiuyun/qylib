#ifndef __QY_CRYPTSTRING_H__
#define __QY_CRYPTSTRING_H__

#include <string>
#include "qyutil/qydefine.h"
#include "qyutil/qyscoped_ptr.h"

DEFINE_NAMESPACE(qy)

class QyCryptStringImpl
{
public:
	virtual ~QyCryptStringImpl() {}
	virtual size_t length() const = 0;
	virtual void copyTo(char * dest, bool nullterminate) const = 0;
	virtual std::string urlEncode() const = 0;
	virtual QyCryptStringImpl * copy() const = 0;
};

class QyEmptyCryptStringImpl : public QyCryptStringImpl
{
public:
	virtual ~QyEmptyCryptStringImpl()
	{}
	virtual size_t length() const { return 0; }
	virtual void copyTo(char * dest, bool nullterminate) const
	{
		if (nullterminate)
		{
			*dest = '\0';
		}
	}
	virtual std::string urlEncode() const { return ""; }
	virtual QyCryptStringImpl * copy() const { return new QyEmptyCryptStringImpl(); }
};

class QyCryptString
{
public:
	QyCryptString()
		: crypt_(new QyEmptyCryptStringImpl())
	{}
	QyCryptString(const QyCryptString & other) 
		: crypt_(other.crypt_->copy())
	{}
	explicit QyCryptString(const QyCryptStringImpl & impl) 
		: crypt_(impl.copy())
	{}
	QyCryptString & operator=(const QyCryptString & other)
	{
		if (this != &other)
		{
			crypt_.reset(other.crypt_->copy());
		}
		return *this;
	}
public:
	size_t length() const { return crypt_->length(); }

	void copyTo(char * dest, bool nullterminate) const
	{ crypt_->copyTo(dest, nullterminate); }

	void clear() { crypt_.reset(new QyEmptyCryptStringImpl()); }

	std::string urlEncode() const { return crypt_->urlEncode(); }

private:
	scoped_ptr<const QyCryptStringImpl> crypt_;
};


// Used for constructing strings where a password is involved and we
// need to ensure that we zero memory afterwards
class QyFormatCryptString
{
public:
	QyFormatCryptString()
	{
		storage_ = new char[32];
		capacity_ = 32;
		length_ = 0;
		storage_[0] = 0;
	}

	void append(const std::string & text)
	{
		append(text.data(), text.length());
	}

	void append(const char * data, size_t length)
	{
		ensureStorage(length_ + length + 1);
		memcpy(storage_ + length_, data, length);
		length_ += length;
		storage_[length_] = '\0';
	}

	void append(const QyCryptString * password) 
	{
		size_t len = password->length();
		ensureStorage(length_ + len + 1);
		password->copyTo(storage_ + length_, true);
		length_ += len;
	}

	size_t length()
	{
		return length_;
	}

	const char * data()
	{
		return storage_;
	}


	// Ensures storage of at least n bytes
	void ensureStorage(size_t n)
	{
		if (capacity_ >= n) {
			return;
		}

		size_t old_capacity = capacity_;
		char * old_storage = storage_;

		for (;;) 
		{
			capacity_ *= 2;
			if (capacity_ >= n)
				break;
		}

		storage_ = new char[capacity_];

		if (old_capacity)
		{
			memcpy(storage_, old_storage, length_);

			// zero memory in a way that an optimizer won't optimize it out
			old_storage[0] = 0;
			for (size_t i = 1; i < old_capacity; i++)
			{
				old_storage[i] = old_storage[i - 1];
			}
			delete[] old_storage;
		}
	}  

	~QyFormatCryptString()
	{
		if (capacity_)
		{
			storage_[0] = 0;
			for (size_t i = 1; i < capacity_; i++)
			{
				storage_[i] = storage_[i - 1];
			}
		}
		delete[] storage_;
	}
private:
	char * storage_;
	size_t capacity_;
	size_t length_;
};

class QyInsecureCryptStringImpl : public QyCryptStringImpl 
{
public:
	virtual ~QyInsecureCryptStringImpl() {}

public:
	std::string& password() { return m_password; }
	const std::string& password() const { return m_password; }

	virtual size_t length() const { return m_password.size(); }
	virtual void copyTo(char * dest, bool nullterminate) const
	{
		memcpy(dest, m_password.data(), m_password.size());
		if (nullterminate) dest[m_password.size()] = 0;
	}
	virtual std::string urlEncode() const { return m_password; }
	virtual QyCryptStringImpl * copy() const
	{
		QyInsecureCryptStringImpl * copy = new QyInsecureCryptStringImpl;
		copy->password() = m_password;
		return copy;
	}
private:
	std::string m_password;
};

END_NAMESPACE(qy)

#endif  // __QY_CRYPTSTRING_H__
