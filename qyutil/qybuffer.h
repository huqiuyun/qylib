#ifndef __QY_BUFFER_H__
#define __QY_BUFFER_H__

#include "qyutil/qydefine.h"
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

DEFINE_NAMESPACE(qy)

/**
*
* @author loach

* @date 2009-04-26
*/
class QyBuffer
{
public:

#define qyutil_buffer_data_char(tj,c){\
	(tj).data = (char*)c;\
	(tj).len = strlen(c);}

#define qyutil_buffer_data_char_len(tj,c,size){\
	(tj).data = (char*)c;\
	(tj).len = size;}

#define qyutil_buffer_data_wchar(tj,c){\
	(tj).data = (char*)c;\
	(tj).len = wcslen(c)*sizeof(wchar_t);}

#define qyutil_buffer_data_wchar_len(tj,c,size){\
	(tj).data = (char*)c;\
	(tj).len = size*sizeof(wchar_t);}

#define qyutil_buffer_data_buff(tj,c,size){\
	(tj).data = (char*)c;\
	(tj).len = size;}

#define qyutil_buffer_for_char_copy(buffer_qy,buf) {\
	size_t l = strlen(buf);\
	(buffer_qy).str(buf,l);}
	
#define qyutil_buffer_for_char_copy_len(buffer_qy,buf,l) {\
	(buffer_qy).str(buf,l);}
	

#define qyutil_buffer_for_wchar_copy(buffer_qy,buf) {\
	size_t l = wcslen(buf);\
	(buffer_qy).str(buf,l*sizeof(wchar_t));}

#define qyutil_buffer_for_wchar_copy_len(buffer_qy,buf,size) {\
	(buffer_qy).str((const char*)buf,size*sizeof(wchar_t));}

#define qyutil_buffer_for_buff_copy(buffer_qy,buf,l) \
	(buffer_qy).str(buf,l);

	struct QyBufferData
	{
		char*  data;
		size_t len;
	};
	typedef void* qyPBUFFERDATA;

	QyBuffer()
	{
		data_.data = 0;
		data_.len = 0;
	}
	QyBuffer(const QyBuffer& d)
	{
		data_.data = 0;
		data_.len = 0;
		*this = d;
	}
	const QyBuffer& operator = (const QyBuffer& d)
	{
		str( d.data() ,d.length());
		return *this;
	}

	// read interface
	const char* c_str(void) const{
		if (data_.data)
		{
			return data_.data;
		}
		return "";
	}
	const wchar_t* wc_str(void){
		if (data_.data)
		{
			return (wchar_t*)data_.data;
		}
		return L"";
	}

	char* data(void) const
	{
		return data_.data;
	}
	size_t length() const
	{
		return data_.len;
	}

	wchar_t* wdata(void)
	{
		return (wchar_t*)data_.data;
	}

	size_t wlength() const
	{
		return data_.len/sizeof(wchar_t);
	}

	bool operator == (const QyBuffer& buff)
	{
		if (length() != buff.length())
		{
			return false;
		}
		return(memcmp( c_str(),buff.c_str(),length()) == 0) ? true : false ;
	}

	void operator =(const QyBufferData& f)
	{
		str(f.data , f.len);
	}

	void operator +=(const QyBufferData& f)
	{
		if (f.len <= 0)
		{
			return ;
		}
		internal_realloc(f.len);
		internal_append(f.data , f.len);
	}

	void operator +=(const QyBuffer& buff)
	{
		if (buff.length() <= 0)
		{
			return ;
		}
		internal_realloc( buff.length());
		internal_append( buff.data() , buff.length());
	}

	bool isequal(const char* buff , size_t l) const
	{
		if (length() != l || l <= 0 || 0 == buff)
		{
			return false;
		}
		return(memcmp( c_str() , buff , l) == 0) ? true : false ;
	}

	size_t rfind(const QyBufferData& f)
	{ 
		if (data_.data && data_.len > 0 && f.data && f.len <= data_.len)
		{
			const char* sz =  data_.data + data_.len - f.len ;
			const char* end = data_.data;
			while( sz > end)
			{
				if (_memicmp(sz , f.data , f.len) == 0)
				{
					return (size_t)(sz - data_.data);
				}
				sz --;
			}
		}
		return -1;
	}

	size_t find(const QyBufferData& f)
	{
		if (data_.data && data_.len > 0 && f.data && f.len <= data_.len)
		{
			const char* sz =  data_.data; 
			const char* end = data_.data + data_.len;
			while( (sz + f.len) <= end)
			{
				if (_memicmp(sz , f.data , f.len) == 0)
					return (size_t)(sz - data_.data);
				sz ++;
			}
		}
		return -1;
	}

	QyBuffer substr(size_t at ,size_t len = -1){
		QyBuffer buff;
		if (at < data_.len)
		{
			if (len == 0 || len == (size_t)-1)
			{
				len = (data_.len - at);
			}
			else if (at + len > data_.len)
			{
				len = data_.len - at;
			}
			buff.str((data_.data + at) , len) ;						
		}
		return buff;
	}

	// write interface
	void str(const char* buff , size_t l)
	{
		if ((l > data_.len) || !buff){
			internal_reset(buff,l);
		}
		if (l && buff){

			if (data_.len >= (l + 64)){
				
				internal_reset(buff,l);
			}else{
				data_.len = l;
				memcpy_s(data_.data ,data_.len , buff , l);			

				*(data_.data + data_.len  ) = '\0';
				*(data_.data + data_.len+1) = '\0';
			}
		}
	}
	// free the buffer
	void clear()
	{
		if (data_.data && data_.len)
		{
			free( data_.data);
			data_.data = 0;
			data_.len = 0;
		}
	}

public:
	~QyBuffer()
	{
		clear();
	}
private:
	void internal_realloc( size_t l)
	{
		if (data_.data && data_.len > 0)
		{
			data_.data = reinterpret_cast<char*>( realloc(data_.data  , (data_.len + l + 2)));
		}
	}

	void internal_append(const char* buff , size_t l)
	{
		if (!data_.data)
		{
			internal_reset( buff , l);
		}

		if (data_.data && data_.len)
		{
			memcpy_s( (data_.data + data_.len) , (data_.len + l) , buff , l);
			data_.len += l;
			*(data_.data + data_.len  ) = '\0';
			*(data_.data + data_.len+1) = '\0';
		}
	}

	void internal_reset(const char* buff , size_t l)
	{
		clear();
		if (buff && l)
		{
			data_.data = reinterpret_cast<char*>( malloc( l + 2));
			memset(data_.data,0,l + 2);
			data_.len = l;
		}
	}
private:
	QyBufferData data_;
};

END_NAMESPACE(qy)

#define qyutil_zeroQyBufferData(tj) { tj.data =0; tj.len = 0;}

#endif //__QY_Buffer_H__
