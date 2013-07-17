/*

Copyright (c) 2004-2005, Nash Tsai
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the distribution.
* Neither the name of the author nor the names of its
contributors may be used to endorse or promote products derived
from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/
#define X_DELEGATE_TEMPLATE_PARAMS    X_MAKE_PARAMS1(X_DELEGATE_NUM_ARGS, class T)
// class T0, class T1, class T2, ...
#define X_DELEGATE_TEMPLATE_ARGS      X_MAKE_PARAMS1(X_DELEGATE_NUM_ARGS, T)
// T0, T1, T2, ...
#define X_DELEGATE_FUNCTION_PARAMS    X_MAKE_PARAMS2(X_DELEGATE_NUM_ARGS, T, a)
// T0 a0, T1 a1, T2 a2, ...
#define X_DELEGATE_FUNCTION_ARGS      X_MAKE_PARAMS1(X_DELEGATE_NUM_ARGS, a)
// a0, a1, a2, ...

// Comma if nonzero number of arguments
#if X_DELEGATE_NUM_ARGS == 0
#define X_DELEGATE_COMMA
#else
#define X_DELEGATE_COMMA    ,
#endif

//-------------------------------------------------------------------------
// class Delegate<R (T1, T2, ..., TN)>
//template <class R, X_MAKE_PARAMS1_1 (class T)> <class R, class T1, class T2,
//class Delegate<R ()> 
template <class R X_DELEGATE_COMMA X_DELEGATE_TEMPLATE_PARAMS, class MT_Policy>
class Delegate<R (X_DELEGATE_TEMPLATE_ARGS), MT_Policy>
{
   // Declaractions
private:
   class DelegateImplBase
   {
      // Fields
   public:
#ifndef USING_LIGHT_DELEGATE
      DelegateImplBase* Previous; // linked list
#endif
      // Constructor/Destructor
   protected:
      DelegateImplBase() 
#ifndef USING_LIGHT_DELEGATE
         : Previous(NULL) 
#endif
      { }
      
      DelegateImplBase(const DelegateImplBase& other) 
#ifndef USING_LIGHT_DELEGATE
         : Previous(NULL) 
#endif
      { }
   public:
      virtual ~DelegateImplBase() { }

      // Methods
   public:
      virtual DelegateImplBase* Qyne() const = 0;
      virtual R Invoke(X_DELEGATE_FUNCTION_PARAMS) const = 0;
   };

   template <class TFunctor>
   struct Invoker
   {
      static R Invoke(const TFunctor& f X_DELEGATE_COMMA X_DELEGATE_FUNCTION_PARAMS)
      {
         return (const_cast<TFunctor&>(f))(X_DELEGATE_FUNCTION_ARGS);
      }
   };

   template <class TPtr, class TFunctionPtr>
   struct Invoker<pair<TPtr, TFunctionPtr> >
   {
      static R Invoke(const pair<TPtr, TFunctionPtr>& mf X_DELEGATE_COMMA X_DELEGATE_FUNCTION_PARAMS)
      {
         return ((*mf.first).*mf.second)(X_DELEGATE_FUNCTION_ARGS);
      }
   };

   template <class TFunctor>
   class DelegateImpl : public DelegateImplBase
   {
      // Fields
   public:
      TFunctor Functor;

      // Constructor
   public:
      DelegateImpl(const TFunctor& f) : Functor(f)
      {
      }
      //DelegateImpl(const DelegateImpl& other) : Functor(other.Functor)
      //{
      //}

      // Methods
   public:
      virtual DelegateImplBase* Qyne() const
      {
         return new (nothrow) DelegateImpl(*this);
      }
      virtual R Invoke(X_DELEGATE_FUNCTION_PARAMS) const
      {
         return Invoker<TFunctor>::Invoke(this->Functor X_DELEGATE_COMMA X_DELEGATE_FUNCTION_ARGS);
      }
   };

   // Fields
private:
   DelegateImplBase* last_;
   mutable MT_Policy m_mutex;

   // Constructor/Destructor
public:
   Delegate()
   {
      this->last_ = NULL;
   }

   template <class TFunctor>
   Delegate(const TFunctor& f)
   {
      this->last_ = NULL;
      *this = f;
   }

   template<class TPtr, class TFunctionPtr>
   Delegate(const TPtr& obj, const TFunctionPtr& mfp)
   {
      this->last_ = NULL;
      *this = make_pair(obj, mfp);
   }

   Delegate(const Delegate& d)
   {
      this->last_ = NULL;
      *this = d;
   }

   ~Delegate()
   {
      Clear();
   }

   // Properties
public:
   bool IsEmpty() const
   {
      LockBlock<MT_Policy> lock(m_mutex);
      return (this->last_ == NULL);
   }
#ifndef USING_LIGHT_DELEGATE
   bool IsMulticast() const
   {
      LockBlock<MT_Policy> lock(m_mutex);

      return (this->last_ != NULL && this->last_->Previous != NULL);
   }
#endif

   // Static Methods
private:
   static DelegateImplBase* QyneDelegateList(DelegateImplBase* list, /*out*/ DelegateImplBase** first)
   {
      DelegateImplBase* list2 = list;
      DelegateImplBase* newList = NULL;
      DelegateImplBase** pNewList = &newList;
      DelegateImplBase* temp = NULL;

      try
      {
         while (list2 != NULL)
         {
            temp = list2->Qyne();
            if (temp)
            {
               *pNewList = temp;
#ifndef USING_LIGHT_DELEGATE
               pNewList = &temp->Previous;
               list2 = list2->Previous;
#else
               break;
#endif
            }
            else
            {
               break;
            }

         }
      }
      catch (...)
      {
         FreeDelegateList(newList);
#ifndef BOOST_NO_EXCEPTIONS
         throw;
#endif
      }

      if (first != NULL)
         *first = temp;
      return newList;
   }

   static void FreeDelegateList(DelegateImplBase* list)
   {
#ifndef USING_LIGHT_DELEGATE
      DelegateImplBase* temp = NULL;
      while (list != NULL)
      {
         temp = list->Previous;
         delete list;
         list = temp;
      }
#else
      delete list;
#endif
   }

#ifndef USING_LIGHT_DELEGATE
   static void InvokeDelegateList(DelegateImplBase* list X_DELEGATE_COMMA X_DELEGATE_FUNCTION_PARAMS)
   {
      if (list != NULL)
      {
         if (list->Previous != NULL)
            InvokeDelegateList(list->Previous X_DELEGATE_COMMA X_DELEGATE_FUNCTION_ARGS);
         list->Invoke(X_DELEGATE_FUNCTION_ARGS);
      }
   }
#endif

   // Methods
private:

   //template <class TPtr, class TFunctionPtr>
   //bool Exist(const TPtr& obj, const TFunctionPtr& mfp)
   //{
   //   DelegateImpl<pair<TPtr, TFunctionPtr> >* last = dynamic_cast<DelegateImpl<pair<TPtr, TFunctionPtr> >*>(this->last_);
   //   while (last)
   //   {
   //      if (last->Functor.first == obj && 
   //         last->Functor.second == mfp)
   //      {
   //         return true;
   //      }
   //      else
   //      {
   //         last = dynamic_cast<DelegateImpl<pair<TPtr, TFunctionPtr> >*>(last->Previous);
   //      }
   //   }
   //   return false;
   //}

   // Methods
public:

#ifndef USING_LIGHT_DELEGATE
   template <class TFunctor>
   void Add(const TFunctor& f)
   {
      if (!Exist(f))
      {
         LockBlock<MT_Policy> lock(m_mutex);

         DelegateImplBase* del = new (nothrow) DelegateImpl<TFunctor>(f);
         if (del)
         {
            del->Previous = this->last_;
            this->last_ = del;
         }
      }
   }

   template<class TPtr, class TFunctionPtr>
   void Add(const TPtr& obj, const TFunctionPtr& mfp)
   {
      Add(make_pair(obj, mfp));
   }

   template <class TFunctor>
   bool Remove(const TFunctor& f)
   {
      LockBlock<MT_Policy> lock(m_mutex);

      DelegateImplBase* pLast = this->last_;
      DelegateImplBase** ppLast = &this->last_;
      DelegateImpl<TFunctor>* impl = NULL;

      while (pLast != NULL)
      {
         impl = dynamic_cast<DelegateImpl<TFunctor>*>(pLast);
         if (impl && impl->Functor == f)
         {
            *ppLast = pLast->Previous;
            delete impl;
            return true;
         }
         ppLast = &pLast->Previous;
         pLast = pLast->Previous;
      }
      return false;
   }

   template<class TPtr, class TFunctionPtr>
   bool Remove(const TPtr& obj, const TFunctionPtr& mfp)
   {
      return Remove(make_pair(obj, mfp));
   }
#endif

   void Clear()
   {
      LockBlock<MT_Policy> lock(m_mutex);

      FreeDelegateList(this->last_);
      this->last_ = NULL;
   }

   template <class TFunctor>
   bool Exist(const TFunctor& f) const
   {
      LockBlock<MT_Policy> lock(m_mutex);

      DelegateImpl<TFunctor>* last = dynamic_cast<DelegateImpl<TFunctor>*>(this->last_);
      while (last)
      {
         if (last->Functor == f)
         {
            return true;
         }
#ifndef USING_LIGHT_DELEGATE
         else
         {
            last = dynamic_cast<DelegateImpl<TFunctor>*>(last->Previous);
         }
#else
         break;
#endif
      }
      return false;
   }
private:
   template <class TFunctor>
   bool Equals(const TFunctor& f) const
   {
      if (this->last_ == NULL 
#ifndef USING_LIGHT_DELEGATE
         || this->last_->Previous != NULL
#endif
        )
         return false;

      DelegateImpl<TFunctor>* impl = dynamic_cast<DelegateImpl<TFunctor>*>(this->last_);
      if (!impl)
         return false;
      return (impl->Functor == f);
   }

   // Operators
public:
   operator bool() const
   {
      return !IsEmpty();
   }

   bool operator!() const
   {
      return IsEmpty();
   }

   template <class TFunctor>
   Delegate& operator=(const TFunctor& f)
   {
      LockBlock<MT_Policy> lock(m_mutex);
      FreeDelegateList(this->last_);
      DelegateImplBase* del = new (nothrow) DelegateImpl<TFunctor>(f);
      if (del)
      {
         this->last_ = del;
      }
      return *this;
   }

   Delegate& operator=(const Delegate& d)
   {
      if (this != &d)
      {
         LockBlock<MT_Policy> lock(m_mutex);
         DelegateImplBase* list = QyneDelegateList(d.last_, NULL);
         FreeDelegateList(this->last_);
         this->last_ = list;
      }
      return *this;
   }

#ifndef USING_LIGHT_DELEGATE
   template <class TFunctor>
   Delegate& operator+=(const TFunctor& f)
   {
      Add(f);
      return *this;
   }

   template <class TFunctor>
   Delegate& operator-=(const TFunctor& f)
   {
      Remove(f);
      return *this;
   }
#endif

   template <class TFunctor>
   bool operator==(const TFunctor& f)
   {
      return this->Exist(f);
   }

   //template <class TFunctor>
   //friend bool operator==(const TFunctor& f, const Delegate& d)
   //{
   //   return (d == f);
   //}

   template <class TFunctor>
   bool operator!=(const TFunctor& f)
   {
      return !(*this == f);
   }

   //template <class TFunctor>
   //friend bool operator!=(const TFunctor& f, const Delegate& d)
   //{
   //   return (d != f);
   //}

   R operator()(X_DELEGATE_FUNCTION_PARAMS) const
   {
      LockBlock<MT_Policy> lock(m_mutex);

      if (this->last_ == NULL)
         return R();
#ifndef USING_LIGHT_DELEGATE
      if (this->last_->Previous != NULL)
         InvokeDelegateList(this->last_->Previous X_DELEGATE_COMMA X_DELEGATE_FUNCTION_ARGS);
#endif
      return this->last_->Invoke(X_DELEGATE_FUNCTION_ARGS);
   }
};

#undef X_DELEGATE_TEMPLATE_PARAMS
#undef X_DELEGATE_TEMPLATE_ARGS
#undef X_DELEGATE_FUNCTION_PARAMS
#undef X_DELEGATE_FUNCTION_ARGS
#undef X_DELEGATE_COMMA
