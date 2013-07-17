#ifndef __QY_REFPTR_H__
#define __QY_REFPTR_H__

#include "qyutil/qydefine.h"

DEFINE_NAMESPACE(qy)

/** QyRefPtrT<> is a reference-counting shared smartpointer.
 *
 * Reference counting means that a shared reference count is incremented each
 * time a QyRefPtrT is copied, and decremented each time a QyRefPtrT is destroyed,
 * for instance when it leaves its scope. When the reference count reaches
 * zero, the contained object is deleted
 *
 * cairomm uses QyRefPtrT so that you don't need to remember
 * to delete the object explicitly, or know when a method expects you to delete 
 * the object that it returns, and to prevent any need to manually  reference 
 * and unreference() cairo objects.
 */
template <class T_CppObject>
class QyRefPtrT
{
public:
  /** Default constructor
   *
   * Afterwards it will be null and use of -> will cause a segmentation fault.
   */
  inline QyRefPtrT();
  
  /// Destructor - decrements reference count.
  inline ~QyRefPtrT();

  /** For use only in the internal implementation of cairomm, gtkmm, etc.
   *
   * This takes ownership of @a pCppObject, so it will be deleted when the 
   * last QyRefPtrT is deleted, for instance when it goes out of scope.
   *
   * This assumes that @a pCppObject already has a starting reference for its underlying cairo object,
   * so that destruction of @a @pCppObject will cause a corresponding unreference of its underlying 
   * cairo object. For instance, a cairo_*_create() function usually provides a starting reference, 
   * but a cairo_*_get_*() function requires the caller to manually reference the returned object.
   * In this case, you should call reference() on @a pCppObject before passing it to this constructor.
   */
  explicit inline QyRefPtrT(T_CppObject* pCppObject);

  ///  For use only in the internal implementation of sharedptr.
  explicit inline QyRefPtrT(T_CppObject* pCppObject, int* refcount);

  /** Copy constructor
   *
   * This increments the shared reference count.
   */
  inline QyRefPtrT(const QyRefPtrT<T_CppObject>& src);

  /** Copy constructor (from different, but castable type).
   *
   * Increments the reference count.
   */
  template <class T_CastFrom>
  inline QyRefPtrT(const QyRefPtrT<T_CastFrom>& src);

  /** Swap the contents of two QyRefPtrT<>.
   * This method swaps the internal pointers to T_CppObject.  This can be
   * done safely without involving a reference/unreference cycle and is
   * therefore highly efficient.
   */
  inline void swap(QyRefPtrT<T_CppObject>& other);

  /// Copy from another QyRefPtrT:
  inline QyRefPtrT<T_CppObject>& operator=(const QyRefPtrT<T_CppObject>& src);

  /** Copy from different, but castable type).
   *
   * Increments the reference count.
   */
  template <class T_CastFrom>
  inline QyRefPtrT<T_CppObject>& operator=(const QyRefPtrT<T_CastFrom>& src);

  /// Tests whether the QyRefPtrT<> point to the same underlying instance.
  inline bool operator==(const QyRefPtrT<T_CppObject>& src) const;
  
  /// See operator==().
  inline bool operator!=(const QyRefPtrT<T_CppObject>& src) const;

  /** Dereferencing.
   *
   * Use the methods of the underlying instance like so:
   * <code>refptr->memberfun()</code>.
   */
  inline T_CppObject* operator->() const;

  /** Test whether the QyRefPtrT<> points to any underlying instance.
   *
   * Mimics usage of ordinary pointers:
   * @code
   *   if (ptr)
   *     do_something();
   * @endcode
   */
  inline operator bool() const;

  /// Set underlying instance to 0, decrementing reference count of existing instance appropriately.
  inline void clear();


  /** Dynamic cast to derived class.
   *
   * The QyRefPtrT can't be cast with the usual notation so instead you can use
   * @code
   *   ptr_derived = QyRefPtrT<Derived>::cast_dynamic(ptr_base);
   * @endcode
   */
  template <class T_CastFrom>
  static inline QyRefPtrT<T_CppObject> cast_dynamic(const QyRefPtrT<T_CastFrom>& src);

  /** Static cast to derived class.
   *
   * Like the dynamic cast; the notation is 
   * @code
   *   ptr_derived = QyRefPtrT<Derived>::cast_static(ptr_base);
   * @endcode
   */
  template <class T_CastFrom>
  static inline QyRefPtrT<T_CppObject> cast_static(const QyRefPtrT<T_CastFrom>& src);

  /** Cast to non-const.
   *
   * The QyRefPtrT can't be cast with the usual notation so instead you can use
   * @code
   *   ptr_unconst = QyRefPtrT<UnConstType>::cast_const(ptr_const);
   * @endcode
   */
  template <class T_CastFrom>
  static inline QyRefPtrT<T_CppObject> cast_const(const QyRefPtrT<T_CastFrom>& src);


  // Warning: This is for internal use only.  Do not manually modify the
  // reference count with this pointer.
  inline int* refcount_() const { return ref_; }

private:
  void unref();

  T_CppObject* object_;
  mutable int* ref_;
};


// QyRefPtrT<>::operator->() comes first here since it's used by other methods.
// If it would come after them it wouldn't be inlined.

template <class T_CppObject> inline
T_CppObject* QyRefPtrT<T_CppObject>::operator->() const
{
  return object_;
}

template <class T_CppObject> inline
QyRefPtrT<T_CppObject>::QyRefPtrT()
:
  object_(0),
  ref_(0)
{}

template <class T_CppObject> inline
QyRefPtrT<T_CppObject>::~QyRefPtrT()
{
  unref();
}

template <class T_CppObject> inline
void QyRefPtrT<T_CppObject>::unref()
{
  if (ref_)
  {
    --(*ref_);

    if (*ref_ == 0)
    {
      if (object_)
      {
        delete object_;
        object_ = 0;
      }

      delete ref_;
      ref_ = 0;
    }
  }
}


template <class T_CppObject> inline
QyRefPtrT<T_CppObject>::QyRefPtrT(T_CppObject* pCppObject)
:
  object_(pCppObject),
  ref_(0)
{
  if (pCppObject)
  {
    ref_ = new int;
    *ref_ = 1; //This will be decremented in the destructor.
  }
}

//Used by cast_*() implementations:
template <class T_CppObject> inline
QyRefPtrT<T_CppObject>::QyRefPtrT(T_CppObject* pCppObject, int* refcount)
:
  object_(pCppObject),
  ref_(refcount)
{
  if (object_ && ref_)
    ++(*ref_);
}

template <class T_CppObject> inline
QyRefPtrT<T_CppObject>::QyRefPtrT(const QyRefPtrT<T_CppObject>& src)
:
  object_ (src.object_),
  ref_(src.ref_)
{
  if (object_ && ref_)
    ++(*ref_);
}

// The templated ctor allows copy construction from any object that's
// castable.  Thus, it does downcasts:
//   base_ref = derived_ref
template <class T_CppObject>
  template <class T_CastFrom>
inline
QyRefPtrT<T_CppObject>::QyRefPtrT(const QyRefPtrT<T_CastFrom>& src)
:
  // A different QyRefPtrT<> will not allow us access to object_.  We need
  // to add a get_underlying() for this, but that would encourage incorrect
  // use, so we use the less well-known operator->() accessor:
  object_ (src.operator->()),
  ref_(src.refcount_())
{
  if (object_ && ref_)
    ++(*ref_);
}

template <class T_CppObject> inline
void QyRefPtrT<T_CppObject>::swap(QyRefPtrT<T_CppObject>& other)
{
  T_CppObject *const temp = object_;
  int* temp_count = ref_; 

  object_ = other.object_;
  ref_ = other.ref_;

  other.object_ = temp;
  other.ref_ = temp_count;
}

template <class T_CppObject> inline
QyRefPtrT<T_CppObject>& QyRefPtrT<T_CppObject>::operator=(const QyRefPtrT<T_CppObject>& src)
{
  // In case you haven't seen the swap() technique to implement copy
  // assignment before, here's what it does:
  //
  // 1) Create a temporary QyRefPtrT<> instance via the copy ctor, thereby
  //    increasing the reference count of the source object.
  //
  // 2) Swap the internal object pointers of *this and the temporary
  //    QyRefPtrT<>.  After this step, *this already contains the new pointer,
  //    and the old pointer is now managed by temp.
  //
  // 3) The destructor of temp is executed, thereby unreferencing the
  //    old object pointer.
  //
  // This technique is described in Herb Sutter's "Exceptional C++", and
  // has a number of advantages over conventional approaches:
  //
  // - Code reuse by calling the copy ctor.
  // - Strong exception safety for free.
  // - Self assignment is handled implicitely.
  // - Simplicity.
  // - It just works and is hard to get wrong; i.e. you can use it without
  //   even thinking about it to implement copy assignment whereever the
  //   object data is managed indirectly via a pointer, which is very common.

  QyRefPtrT<T_CppObject> temp (src);
  this->swap(temp);
  return *this;
}

template <class T_CppObject>
  template <class T_CastFrom>
inline
QyRefPtrT<T_CppObject>& QyRefPtrT<T_CppObject>::operator=(const QyRefPtrT<T_CastFrom>& src)
{
  QyRefPtrT<T_CppObject> temp (src);
  this->swap(temp);
  return *this;
}

template <class T_CppObject> inline
bool QyRefPtrT<T_CppObject>::operator==(const QyRefPtrT<T_CppObject>& src) const
{
  return (object_ == src.object_);
}

template <class T_CppObject> inline
bool QyRefPtrT<T_CppObject>::operator!=(const QyRefPtrT<T_CppObject>& src) const
{
  return (object_ != src.object_);
}

template <class T_CppObject> inline
QyRefPtrT<T_CppObject>::operator bool() const
{
  return (object_ != 0);
}

template <class T_CppObject> inline
void QyRefPtrT<T_CppObject>::clear()
{
  QyRefPtrT<T_CppObject> temp; // swap with an empty QyRefPtrT<> to clear *this
  this->swap(temp);
}

template <class T_CppObject>
  template <class T_CastFrom>
inline
QyRefPtrT<T_CppObject> QyRefPtrT<T_CppObject>::cast_dynamic(const QyRefPtrT<T_CastFrom>& src)
{
  T_CppObject *const pCppObject = dynamic_cast<T_CppObject*>(src.operator->());

  if (pCppObject) //Check whether dynamic_cast<> succeeded so we don't pass a null object with a used refcount:
    return QyRefPtrT<T_CppObject>(pCppObject, src.refcount_());
  else
    return QyRefPtrT<T_CppObject>();
}

template <class T_CppObject>
  template <class T_CastFrom>
inline
QyRefPtrT<T_CppObject> QyRefPtrT<T_CppObject>::cast_static(const QyRefPtrT<T_CastFrom>& src)
{
  T_CppObject *const pCppObject = static_cast<T_CppObject*>(src.operator->());

  return QyRefPtrT<T_CppObject>(pCppObject, src.refcount_());
}

template <class T_CppObject>
  template <class T_CastFrom>
inline
QyRefPtrT<T_CppObject> QyRefPtrT<T_CppObject>::cast_const(const QyRefPtrT<T_CastFrom>& src)
{
  T_CppObject *const pCppObject = const_cast<T_CppObject*>(src.operator->());

  return QyRefPtrT<T_CppObject>(pCppObject, src.refcount_());
}

/** @relates Glib::QyRefPtrT */
template <class T_CppObject> inline
void swap(QyRefPtrT<T_CppObject>& lhs, QyRefPtrT<T_CppObject>& rhs)
{
  lhs.swap(rhs);
}

END_NAMESPACE(qy)

#endif /* __QY_REFPTR_H__ */
