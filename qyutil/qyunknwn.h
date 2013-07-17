#ifndef  __QY_UNKNOWN_H__
#define  __QY_UNKNOWN_H__

#include "qyutil/qyconfig.h"
#include <assert.h>

/** 定义 */
#ifndef QY_DEFINE_GUID

#define QY_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	const QYGUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

#define EXTERN_GUID_QY(name) extern const QYGUID name;

#endif // QY_DEFINE_GUID

// uuid 值
#if defined(WIN32) || defined(WINCE)

#include <windows.h>
#include <unknwn.h>

#define QYREFIID       QYREFIID
#define GUID           QYGUID
#define IID            QYIID
#define REFCLSID       QYREFCLSID
#define CLSID          QYCLSID
typedef IUnknown       IQyUnknown;
#define COMPARE_REFIID IsEqualCLSID

#else

typedef struct
{
	unsigned long  Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char  Data4[ 8 ];
}QYGUID,QYIID,QYCLSID;

typedef const QYGUID& QYREFIID;
typedef const QYGUID& QYREFCLSID;

BOOL compare_ref_id(const QYREFIID &left,const QYREFIID&);
#define COMPARE_REFIID(left,right) compare_ref_id(left,right)

BOOL compare_ref_id(const QYREFIID &left,const QYREFIID &right)
{
	BOOL bEque =(
	  (left.Data1 == right.Data1) &&
	  (left.Data2 == right.Data2) &&
	  (left.Data3 == right.Data3)
	 ) ;

	if (bEque)
	{
		for( int i = 0; i < 8 ; i ++)
		{
			if (left.Data4[i]  != right.Data4[i]) return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}


/** 基类接口 */
struct  /*__declspec(novtable)*/ IQyUnknown
{
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(QYREFIID riid,void ** ppvObject) = 0;

	virtual ULONG STDMETHODCALLTYPE AddRef( void) = 0;

	virtual ULONG STDMETHODCALLTYPE Release( void) = 0;
};

// {C47EE582-7E56-4f0a-9BFF-772A1210ABF6}
QY_DEFINE_GUID(QYIID_IUnknown,
			   0xc47ee582, 0x7e56, 0x4f0a, 0x9b, 0xff, 0x77, 0x2a, 0x12, 0x10, 0xab, 0xf6);

#endif /* !WIN32 */


/** 接口方式 */
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
		unknown_ = NULL;
	}
	QyComPtr(T* p){
		unknown_ = p;
		qyAddRef(unknown_);
	}
	~QyComPtr(){
		qyRelease(unknown_);
	}
	operator T*() const{
		return unknown_;
	}
	T& operator*() const
	{	
		return *unknown_;
	}
	T* operator ->() const{
		return unknown_;
	}

	T** operator&()
	{
		return &unknown_;
	}

	bool operator!() const
	{
		return (unknown_ == NULL);
	}

	bool operator != (T* pT) const
	{
		return !operator==(pT);
	}

	bool operator == (T* pT) const
	{
		return unknown_ == pT;
	}

	void AddRef()
	{
		if (unknown_)
		{
			unknown_->AddRef();
		}
	}
	// Release the interface and set to NULL
	void Release()
	{
		T* pTemp = unknown_;
		if (pTemp)
		{
			unknown_ = NULL;
			pTemp->Release();
		}
	}
	// attach to an existing interface (does not AddRef)
	void attach(T* p2)
	{
		T* o = unknown_;
		unknown_ = p2;
		qyRelease(o);
	}
	// detach the interface (does not Release)
	T* detach()
	{
		T* pt = unknown_;
		unknown_ = NULL;
		return pt;
	}
	T* unknown_;
};

struct IQyNonDelegatingUnknown
{
	virtual HRESULT STDMETHODCALLTYPE  NonDelegatingQueryInterface( QYREFIID riid,void** ppvObject) = 0;
	virtual ULONG   STDMETHODCALLTYPE  NonDelegatingAddRef( void) = 0;
	virtual ULONG  STDMETHODCALLTYPE   NonDelegatingRelease( void) = 0;
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
	IQyUnknown* m_pUnk;

protected:
	/// 引用计数
	volatile long ref_;

public:
	/// Constructor
	QyUnknown (IQyUnknown* pUnk)
	{
		ref_ = 0;
		m_pUnk =(pUnk != 0 ? pUnk : reinterpret_cast<IQyUnknown*>( static_cast<IQyNonDelegatingUnknown*>(this)));
	}
	/// Destructor
	virtual ~QyUnknown (){};
public:
	/// 获取自己的
	IQyUnknown* GetOwner(void)
	{
		return m_pUnk;
	}

	virtual HRESULT STDMETHODCALLTYPE NonDelegatingQueryInterface( QYREFIID riid,void** ppvObject)
	{
		if(!ppvObject)
		{
			return E_POINTER;
		}
		if(COMPARE_REFIID(riid ,QYIID_IUnknown))
		{
			IQyUnknown* pUnk = (IQyUnknown*)( (IQyNonDelegatingUnknown*)this);
			pUnk->AddRef();
			*ppvObject = pUnk;
			return S_OK;
		}
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	virtual ULONG STDMETHODCALLTYPE  NonDelegatingAddRef( void)
	{
#ifdef WIN32
		InterlockedIncrement( &ref_);
#else
//#error Please code
		ref_ ++;
#endif
		return ref_;
	}

	virtual ULONG STDMETHODCALLTYPE  NonDelegatingRelease(void)
	{
#ifdef WIN32
		assert(ref_>0);
		if (0 == InterlockedDecrement( &ref_)){
			delete this;
			return( 0);
		}
#else
//#error Please code
		assert(ref_>0);
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

#ifndef QY_USE_DECLARE_UNKNOWN
#define QY_USE_DECLARE_UNKNOWN

//声明 IQyUnknown 接口函数
#define QY_DECLARE_IUNKNOWN() \
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(QYREFIID riid, void **ppv) { \
	return GetOwner()->QueryInterface(riid,ppv);}\
	virtual ULONG   STDMETHODCALLTYPE AddRef() {\
	return GetOwner()->AddRef();}\
	virtual ULONG   STDMETHODCALLTYPE Release() {\
	return GetOwner()->Release();}

#endif // QY_USE_DECLARE_UNKNOWN

// 静态创建
#define	QYCOM_DECLARE_CLASS_STATIC_CREATE                                       \
	static QyUnknown* STDCALL create(IQyUnknown  * pUnkOuter,HRESULT *phr);

#define QYCOdef_INE_CLASS_STATIC_CREATE(CLASS)                                 \
	QyUnknown* _stdcall CLASS::create(IQyUnknown  * pUnkOuter,HRESULT *phr) { \
	CLASS* p = new CLASS(pUnkOuter);                                          \
	if(!p) {                                                               \
	*phr = E_OUTOFMEMORY;                                                     \
	return NULL;                                                              \
	}                                                                         \
	*phr = S_OK;                                                              \
	return p;}

END_NAMESPACE(qy)

#endif /* __QY_UNKNOWN_H__ */
