#ifndef __QY_PROPERTY_H__
#define __QY_PROPERTY_H__

#include "qyutil/qyconfig.h"

// namespace qyUtil
DEFINE_NAMESPACE(qy)

/** 属性值
*
* @author loach
*
* @date 2009-06-21
*/
template< typename PROPERTY_KEY>
struct QYNovtableM IQYPropertyItem
{
	/** 添加一项(如果存在,就进行修改) */
	virtual int add(const PROPERTY_KEY& key,const char* value,size_t len) =0;

	/** 删除特定属性值 */
	virtual int remove(const PROPERTY_KEY& key) =0;

	/** 清空所有属性值 */
	virtual void clear(void) = 0;

	/** 获取相应的值 */
	virtual const char* get(const PROPERTY_KEY& key) const =0;

	/** 获取相应的值 */
	virtual const char* get(const PROPERTY_KEY& key , size_t& len) const = 0;

	/** 获取子属性值 */
	virtual IQYPropertyItem<PROPERTY_KEY>* getChild(void) const = 0;
	virtual bool isChild(void) const = 0;
	virtual bool newChild(void) = 0;	
};

END_NAMESPACE(qy)

#endif //