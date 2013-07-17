
qyINLINE QyByteBuffer& QyByteBuffer::operator>>(char& val)
{
	read(reinterpret_cast<char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator>>(uint8& val)
{
	read(reinterpret_cast<char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator>>(short& val)
{
	read(reinterpret_cast<char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator>>(uint16& val)
{
	read(reinterpret_cast<char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator>>(int& val)
{
	read(reinterpret_cast<char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator>>(uint& val)
{
	read(reinterpret_cast<char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator>>(int64& val)
{
	read(reinterpret_cast<char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator>>(uint64& val)
{
	read(reinterpret_cast<char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator>>(float& val)
{
	read(reinterpret_cast<char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator>>(double& val)
{
	read(reinterpret_cast<char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator>>(std::string& val)
{
	readString(val);
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator>>(QyByteBuffer& val)
{
	val.write( data() , length());
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator<<(const char& val)
{
	write(reinterpret_cast<const char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator<<(const uint8& val)
{
	write(reinterpret_cast<const char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator<<(const short& val)
{
	write(reinterpret_cast<const char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator<<(const uint16& val)
{
	write(reinterpret_cast<const char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator<<(const int& val)
{
	write(reinterpret_cast<const char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator<<(const uint& val)
{
	write(reinterpret_cast<const char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator<<(const int64& val)
{
	write(reinterpret_cast<const char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator<<(const uint64& val)
{
	write(reinterpret_cast<const char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator<<(const float& val)
{
	write(reinterpret_cast<const char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator<<(const double& val)
{
	write(reinterpret_cast<const char*>(&val), sizeof(val));
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator<<(const char* val)
{
	if (val)
	{
		write(val, strlen(val));
	}
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator<<(const std::string& val)
{
	logString(val);
	return *this;
}

qyINLINE QyByteBuffer& QyByteBuffer::operator<<(const QyByteBuffer& val)
{
	write( val.data() , val.length());
	return *this;
}