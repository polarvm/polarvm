// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
// 
// Created by softboy on 2018/06/05.

#ifndef POLAR_BASIC_ADT_INTRUSIVE_LIST_BASE_H
#define POLAR_BASIC_ADT_INTRUSIVE_LIST_BASE_H

#include "polar/basic/adt/IntrusiveListNodeBase.h"
#include <cassert>

namespace polar {
namespace basic {

/// Implementations of list algorithms using IntrusiveListNodeBase.
template <bool EnableSentinelTracking>
class IntrusiveListBase {
public:
   using NodeBaseType = IntrusiveListNodeBase<EnableSentinelTracking>;
   
   static void insertBeforeImpl(NodeBaseType &next, NodeBaseType &node)
   {
      NodeBaseType &prev = *next.getPrev();
      node.setNext(&next);
      node.setPrev(&prev);
      prev.setNext(&node);
      next.setPrev(&node);
   }
   
   static void removeImpl(NodeBaseType &node)
   {
      NodeBaseType *prev = node.getPrev();
      NodeBaseType *next = node.getNext();
      next->setPrev(prev);
      prev->setNext(next);
      
      // Not strictly necessary, but helps catch a class of bugs.
      node.setPrev(nullptr);
      node.setNext(nullptr);
   }
   
   static void removeRangeImpl(NodeBaseType &first, NodeBaseType &last)
   {
      NodeBaseType *prev = first.getPrev();
      NodeBaseType *final = last.getPrev();
      last.setPrev(prev);
      prev->setNext(&last);
      
      // Not strictly necessary, but helps catch a class of bugs.
      first.setPrev(nullptr);
      final->setNext(nullptr);
   }
   
   static void transferBeforeImpl(NodeBaseType &next, NodeBaseType &first,
                                  NodeBaseType &last)
   {
      if (&next == &last || &first == &last) {
         return;
      }
      // Position cannot be contained in the range to be transferred.
      assert(&next != &first &&
            // Check for the most common mistake.
            "Insertion point can't be one of the transferred nodes");
      
      NodeBaseType &final = *last.getPrev();
      
      // Detach from old list/position.
      first.getPrev()->setNext(&last);
      last.setPrev(first.getPrev());
      
      // Splice [First, Final] into its new list/position.
      NodeBaseType &prev = *next.getPrev();
      final.setNext(&next);
      first.setPrev(&prev);
      prev.setNext(&first);
      next.setPrev(&final);
   }
   
   template <class T> static void insertBefore(T &next, T &node)
   {
      insertBeforeImpl(next, node);
   }
   
   template <class T> static void remove(T &node)
   {
      removeImpl(node);
   }
   
   template <class T> static void removeRange(T &first, T &last)
   {
      removeRangeImpl(first, last);
   }
   
   template <class T>
   static void transferBefore(T &next, T &first, T &last)
   {
      transferBeforeImpl(next, first, last);
   }
};

} // basic
} // polar

#endif // POLAR_BASIC_ADT_INTRUSIVE_LIST_BASE_H
