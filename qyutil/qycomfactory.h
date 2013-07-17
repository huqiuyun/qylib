#ifndef __QY_COM_FACTORY_H__
#define __QY_COM_FACTORY_H__ 

#include "qyutil/qyconfig.h"
#include "qyutil/qyunknwn.h"
#include "qyutil/qycomfactory_i.h"

DEFINE_NAMESPACE(qy)

class QyComFactoryPrivate;
class QYUTIL_API QyComFactory : public IQyComFactory
{
public:
	QyComFactory();
	virtual ~QyComFactory();

public:

	HRESULT coInitialize();
	HRESULT coUninitialize();

	HRESULT coRegister(QYREFCLSID rclsid , const QyComRegister* com);
	HRESULT coUnRegister(QYREFCLSID rclsid , const QyComRegister* com);
	HRESULT createInstance(QYREFCLSID rclsid , IQyUnknown  * pUnkOuter , QYREFIID riid , LPVOID* ppv);
private:
	friend class QyComFactoryPrivate;
	QyComFactoryPrivate*  d_ptr_;
};

/**
 * exmaple:
 *    class QyExample : public QyUnknown , public IQyUnknown  
 *    {
     public:
	     QY_DECLARE_IUNKNOWN()

 *       QYCOM_DECLARE_CLASS_STATIC_CREATE
 *    };
 *
 *    QyComRegister exmaple = { "", QYIID_exmaple , &QyExample::create };
 *   
 *    
 *    QyComFactory* factory = NULL;
 *    factory->coRegister( QYIID_exmaple , &exmaple);
 * 
 *    
*/

END_NAMESPACE(qy)

#endif /* __QY_COM_FACTORY_H__ */