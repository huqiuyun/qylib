
#ifndef __QY_MALLOC_ALLOCATOR_H__
#define __QY_MALLOC_ALLOCATOR_H__

#include "qyutil/qymallocbase.h"

DEFINE_NAMESPACE(qy)
/**
* @class QyNewAllocator
*
* @brief Defines a class that provided a simple implementation of
* memory allocation.
*
* This class uses the new/delete operators to allocate and free up
* memory.  Please note that the only methods that are supported are
* <malloc>, <calloc>, and <free>. All other methods are no-ops that
* return -1 and set @c errno to <ENOTSUP>.  If you require this
* functionality, please use: CAllocator_qyUtilAdapter <qyMalloc
* <qyLOCAL_MEMORY_POOL, MUTEX> >, which will allow you to use the
* added functionality of bind/find/etc. while using the new/delete
* operators.
*/
class QYUTIL_API QyNewAllocator : public QyAllocator
{
public:
	/// These methods are defined.
	virtual void *malloc (size_t nbytes);
	virtual void *calloc (size_t nbytes, char initial_value = '\0');
	virtual void *calloc (size_t n_elem, size_t elesize_, char initial_value = '\0');
	virtual void free (void *ptr);
};

END_NAMESPACE(qy)

#endif /* __QY_MALLOC_ALLOCATOR_H__ */
