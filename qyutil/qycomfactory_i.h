#ifndef __QY_IQYCOMFACTORY_H__ 
#define __QY_IQYCOMFACTORY_H__ 

#include "qyutil/qyconfig.h"
#include "qyutil/qyunknwn.h"

DEFINE_NAMESPACE(qy)

/** 实例创建回调函数 */
typedef QyUnknown* (STDCALL *QyComCreateInstanceCB)(IQyUnknown* pUnkOuter, HRESULT *phr);

/**
*
* @author loach
*
* @date 2007-07-24
*
* @brief 注册工厂类，为了向系统注册
*/
struct QyComRegister
{
	const char*      name_;
	const QY_GUID*     iid_;
	QyComCreateInstanceCB  fnCreate_;
};


/** 类似于COM生成工厂 */
struct QYNovtableM IQyComFactory
{
	// 注册工厂类
	virtual HRESULT coRegister(QY_REFID rclsid , const QyComRegister* com) = 0;
	virtual HRESULT coUnRegister(QY_REFID rclsid , const QyComRegister* com) = 0;

    /**
     * @brief 创建实例
     *
     * @param rclsid 请参考
     *
     * @param pUnkOuter 父接口
     * @param riid 请参考 vguid.h guid 的定义
     * @param [out]ppv 返回创建的实例对象
    */
	virtual HRESULT createInstance(QY_REFID rclsid , IQyUnknown  * pUnkOuter , QY_REFID riid , LPVOID* ppv) = 0;
};

END_NAMESPACE(qy)

#endif /* __QY_IQYCOMFACTORY_H__ */