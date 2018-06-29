// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/29.

#ifndef POLAR_UTILS_CRASH_RECOVERY_CONTEXT_H
#define POLAR_UTILS_CRASH_RECOVERY_CONTEXT_H

#include "polar/basic/adt/StlExtras.h"

namespace polar {
namespace utils {

class CrashRecoveryContextCleanup;

using polar::basic::FunctionRef;

/// \brief Crash recovery helper object.
///
/// This class implements support for running operations in a safe m_context so
/// that crashes (memory errors, stack overflow, assertion violations) can be
/// detected and control restored to the crashing thread. Crash detection is
/// purely "best effort", the exact set of failures which can be recovered from
/// is platform dependent.
///
/// Clients make use of this code by first calling
/// CrashRecoveryContext::Enable(), and then executing unsafe operations via a
/// CrashRecoveryContext object. For example:
///
///    void actual_work(void *);
///
///    void foo() {
///      CrashRecoveryContext CRC;
///
///      if (!CRC.runSafely(actual_work, 0)) {
///         ... a crash was detected, report error to user ...
///      }
///
///      ... no crash was detected ...
///    }
class CrashRecoveryContext
{
   void *m_impl;
   CrashRecoveryContextCleanup *m_head;

public:
   CrashRecoveryContext() : m_impl(nullptr), m_head(nullptr)
   {}
   ~CrashRecoveryContext();

   void registerCleanup(CrashRecoveryContextCleanup *cleanup);
   void unregisterCleanup(CrashRecoveryContextCleanup *cleanup);

   /// \brief Enable crash recovery.
   static void enable();

   /// \brief Disable crash recovery.
   static void disable();

   /// \brief Return the active m_context, if the code is currently executing in a
   /// thread which is in a protected m_context.
   static CrashRecoveryContext *getCurrent();

   /// \brief Return true if the current thread is recovering from a
   /// crash.
   static bool isRecoveringFromCrash();

   /// \brief Execute the provide callback function (with the given arguments) in
   /// a protected m_context.
   ///
   /// \return True if the function completed successfully, and false if the
   /// function crashed (or handleCrash was called explicitly). Clients should
   /// make as little assumptions as possible about the program state when
   /// runSafely has returned false.
   bool runSafely(FunctionRef<void()> func);
   bool runSafely(void (*func)(void*), void *userData)
   {
      return runSafely([&]() { func(userData); });
   }

   /// \brief Execute the provide callback function (with the given arguments) in
   /// a protected m_context which is run in another thread (optionally with a
   /// requested stack size).
   ///
   /// See runSafely() and llvm_execute_on_thread().
   ///
   /// On Darwin, if PRIO_DARWIN_BG is set on the calling thread, it will be
   /// propagated to the new thread as well.
   bool runSafelyOnThread(FunctionRef<void()>, unsigned requestedStackSize = 0);
   bool runSafelyOnThread(void (*func)(void*), void *userData,
                          unsigned requestedStackSize = 0)
   {
      return runSafelyOnThread([&]() { func(userData); }, requestedStackSize);
   }

   /// \brief Explicitly trigger a crash recovery in the current process, and
   /// return failure from runSafely(). This function does not return.
   void handleCrash();
};

class CrashRecoveryContextCleanup
{
protected:
   CrashRecoveryContext *m_context;
   CrashRecoveryContextCleanup(CrashRecoveryContext *context)
      : m_context(context), m_cleanupFired(false)
   {}

public:
   bool m_cleanupFired;

   virtual ~CrashRecoveryContextCleanup();
   virtual void recoverResources() = 0;

   CrashRecoveryContext *getContext() const
   {
      return m_context;
   }

private:
   friend class CrashRecoveryContext;
   CrashRecoveryContextCleanup *m_prev, *m_next;
};

template<typename DERIVED, typename T>
class CrashRecoveryContextCleanupBase : public CrashRecoveryContextCleanup
{
protected:
   T *m_resource;
   CrashRecoveryContextCleanupBase(CrashRecoveryContext *context, T *resource)
      : CrashRecoveryContextCleanup(context), m_resource(resource)
   {}

public:
   static DERIVED *create(T *x)
   {
      if (x) {
         if (CrashRecoveryContext *context = CrashRecoveryContext::getCurrent()) {
            return new DERIVED(context, x);
         }
      }
      return nullptr;
   }
};

template <typename T>
class CrashRecoveryContextDestructorCleanup : public
      CrashRecoveryContextCleanupBase<CrashRecoveryContextDestructorCleanup<T>, T>
{
public:
   CrashRecoveryContextDestructorCleanup(CrashRecoveryContext *context,
                                         T *resource)
      : CrashRecoveryContextCleanupBase<
        CrashRecoveryContextDestructorCleanup<T>, T>(context, resource)
   {}

   virtual void recoverResources()
   {
      this->m_resource->~T();
   }
};

template <typename T>
class CrashRecoveryContextDeleteCleanup : public
      CrashRecoveryContextCleanupBase<CrashRecoveryContextDeleteCleanup<T>, T>
{
public:
   CrashRecoveryContextDeleteCleanup(CrashRecoveryContext *context, T *resource)
      : CrashRecoveryContextCleanupBase<
        CrashRecoveryContextDeleteCleanup<T>, T>(context, resource)
   {}

   void recoverResources() override
   {
      delete this->m_resource;
   }
};

template <typename T>
class CrashRecoveryContextReleaseRefCleanup : public
      CrashRecoveryContextCleanupBase<CrashRecoveryContextReleaseRefCleanup<T>, T>
{
public:
   CrashRecoveryContextReleaseRefCleanup(CrashRecoveryContext *context,
                                         T *resource)
      : CrashRecoveryContextCleanupBase<CrashRecoveryContextReleaseRefCleanup<T>,
        T>(context, resource)
   {}

   void recoverResources() override
   {
      this->m_resource->release();
   }
};

template <typename T, typename Cleanup = CrashRecoveryContextDeleteCleanup<T> >
class CrashRecoveryContextCleanupRegistrar
{
   CrashRecoveryContextCleanup *m_cleanup;

public:
   CrashRecoveryContextCleanupRegistrar(T *x)
      : m_cleanup(Cleanup::create(x))
   {
      if (m_cleanup) {
         m_cleanup->getContext()->registerCleanup(m_cleanup);
      }
   }

   ~CrashRecoveryContextCleanupRegistrar()
   {
      unregister();
   }

   void unregister()
   {
      if (m_cleanup && !m_cleanup->m_cleanupFired) {
         m_cleanup->getContext()->unregisterCleanup(m_cleanup);
      }
      m_cleanup = nullptr;
   }
};

} // utils
} // polar

#endif // POLAR_UTILS_CRASH_RECOVERY_CONTEXT_H
