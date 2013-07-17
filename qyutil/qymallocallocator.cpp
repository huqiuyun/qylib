#include "qymallocallocator.h"
#include "qyosstring.h"

DEFINE_NAMESPACE(qy)

static QyNewAllocator SingletonAllocator;

QyAllocator * QyAllocator::instance (void)
{
	return static_cast<QyAllocator*>(&SingletonAllocator);
}

QyAllocator::~QyAllocator (void)
{
}

QyAllocator::QyAllocator (void)
{
}

void * QyNewAllocator::malloc (size_t nbytes)
{
  char *ptr = 0;
  if (nbytes > 0)
  {
    qyNEW_RETURN(ptr,char[nbytes],0);
  }
  return (void *) ptr;
}

void * QyNewAllocator::calloc (size_t nbytes, char initial_value)
{
  char *ptr = 0;

  qyNEW_RETURN(ptr,char[nbytes],0);

  qyos_memset (ptr, initial_value, nbytes);
  return (void *) ptr;
}

void * QyNewAllocator::calloc (size_t n_elem, size_t elesize_, char initial_value)
{
  return QyNewAllocator::calloc (n_elem * elesize_, initial_value);
}

void QyNewAllocator::free (void *ptr)
{
  delete [] (char *) ptr;
}

END_NAMESPACE(qy)