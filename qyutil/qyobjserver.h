#ifndef __QY_OBJSERVER_H__
#define __QY_OBJSERVER_H__

#include "qyutil/qyconfig.h"
#include "qyutil/qyobjserver_i.h"

DEFINE_NAMESPACE(qy)

const int kSubDestroy = -1;

class QYUTIL_API QyObjserver : public IQyObjserver
{
public:
	QyObjserver(IQySubject* pSubject);
	~QyObjserver();

    /** @param fupdate = kSubDestroy */
	void update(int fupdate , void* param0 , void* param1);
protected:
	IQySubject* m_subject;
};

class QySubjectPrivate;
/** 
*
* @author loach
*
* @date 2009-09-27
*/
class QYUTIL_API QySubject : public IQySubject
{
public:
	QySubject();
	~QySubject();
public:
	void attach(IQyObjserver* pObjserver);
	void detach(IQyObjserver* pObjserver);
	void notifyObjserver(int fupdate , void* param0 , void* param1);
private:
	friend class QySubjectPrivate;
	QySubjectPrivate* d_ptr_;
};

END_NAMESPACE(qy)

#endif //__QY_OBJSERVER_H__