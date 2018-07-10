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

#include "polar/global/ManagedStatic.h"
#include "polar/global/Config.h"
#include <thread>
#include <mutex>
#include <cassert>
#include <iostream>

namespace polar {
static const ManagedStaticBase *sg_staticList = nullptr;

namespace {

std::recursive_mutex& get_managed_static_mutex()
{
   // We need to use a function local static here, since this can get called
   // during a static constructor and we need to guarantee that it's initialized
   // correctly.
   static std::recursive_mutex managedStaticMutex;
   return managedStaticMutex;
}

} // anonymous namespace

void ManagedStaticBase::registerManagedStatic(GlobalCreatorFuncType *creator, GlobalDeleterFuncType *deleter) const
{
   assert(creator);
   std::lock_guard lock(get_managed_static_mutex());
   if (!m_ptr.load(std::memory_order_relaxed)) {
      void *temp = creator();
      
      m_ptr.store(temp, std::memory_order_release);
      m_deleterFn = deleter;
      
      // Add to list of managed statics.
      m_next = sg_staticList;
      sg_staticList = this;
   }
}

void ManagedStaticBase::destroy() const
{
   assert(m_deleterFn && "ManagedStatic not initialized correctly!");
   assert(sg_staticList == this &&
          "Not destroyed in reverse order of construction?");
   // Unlink from list.
   sg_staticList = m_next;
   m_next = nullptr;
   
   // Destroy memory.
   m_deleterFn(m_ptr);
   // Cleanup.
   m_ptr = nullptr;
   m_deleterFn = nullptr;
}

/// ::polar::shutdown - Deallocate and destroy all ManagedStatic variables.
void shutdown()
{
   std::lock_guard lock(get_managed_static_mutex());
   while (sg_staticList) {
      sg_staticList->destroy();
   }
}

} // polar
