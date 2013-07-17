#pragma once

#include "qyutil/qyconfig.h"
/**
 *
 * @author loach
 *
 * @date 2009-09-27
 */
DEFINE_NAMESPACE(qy)

class QYUTIL_API IQyObjserver
{
public:
    virtual ~IQyObjserver(){}
    virtual void update(int iUpdate , void* param0 , void* param1) = 0;
};

class QYUTIL_API IQySubject
{
public:
    virtual ~IQySubject(){}
    virtual void attach(IQyObjserver* pObjserver) = 0;
    virtual void detach(IQyObjserver* pObjserver) = 0;
    virtual void notifyObjserver(int iUpdate , void* param0 , void* param1) = 0;
};

END_NAMESPACE(qy)
