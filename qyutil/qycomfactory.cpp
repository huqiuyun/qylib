#include "qycomfactory.h"
#include <list>

DEFINE_NAMESPACE(qy)

/// 工厂类集合
typedef std::list<const QyComRegister*> FactoryList;

/**
*
* @author loach
*
* @date 2007-07-24
*
* @brief 工厂类的集合类
*/
class QyRegisters : public FactoryList
{
public:
	const QYCLSID* rclsid_;
public:
	QyRegisters(QYREFCLSID rclsid)
	{
		rclsid_ = &rclsid;
	}
	/// Destructor
	~QyRegisters()
	{
		this->clear();
	}
    
	const QyComRegister* findFatory(const QYIID & clsid)
	{
		FactoryList::iterator it = begin();
		while (it != end())
		{
			const QyComRegister *p = *it;
			if(( COMPARE_REFIID( *(p->iid_) ,clsid)))
			{
				return p;
			}
			++it;
		}
		return NULL;
	}
    
	bool deleteFactory(const QYIID & clsid)
	{
		FactoryList::iterator it = begin();
		while(it != end()) 
		{
			const QyComRegister *p = *it;
			if(( COMPARE_REFIID(*(p->iid_) ,clsid)))
			{
				this->erase(it);
				return true;
			}
			++it;
		}
		return false;
	}
};

typedef std::list<QyRegisters*> QyRegisterList;
class QyComFactoryPrivate : public QyRegisterList
{
public:
	/// Constructor
	~QyComFactoryPrivate()
	{	
		init_ = false;
	}
	bool isInit(void) const { return init_; }


	QyUnknown *createInstance(const QyComRegister* com,IQyUnknown  * pUnk, HRESULT *phr) const 
	{
		if (!phr)
		{
			return NULL;
		}
		return com->fnCreate_?com->fnCreate_(pUnk, phr):NULL;
	}
private:
	friend class QyComFactory;
	bool   init_;
};

QyComFactory::QyComFactory()
:d_ptr_(new QyComFactoryPrivate())
{
}

QyComFactory::~QyComFactory()
{
	coUninitialize();
	qyDeleteM(d_ptr_);
}

HRESULT QyComFactory::coInitialize()
{
	QY_D(QyComFactory);
	if (!d->init_)
	{
		d->init_ = true;
	}
	return 0;
}

HRESULT QyComFactory::coUninitialize()
{
	QY_D(QyComFactory);
	if (!d->isInit())
	{
		return 0;
	}
	QyRegisterList::iterator it = d->begin();
	while (it != d->end()) 
	{
		delete (*it);
		++it;
	}
	d->clear();
	d->init_ = false;
	return 0;
}

HRESULT QyComFactory::coRegister(QYREFCLSID rclsid, const QyComRegister* com)
{
	QY_D(QyComFactory);
	if (!d->isInit())
	{
		return E_FAIL;
	}
	try
	{
		QyRegisters* fl = 0;
		QyRegisterList::iterator it = d->begin();
		while (it != d->end())
		{
			fl = (*it);
			if (COMPARE_REFIID(*(fl->rclsid_) ,rclsid))
			{
				fl ->push_back(com);
				return S_OK;
			}
			++it;
		}
		fl = new QyRegisters(rclsid);
		if (!fl)
		{
			return E_OUTOFMEMORY;
		}
		try{
			d->push_back(fl);
			fl->push_back(com);
		}
		catch(...)
		{
			delete fl;
			return E_FAIL;
		}
		return S_OK;

	}
	catch(...)
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT QyComFactory::coUnRegister(QYREFCLSID rclsid   , const QyComRegister* com)
{
	QY_D(QyComFactory);
	if (!d->isInit())
	{
		return E_FAIL;
	}
	try
	{
		QyRegisters* fl = 0;
		QyRegisterList::iterator it = d->begin();
		while (it != d->end())
		{
			fl = (*it);
			if (COMPARE_REFIID(*(fl->rclsid_) ,rclsid))
			{
				return fl->deleteFactory(*com->iid_);
			}
			++it;
		}
	}
	catch(...)
	{
		return E_FAIL;
	}
	return S_OK;
}

HRESULT QyComFactory::createInstance(QYREFCLSID rclsid,
                                     IQyUnknown* pUnkOuter , QYREFIID riid , LPVOID* ppv)
{
	QY_D(QyComFactory);
	HRESULT hr = E_FAIL;
	if (!d->isInit())
	{
		return hr;
	}
    
    QyRegisters* fl = 0;
    QyRegisterList::iterator it = d->begin();
    while (it != d->end())
    {
        fl = (*it);
        if (COMPARE_REFIID(*( fl->rclsid_) ,rclsid))
        {
            const QyComRegister * com = fl->findFatory(riid);
            if (!com)
            {
                *ppv = NULL;
                return E_FAIL;
            }
            QyUnknown * pObj = d->createInstance(com,pUnkOuter,&hr);
            if (!pObj)
            {
                return E_OUTOFMEMORY;
            }
            if (FAILED(hr))
            {
                delete pObj;
                return hr;
            }
            pObj->NonDelegatingAddRef();
            hr = pObj->NonDelegatingQueryInterface(riid, ppv);
            pObj->NonDelegatingRelease();
            break;
        }
        ++it;
    }
    return hr;
}

END_NAMESPACE(qy)