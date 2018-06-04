// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
// 
// Created by softboy on 2018/06/04.

#ifndef POLAR_GLOBAL_MANAGED_STATIC_H
#define POLAR_GLOBAL_MANAGED_STATIC_H

#include <atomic>
#include <cstddef>

namespace polar {

using GlobalCreatorFuncType = void *();
using GlobalDeleterFuncType = void (void *);

/// ObjectCreator - Helper method for ManagedStatic.
template <typename ClassType>
struct ObjectCreator
{
   static void *call()
   {
      return new ClassType();
   }
};

/// ObjectDeleter - Helper method for ManagedStatic.
///
template <typename T>
struct ObjectDeleter
{
   static void call(void *ptr)
   {
      delete (T *)ptr;
   }
};
template <typename T, size_t N>
struct ObjectDeleter<T[N]>
{
   static void call(void *ptr)
   {
      delete[](T *)ptr;
   }
};

/// ManagedStaticBase - Common base class for ManagedStatic instances.
class ManagedStaticBase
{
protected:
   // This should only be used as a static variable, which guarantees that this
   // will be zero initialized.
   mutable std::atomic<void *> m_ptr;
   mutable GlobalDeleterFuncType *m_deleterFn;
   mutable const ManagedStaticBase *m_next;
   
   void registerManagedStatic(GlobalCreatorFuncType *creator, GlobalDeleterFuncType *deleter) const;
   
public:
   /// isConstructed - Return true if this object has not been created yet.
   bool isConstructed() const
   {
      return m_ptr != nullptr;
   }
   
   void destroy() const;
};

/// ManagedStatic - This transparently changes the behavior of global statics to
/// be lazily constructed on demand (good for reducing startup times of dynamic
/// libraries that link in LLVM components) and for making destruction be
/// explicit through the llvm_shutdown() function call.
///
template <typename ClassType, typename Creator = ObjectCreator<ClassType>,
          typename Deleter = ObjectDeleter<ClassType>>
class ManagedStatic : public ManagedStaticBase
{
public:
   // Accessors.
   ClassType &operator*()
   {
      void *temp = m_ptr.load(std::memory_order_acquire);
      if (!temp) {
         registerManagedStatic(Creator::call, Deleter::call);
      }
      return *static_cast<ClassType *>(m_ptr.load(std::memory_order_relaxed));
   }
   
   ClassType *operator->()
   {
      return &**this;
   }
   
   const ClassType &operator*() const
   {
      void *temp = m_ptr.load(std::memory_order_acquire);
      if (!temp) {
         registerManagedStatic(Creator::call, Deleter::call);
      }
      return *static_cast<ClassType *>(m_ptr.load(std::memory_order_relaxed));
   }
   
   const ClassType *operator->() const
   {
      return &**this;
   }
};

/// shutdown - Deallocate and destroy all ManagedStatic variables.
void shutdown();

/// ShutdownObject - This is a simple helper class that calls
/// ::polar::shutdown() when it is destroyed.
struct ShutdownObject
{
   ShutdownObject() = default;
   ~ShutdownObject()
   {
      ::polar::shutdown();
   }
};

} // polar

#endif // POLAR_GLOBAL_MANAGED_STATIC_H
