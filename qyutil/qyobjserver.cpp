#include "qyobjserver.h"
#include "qylock.h"
#include <map>

DEFINE_NAMESPACE(qy)

QyObjserver::QyObjserver(IQySubject* pSubject)
:m_subject(pSubject)
{

}
QyObjserver::~QyObjserver()
{
	if (m_subject)
	{
		m_subject->detach( this);
		m_subject = NULL;
	}
}

void QyObjserver::update(int iUpdate , void* param0 , void* param1)
{
	if (kSubDestroy == iUpdate)
	{			
		m_subject = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////
typedef std::map<IQyObjserver* , IQyObjserver*> ObjserverMAP;

class QySubjectPrivate
{
public:
	QySubjectPrivate()
	{
	}

	~QySubjectPrivate()
	{
		QyAutoLock<QY_CS> cs(&m_cs);
		m_objs.clear();
	}	
	ObjserverMAP m_objs;
    QySmartLock<QY_CS> m_cs;	
};

QySubject::QySubject()
{
	d_ptr_ = new QySubjectPrivate();
}

QySubject::~QySubject()
{
	notifyObjserver(kSubDestroy,0,0);

	delete d_ptr_;
	d_ptr_ = 0;
}

void QySubject::attach(IQyObjserver* pObjserver)
{
	QY_D(QySubject);
	QyAutoLock<QY_CS> cs(& d->m_cs);
	ObjserverMAP::iterator it = d->m_objs.find( pObjserver);
	if (it != d->m_objs.end())
	{
		return ;
	}
	d->m_objs[ pObjserver ] = pObjserver;
}

void QySubject::detach(IQyObjserver* pObjserver)
{
	QY_D(QySubject);
	QyAutoLock<QY_CS> cs(& d->m_cs);
	ObjserverMAP::iterator it = d->m_objs.find( pObjserver);
	if (it != d->m_objs.end())
	{
		d->m_objs.erase( it);
	}
}

void QySubject::notifyObjserver(int iUpdate , void* param0 , void* param1)
{
	QY_D(QySubject);
	QyAutoLock<QY_CS> cs(& d->m_cs);
	ObjserverMAP::iterator it = d->m_objs.begin();
	while( it != d->m_objs.end())
	{
		it->second->update( iUpdate , param0 , param1);
		++it;
	}
}

END_NAMESPACE(qy)