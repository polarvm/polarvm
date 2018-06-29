// This source file is part of the polarphp.org open source project
//
// copyright (c) 2017 - 2018 polarPHP software foundation
// copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/29.

#ifndef POLAR_BASIC_ADT_BREADTHFIRSTITERATOR_H
#define POLAR_BASIC_ADT_BREADTHFIRSTITERATOR_H

#include "polar/basic/adt/GraphTraits.h"
#include "polar/basic/adt/SmallPtrSet.h"
#include "polar/basic/adt/IteratorRange.h"
#include <iterator>
#include <queue>
#include <utility>
#include <optional>

namespace polar {
namespace basic {

// BreadthFirstIteratorStorage - A private class which is used to figure out where to
// store the visited set. We only provide a non-external variant for now.
template <class SetType>
class BreadthFirstIteratorStorage
{
public:
   SetType m_visited;
};

// The visited state for the iteration is a simple set.
template <typename NodeRef, unsigned SmallSize = 8>
using BreadthFirstIteratorDefaultSet = SmallPtrSet<NodeRef, SmallSize>;

// Generic Breadth first search iterator.
template <class GraphT,
          class SetType =
          BreadthFirstIteratorDefaultSet<typename GraphTraits<GraphT>::NodeRef>,
          class GT = GraphTraits<GraphT>>
class BreadthFirstIterator
      : public std::iterator<std::forward_iterator_tag, typename GT::NodeRef>,
      public BreadthFirstIteratorStorage<SetType>
{
   using super = std::iterator<std::forward_iterator_tag, typename GT::NodeRef>;

   using NodeRef = typename GT::NodeRef;
   using childIterTy = typename GT::childItereratorType;

   // First element is the node reference, second is the next child to visit.
   using QueueElement = std::pair<NodeRef, std::optional<childIterTy>>;

   // Visit queue - used to maintain BFS ordering.
   // std::optional<> because we need markers for levels.
   std::queue<std::optional<QueueElement>> m_visitQueue;

   // Current level.
   unsigned m_level;

private:
   inline BreadthFirstIterator(NodeRef node)
   {
      this->Visited.insert(node);
      m_level = 0;

      // Also, insert a dummy node as marker.
      m_visitQueue.push(QueueElement(std::nullopt, std::nullopt));
      m_visitQueue.push(std::nullopt);
   }

   inline BreadthFirstIterator() = default;

   inline void toNext()
   {
      std::optional<QueueElement> m_head = m_visitQueue.front();
      QueueElement H = m_head.getValue();
      NodeRef node = H.first;
      std::optional<childIterTy> &childIter = H.second;

      if (!childIter) {
         childIter.emplace(GT::childBegin(node));
      }
      while (*childIter != GT::childEnd(node)) {
         NodeRef next = *(*childIter)++;
         // Already visited?
         if (this->Visited.insert(next).second) {
            m_visitQueue.push(QueueElement(next, std::nullopt));
         }
      }
      m_visitQueue.pop();

      // Go to the next element skipping markers if needed.
      if (!m_visitQueue.empty()) {
         m_head = m_visitQueue.front();
         if (m_head != std::nullopt) {
            return;
         }
         m_level += 1;
         m_visitQueue.pop();

         // Don't push another marker if this is the last
         // element.
         if (!m_visitQueue.empty()) {
            m_visitQueue.push(std::nullopt);
         }
      }
   }

public:
   using pointer = typename super::pointer;

   // Provide static begin and end methods as our public "constructors"
   static BreadthFirstIterator begin(const GraphT &graph)
   {
      return BreadthFirstIterator(GT::getEntryNode(graph));
   }

   static BreadthFirstIterator end(const GraphT &graph)
   {
      return BreadthFirstIterator();
   }

   bool operator==(const BreadthFirstIterator &other) const
   {
      return m_visitQueue == other.m_visitQueue;
   }

   bool operator!=(const BreadthFirstIterator &other) const
   {
      return !(*this == other);
   }

   const NodeRef &operator*() const
   {
      return m_visitQueue.front()->first;
   }

   // This is a nonstandard operator-> that dereferenfces the pointer an extra
   // time so that you can actually call methods on the node, because the
   // contained type is a pointer.
   NodeRef operator->() const
   {
      return **this;
   }

   BreadthFirstIterator &operator++()
   { // Pre-increment
      toNext();
      return *this;
   }

   BreadthFirstIterator operator++(int)
   { // Post-increment
      BreadthFirstIterator iterCopy = *this;
      ++*this;
      return iterCopy;
   }

   unsigned getLevel() const
   {
      return m_level;
   }
};

// Provide global constructors that automatically figure out correct types.
template <class T>
BreadthFirstIterator<T> bf_begin(const T &graph)
{
   return BreadthFirstIterator<T>::begin(graph);
}

template <class T>
BreadthFirstIterator<T> bf_end(const T &graph)
{
   return BreadthFirstIterator<T>::end(graph);
}

// Provide an accessor method to use them in range-based patterns.
template <class T>
IteratorRange<BreadthFirstIterator<T>> breadth_first(const T &graph)
{
   return make_range(bf_begin(graph), bf_end(graph));
}

} // basic
} // polar

#endif // POLAR_BASIC_ADT_BREADTHFIRSTITERATOR_H
