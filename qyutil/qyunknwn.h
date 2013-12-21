#ifndef  __QY_UNKNOWN_H__
#define  __QY_UNKNOWN_H__

#include "qyutil/qyconfig.h"


#ifndef QY_DEFINE_GUID

#define QY_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	const QY_GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

#define EXTERN_GUID_QY(name) extern const QY_GUID name;

#endif // QY_DEFINE_GUID

#if defined(WIN32) || defined(WINCE)

#include <windows.h>
#include <unknwn.h>

#define QY_REFID  REFIID
#define QY_GUID   GUID
typedef IUnknown  IQyUnknown;

#define COMPARE_REFIID(left,right) left == right 

#else //!WIN32

typedef struct
{
	unsigned long  Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char  Data4[ 8 ];
}QY_GUID;

typedef const QY_GUID& QY_REFID;
typedef QY_CLSID       QY_GUID;

BOOL compare_ref_id(QY_REFID, QY_REFID );
#define COMPARE_REFIID(left,right) compare_ref_id(left,right)

BOOL compare_ref_id(QY_REFID left, QY_REFID right)
{
	BOOL bEque = ( 
	  (left.Data1 == right.Data1) &&
	  (left.Data2 == right.Data2) &&
	  (left.Data3 == right.Data3)
	  ) ;

	if( bEque )
	{
		for( int i = 0; i < 8 ; i ++ )
		{
			if( left.Data4[i]  != right.Data4[i] ) return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

/** 基类接口 */
struct  /*__declspec(novtable)*/ IQyUnknown
{
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(QY_REFID riid,void ** ppvObject) = 0;

	virtual ULONG STDMETHODCALLTYPE AddRef( void) = 0;

	virtual ULONG STDMETHODCALLTYPE Release( void) = 0;
};

// {C47EE582-7E56-4f0a-9BFF-772A1210ABF6}
QY_DEFINE_GUID(IID_IUnknown,
			   0xc47ee582, 0x7e56, 0x4f0a, 0x9b, 0xff, 0x77, 0x2a, 0x12, 0x10, 0xab, 0xf6);

#endif // WIN32


DEFINE_NAMESPACE(qy)

/** 简单智能接口，类似于 CComPtr 
 *
 * @author loach 
 *
 * @date 2009-07-12
*/
template <typename T>
class QyComPtr
{
public:
	QyComPtr(){
		unk_ = NULL;
	}
	QyComPtr(T* p){
		unk_ = p;
		qyAddRef(unk_);
	}
	~QyComPtr(){
		qyRelease(unk_);
	}
	operator T*() const{
		return unk_;
	}
	T& operator*() const
	{	
		return *unk_;
	}
	T* operator ->() const{
		return unk_;
	}

	T** operator&()
	{
		return &unk_;
	}

	bool operator!() const
	{
		return (unk_ == NULL);
	}

	bool operator != (T* pT) const
	{
		return !operator==(pT);
	}

	bool operator == (T* pT) const
	{
		return unk_ == pT;
	}

	void AddRef()
	{
		if (unk_)
		{
			unk_->AddRef();
		}
	}
	// Release the interface and set to NULL
	void Release()
	{
		T* u = unk_;
		if (u)
		{
			unk_ = NULL;
			u->Release();
		}
	}
	// attach to an existing interface (does not AddRef)
	void attach(T* p2)
	{
		T* o = unk_;
		unk_ = p2;
		qyRelease(o);
	}
	// detach the interface (does not Release)
	T* detach()
	{
		T* pt = unk_;
		unk_ = NULL;
		return pt;
	}
	T* unk_;
};

struct IQyNonDelegatingUnknown
{
	virtual HRESULT STDMETHODCALLTYPE  NonDelegatingQueryInterface( QY_REFID riid,void** ppvObject) = 0;
	virtual ULONG   STDMETHODCALLTYPE  NonDelegatingAddRef( void ) = 0;
	virtual ULONG  STDMETHODCALLTYPE   NonDelegatingRelease( void ) = 0;
};

/**
* @ingroup loa
*
* @author loach
*
* @data 2009-05-11
* 
* @brief 不知明的接口类
*/
class QYNovtableM QyUnknown : public IQyNonDelegatingUnknown
{
private:
	/* Owner of this object */
	IQyUnknown* unk_;

protected:
	/// 引用计数
	volatile long ref_;

public:
	/// Constructor
	QyUnknown (IQyUnknown* pUnk)
	{
		ref_ = 0;
		unk_ = ( pUnk != 0 ? pUnk : reinterpret_cast<IQyUnknown*>( static_cast<IQyNonDelegatingUnknown*>(this) ) );
	}
	/// Destructor
	virtual ~QyUnknown (){};
public:
	/// 获取自己的
	IQyUnknown* GetOwner(void)
	{
		return unk_;
	}

	virtual HRESULT STDMETHODCALLTYPE NonDelegatingQueryInterface( QY_REFID riid,void** ppvObject)
	{
		if  (!ppvObject)
		{
			return E_POINTER;
		}
		if (COMPARE_REFIID(riid ,IID_IUnknown))
		{
			IQyUnknown* pUnk = (IQyUnknown*)( (IQyNonDelegatingUnknown*)this);
			pUnk->AddRef();
			*ppvObject = pUnk;
			return S_OK;
		}
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	virtual ULONG STDMETHODCALLTYPE  NonDelegatingAddRef( void )
	{
#ifdef WIN32
		InterlockedIncrement(&ref_);
#else
		ref_ ++;
#endif
		return ref_;
	}

	virtual ULONG STDMETHODCALLTYPE  NonDelegatingRelease(void)
	{
#ifdef WIN32
		if (0 == InterlockedDecrement(&ref_))
		{
			delete this;
			return( 0 );
		}
#else
		ref_ --;
		if (ref_ <= 0)
		{
			delete this;
			return 0;
		}
#endif
		return ref_;
	}
};

//声明 IQyUnknown 接口函数
#define QY_DECLARE_IUNKNOWN() \
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(QY_REFID riid, void **ppv) { \
	return GetOwner()->QueryInterface(riid,ppv);}\
	virtual ULONG   STDMETHODCALLTYPE AddRef() {\
	return GetOwner()->AddRef();}\
	virtual ULONG   STDMETHODCALLTYPE Release() {\
	return GetOwner()->Release();}

END_NAMESPACE(qy)

#endif /* __QY_UNKNOWN_H__ */
