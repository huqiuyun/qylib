#ifndef __QY_ATTRIBUTE_H__
#define __QY_ATTRIBUTE_H__

/*
*
* 此库可以由读者用于学习之用,不能用于商业用途
* 如果用在商业用途,一块法律追究自己承担!
*
* 作者： loach.h(胡秋云)
*
* 时间： 2009-06-08
*/

#include "qyutil/qyunknwn.h"
#include "qyutil/qyproperty.h"

// namespace qyUtil
DEFINE_NAMESPACE(qy)

// IQyAttribute
// {3208B374-8A36-4aac-9744-AE185BD09F30}
QY_DEFINE_GUID(QYIID_IQyAttribute, 
			   0x3208b374, 0x8a36, 0x4aac, 0x97, 0x44, 0xae, 0x18, 0x5b, 0xd0, 0x9f, 0x30);

/**
* @brief 属性值接口
*
* @author loach
*
* @group vloachunit
*/
struct QYNovtableM IQyAttribute : public IQyUnknown
{
	typedef void* AttrValue;
	typedef void* AttrParam;
	/** 
	* @brief 添加属性值 
	* @return 正常(失败 -1 ,成功 0)
	*/
	virtual int  add(unsigned int nKey ,AttrValue attr,AttrParam param = 0) = 0 ;
	/**
	* @brief 查询属性值 
	*/
	virtual AttrValue get(unsigned int nKey , AttrParam param = 0) = 0;
	/**
	* @brief Delete
	*/
	virtual bool remove(unsigned int nKey) = 0;
};

typedef IQyAttribute* POSITION_attr;


END_NAMESPACE(qy)

#endif /* __QY_ATTRIBUTE_H__ */