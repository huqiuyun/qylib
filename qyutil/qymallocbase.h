#ifndef __QY_MALLOC_BASE_H__
#define __QY_MALLOC_BASE_H__

#include "qyutil/qyconfig.h"
// The definition of this class is located in Malloc.cpp.

DEFINE_NAMESPACE(qy)

/**
 * @class QyAllocator
 *
 * @brief Interface for a dynamic memory allocator that uses inheritance
 * and dynamic binding to provide extensible mechanisms for
 * allocating and deallocating memory.
 */
class QYUTIL_API QyAllocator
{
public:

  /// Unsigned integer type used for specifying memory block lengths.
  typedef size_t size_type;

  // = Memory Management

  /// Get pointer to a default QyAllocator.
  static QyAllocator *instance (void);

  /// "No-op" constructor (needed to make certain compilers happy).
  QyAllocator (void);

  /// Virtual destructor
  virtual ~QyAllocator (void);

  /// Allocate @a nbytes, but don't give them any initial value.
  virtual void *malloc (size_type nbytes) = 0;

  /// Allocate @a nbytes, giving them @a initial_value.
  virtual void *calloc (size_type nbytes, char initial_value = '\0') = 0;

  /// Allocate <n_elem> each of size @a elesize_, giving them
  /// @a initial_value.
  virtual void *calloc (size_type n_elem, size_type elesize_, char initial_value = '\0') = 0;

  /// Free <ptr> (must have been allocated by <QyAllocator::malloc>).
  virtual void free (void *ptr) = 0;
};

END_NAMESPACE(qy)

#define qyNEW_RETURN(DEST , SRC , RET)\
	do{\
	DEST = new SRC;\
	if (!DEST) return RET;\
	}while(0)

#define qyALLOCATOR(DEST , SRC)\
	do{\
	DEST = SRC;\
	if (!DEST) return;\
	}while(0)


#define qyALLOCATOR_RETURN(DEST , SRC,RET)\
	do{\
	DEST = SRC;\
	if (!DEST) return RET;\
	}while(0)

#endif /* __QY_MALLOC_BASE_H__ */
