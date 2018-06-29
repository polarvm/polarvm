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

#ifndef POLAR_BASIC_ADT_DEPTHFIRSTITERATOR_H
#define POLAR_BASIC_ADT_DEPTHFIRSTITERATOR_H

#include "polar/basic/adt/GraphTraits.h"
#include "polar/basic/adt/SmallPtrSet.h"
#include "polar/basic/adt/IteratorRange.h"
#include <iterator>
#include <set>
#include <utility>
#include <vector>

namespace polar {
namespace basic {

// DepthFirstIteratorStorage - A private class which is used to figure out where to
// store the visited set.
template<class SetType, bool External>   // Non-external set
class DepthFirstIteratorStorage
{
public:
   SetType m_visited;
};

template<class SetType>
class DepthFirstIteratorStorage<SetType, true>
{
public:
   DepthFirstIteratorStorage(SetType &vset) : m_visited(vset)
   {}
   DepthFirstIteratorStorage(const DepthFirstIteratorStorage &set) : m_visited(set.m_visited)
   {}

   SetType &m_visited;
};

// The visited stated for the iteration is a simple set augmented with
// one more method, completed, which is invoked when all children of a
// node have been processed. It is intended to distinguish of back and
// cross edges in the spanning tree but is not used in the common case.
template <typename NodeRef, unsigned SmallSize=8>
struct DepthFirstIteratorDefaultSet : public SmallPtrSet<NodeRef, SmallSize>
{
   using BaseSet = SmallPtrSet<NodeRef, SmallSize>;
   using iterator = typename BaseSet::iterator;

   std::pair<iterator,bool> insert(NodeRef node)
   {
      return BaseSet::insert(node);
   }

   template <typename IterT>
   void insert(IterT begin, IterT end)
   {
      BaseSet::insert(begin, end);
   }

   void completed(NodeRef)
   {}
};

// Generic Depth First Iterator
template <class GraphT,
          class SetType =
          DepthFirstIteratorDefaultSet<typename GraphTraits<GraphT>::NodeRef>,
          bool ExtStorage = false, class GT = GraphTraits<GraphT>>
class DepthFirstIterator
      : public std::iterator<std::forward_iterator_tag, typename GT::NodeRef>,
      public DepthFirstIteratorStorage<SetType, ExtStorage>
{
   using super = std::iterator<std::forward_iterator_tag, typename GT::NodeRef>;
   using NodeRef = typename GT::NodeRef;
   using ChildItTy = typename GT::ChildIteratorType;

   // First element is node reference, second is the 'next child' to visit.
   // The second child is initialized lazily to pick up graph changes during the
   // DFS.
   using StackElement = std::pair<NodeRef, std::optional<ChildItTy>>;

   // m_visitStack - Used to maintain the ordering.  Top = current block
   std::vector<StackElement> m_visitStack;

private:
   inline DepthFirstIterator(NodeRef node)
   {
      this->m_visited.insert(node);
      m_visitStack.push_back(StackElement(node, std::nullopt));
   }

   inline DepthFirstIterator() = default; // End is when stack is empty

   inline DepthFirstIterator(NodeRef node, SetType &set)
      : DepthFirstIteratorStorage<SetType, ExtStorage>(set) {
      if (this->m_visited.insert(node).second)
         m_visitStack.push_back(StackElement(node, None));
   }

   inline DepthFirstIterator(SetType &set)
      : DepthFirstIteratorStorage<SetType, ExtStorage>(set) {
      // End is when stack is empty
   }

   inline void toNext()
   {
      do {
         NodeRef node = m_visitStack.back().first;
         std::optional<ChildItTy> &opt = m_visitStack.back().second;

         if (!opt) {
            opt.emplace(GT::child_begin(node));
         }

         // Notice that we directly mutate *opt here, so that
         // m_visitStack.back().second actually gets updated as the iterator
         // increases.
         while (*opt != GT::child_end(node)) {
            NodeRef next = *(*opt)++;
            // Has our next sibling been visited?
            if (this->m_visited.insert(next).second) {
               // No, do it now.
               m_visitStack.push_back(StackElement(next, std::nullopt));
               return;
            }
         }
         this->m_visited.completed(node);

         // Oops, ran out of successors... go up a level on the stack.
         m_visitStack.pop_back();
      } while (!m_visitStack.empty());
   }

public:
   using pointer = typename super::pointer;

   // Provide static begin and end methods as our public "constructors"
   static DepthFirstIterator begin(const GraphT &graph)
   {
      return DepthFirstIterator(GT::getEntryNode(graph));
   }

   static DepthFirstIterator end(const GraphT &graph)
   {
      return DepthFirstIterator();
   }

   // Static begin and end methods as our public ctors for external iterators
   static DepthFirstIterator begin(const GraphT &graph, SetType &set)
   {
      return DepthFirstIterator(GT::getEntryNode(graph), set);
   }

   static DepthFirstIterator end(const GraphT &graph, SetType &set)
   {
      return DepthFirstIterator(set);
   }

   bool operator==(const DepthFirstIterator &iter) const
   {
      return m_visitStack == iter.m_visitStack;
   }

   bool operator!=(const DepthFirstIterator &iter) const
   {
      return !(*this == iter);
   }

   const NodeRef &operator*() const
   {
      return m_visitStack.back().first;
   }

   // This is a nonstandard operator-> that dereferences the pointer an extra
   // time... so that you can actually call methods ON the node, because
   // the contained type is a pointer.  This allows BBIt->getTerminator() f.e.
   //
   NodeRef operator->() const
   {
      return **this;
   }

   DepthFirstIterator &operator++()
   { // Preincrement
      toNext();
      return *this;
   }

   /// \brief Skips all children of the current node and traverses to next node
   ///
   /// Note: This function takes care of incrementing the iterator. If you
   /// always increment and call this function, you risk walking off the end.
   DepthFirstIterator &skipChildren()
   {
      m_visitStack.pop_back();
      if (!m_visitStack.empty()) {
         toNext();
      }
      return *this;
   }

   DepthFirstIterator operator++(int)
   { // Postincrement
      DepthFirstIterator tmp = *this;
      ++*this;
      return tmp;
   }

   // nodeVisited - return true if this iterator has already visited the
   // specified node.  This is public, and will probably be used to iterate over
   // nodes that a depth first iteration did not find: ie unreachable nodes.
   //
   bool nodeVisited(NodeRef node) const
   {
      return this->m_visited.count(node) != 0;
   }

   /// getPathLength - Return the length of the path from the entry node to the
   /// current node, counting both nodes.
   unsigned getPathLength() const
   {
      return m_visitStack.size();
   }

   /// getPath - Return the n'th node in the path from the entry node to the
   /// current node.
   NodeRef getPath(unsigned n) const
   {
      return m_visitStack[n].first;
   }
};

// Provide global constructors that automatically figure out correct types...
//
template <class T>
DepthFirstIterator<T> df_begin(const T& graph) {
   return DepthFirstIterator<T>::begin(graph);
}

template <class T>
DepthFirstIterator<T> df_end(const T& graph) {
   return DepthFirstIterator<T>::end(graph);
}

// Provide an accessor method to use them in range-based patterns.
template <class T>
IteratorRange<DepthFirstIterator<T>> depth_first(const T& graph) {
   return make_range(df_begin(graph), df_end(graph));
}

// Provide global definitions of external depth first iterators...
template <class T, class SetTy = std::set<typename GraphTraits<T>::NodeRef>>
struct df_ext_iterator : public DepthFirstIterator<T, SetTy, true> {
   df_ext_iterator(const DepthFirstIterator<T, SetTy, true> &iter)
      : DepthFirstIterator<T, SetTy, true>(iter) {}
};

template <class T, class SetTy>
df_ext_iterator<T, SetTy> df_ext_begin(const T& graph, SetTy &set) {
   return df_ext_iterator<T, SetTy>::begin(graph, set);
}

template <class T, class SetTy>
df_ext_iterator<T, SetTy> df_ext_end(const T& graph, SetTy &set) {
   return df_ext_iterator<T, SetTy>::end(graph, set);
}

template <class T, class SetTy>
IteratorRange<df_ext_iterator<T, SetTy>> depth_first_ext(const T& graph,
                                                         SetTy &set) {
   return make_range(df_ext_begin(graph, set), df_ext_end(graph, set));
}

// Provide global definitions of inverse depth first iterators...
template <class T,
          class SetTy =
          DepthFirstIteratorDefaultSet<typename GraphTraits<T>::NodeRef>,
          bool External = false>
struct inverseDepthFirstIterator : public DepthFirstIterator<Inverse<T>, SetTy, External> {
   inverseDepthFirstIterator(const DepthFirstIterator<Inverse<T>, SetTy, External> &iter)
      : DepthFirstIterator<Inverse<T>, SetTy, External>(iter) {}
};

template <class T>
inverseDepthFirstIterator<T> idf_begin(const T& graph) {
   return inverseDepthFirstIterator<T>::begin(Inverse<T>(graph));
}

template <class T>
inverseDepthFirstIterator<T> idf_end(const T& graph){
   return inverseDepthFirstIterator<T>::end(Inverse<T>(graph));
}

// Provide an accessor method to use them in range-based patterns.
template <class T>
IteratorRange<inverseDepthFirstIterator<T>> inverse_depth_first(const T& graph) {
   return make_range(idf_begin(graph), idf_end(graph));
}

// Provide global definitions of external inverse depth first iterators...
template <class T, class SetTy = std::set<typename GraphTraits<T>::NodeRef>>
struct idf_ext_iterator : public inverseDepthFirstIterator<T, SetTy, true> {
   idf_ext_iterator(const inverseDepthFirstIterator<T, SetTy, true> &iter)
      : inverseDepthFirstIterator<T, SetTy, true>(iter) {}
   idf_ext_iterator(const DepthFirstIterator<Inverse<T>, SetTy, true> &iter)
      : inverseDepthFirstIterator<T, SetTy, true>(iter) {}
};

template <class T, class SetTy>
idf_ext_iterator<T, SetTy> idf_ext_begin(const T& graph, SetTy &set)
{
   return idf_ext_iterator<T, SetTy>::begin(Inverse<T>(graph), set);
}

template <class T, class SetTy>
idf_ext_iterator<T, SetTy> idf_ext_end(const T& graph, SetTy &set)
{
   return idf_ext_iterator<T, SetTy>::end(Inverse<T>(graph), set);
}

template <class T, class SetTy>
IteratorRange<idf_ext_iterator<T, SetTy>> inverse_depth_first_ext(const T& graph,
                                                                  SetTy &set)
{
   return make_range(idf_ext_begin(graph, set), idf_ext_end(graph, set));
}

} // basic
} // polar

#endif // POLAR_BASIC_ADT_DEPTHFIRSTITERATOR_H
