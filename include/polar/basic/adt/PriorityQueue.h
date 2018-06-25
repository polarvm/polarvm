// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/25.

#ifndef POLAR_BASIC_ADT_PRIORITY_QUEUE_H
#define POLAR_BASIC_ADT_PRIORITY_QUEUE_H

#include <algorithm>
#include <queue>

namespace polar {
namespace basic {

/// PriorityQueue - This class behaves like std::priority_queue and
/// provides a few additional convenience functions.
///
template<class T,
         class Sequence = std::vector<T>,
         class Compare = std::less<typename Sequence::value_type> >
class PriorityQueue : public std::priority_queue<T, Sequence, Compare>
{
public:
   explicit PriorityQueue(const Compare &compare = Compare(),
                          const Sequence &sequence = Sequence())
      : std::priority_queue<T, Sequence, Compare>(compare, sequence)
   {}

   template<class Iterator>
   PriorityQueue(Iterator begin, Iterator end,
                 const Compare &compare = Compare(),
                 const Sequence &sequence = Sequence())
      : std::priority_queue<T, Sequence, Compare>(begin, end, compare, sequence)
   {}

   /// eraseOne - Erase one element from the queue, regardless of its
   /// position. This operation performs a linear search to find an element
   /// equal to t, but then uses all logarithmic-time algorithms to do
   /// the erase operation.
   ///
   void eraseOne(const T &t)
   {
      // Linear-search to find the element.
      typename Sequence::size_type i = find(this->c, t) - this->c.begin();

      // Logarithmic-time heap bubble-up.
      while (i != 0) {
         typename Sequence::size_type parent = (i - 1) / 2;
         this->c[i] = this->c[parent];
         i = parent;
      }

      // The element we want to remove is now at the root, so we can use
      // priority_queue's plain pop to remove it.
      this->pop();
   }

   /// reheapify - If an element in the queue has changed in a way that
   /// affects its standing in the comparison function, the queue's
   /// internal state becomes invalid. Calling reheapify() resets the
   /// queue's state, making it valid again. This operation has time
   /// complexity proportional to the number of elements in the queue,
   /// so don't plan to use it a lot.
   ///
   void reheapify()
   {
      std::make_heap(this->c.begin(), this->c.end(), this->comp);
   }

   /// clear - Erase all elements from the queue.
   ///
   void clear()
   {
      this->c.clear();
   }
};

} // basic
} // polar

#endif // POLAR_BASIC_ADT_PRIORITY_QUEUE_H
