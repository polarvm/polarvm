// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/01.

#ifndef POLAR_UTILS_PARALLEL_H
#define POLAR_UTILS_PARALLEL_H

#include "polar/basic/adt/StlExtras.h"
#include "polar/utils/MathExtras.h"

#include <algorithm>
#include <condition_variable>
#include <functional>
#include <mutex>

#if defined(_MSC_VER) && LLVM_ENABLE_THREADS
#pragma warning(push)
#pragma warning(disable : 4530)
#include <concrt.h>
#include <ppl.h>
#pragma warning(pop)
#endif

namespace polar {
namespace utils {

namespace parallel {
struct sequential_execution_policy {};
struct parallel_execution_policy {};

template <typename T>
struct is_execution_policy
      : public std::integral_constant<
      bool, polar::basic::is_one_of<T, sequential_execution_policy,
      parallel_execution_policy>::value>
{};

constexpr sequential_execution_policy seq{};
constexpr parallel_execution_policy par{};

namespace internal {


class Latch
{
   uint32_t m_count;
   mutable std::mutex m_mutex;
   mutable std::condition_variable m_cond;

public:
   explicit Latch(uint32_t count = 0) : m_count(count)
   {}
   ~Latch()
   {
      sync();
   }

   void inc()
   {
      std::unique_lock<std::mutex> lock(m_mutex);
      ++m_count;
   }

   void dec()
   {
      std::unique_lock<std::mutex> lock(m_mutex);
      if (--m_count == 0) {
         m_cond.notify_all();
      }
   }

   void sync() const
   {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_cond.wait(lock, [&] { return m_count == 0; });
   }
};

class TaskGroup
{
   Latch m_latch;

public:
   void spawn(std::function<void()> func);

   void sync() const
   {
      m_latch.sync();
   }
};

#if defined(_MSC_VER)
template <class RandomAccessIterator, class Comparator>
void parallel_sort(RandomAccessIterator start, RandomAccessIterator end,
                   const Comparator &comp)
{
   concurrency::parallel_sort(start, end, comp);
}

template <class IterTy, class FuncTy>
void parallel_for_each(IterTy begin, IterTy end, FuncTy func)
{
   concurrency::parallel_for_each(begin, end, func);
}

template <class IndexTy, class FuncTy>
void parallel_for_each_n(IndexTy begin, IndexTy end, FuncTy func)
{
   concurrency::parallel_for(begin, end, func);
}

#else
const ptrdiff_t sg_minParallelSize = 1024;

/// \brief Inclusive median.
template <class RandomAccessIterator, class Comparator>
RandomAccessIterator median_of_three(RandomAccessIterator start,
                                     RandomAccessIterator end,
                                     const Comparator &comp)
{
   RandomAccessIterator mid = start + (std::distance(start, end) / 2);
   return comp(*start, *(end - 1))
         ? (comp(*mid, *(end - 1)) ? (comp(*start, *mid) ? mid : start)
                                   : end - 1)
         : (comp(*mid, *start) ? (comp(*(end - 1), *mid) ? mid : end - 1)
                               : start);
}

template <class RandomAccessIterator, class Comparator>
void parallel_quick_sort(RandomAccessIterator start, RandomAccessIterator end,
                         const Comparator &comp, TaskGroup &taskGroup, size_t depth)
{
   // Do a sequential sort for small inputs.
   if (std::distance(start, end) < internal::sg_minParallelSize || depth == 0) {
      std::sort(start, end, comp);
      return;
   }

   // Partition.
   auto pivot = median_of_three(start, end, comp);
   // Move pivot to end.
   std::swap(*(end - 1), *pivot);
   pivot = std::partition(start, end - 1, [&comp, end](decltype(*start) value) {
      return comp(value, *(end - 1));
   });
   // Move pivot to middle of partition.
   std::swap(*pivot, *(end - 1));

   // Recurse.
   taskGroup.spawn([=, &comp, &taskGroup] {
      parallel_quick_sort(start, pivot, comp, taskGroup, depth - 1);
   });
   parallel_quick_sort(pivot + 1, end, comp, taskGroup, depth - 1);
}

template <class RandomAccessIterator, class Comparator>
void parallel_sort(RandomAccessIterator start, RandomAccessIterator end,
                   const Comparator &comp)
{
   TaskGroup taskGroup;
   parallel_quick_sort(start, end, comp, taskGroup,
                       llvm::Log2_64(std::distance(start, end)) + 1);
}

template <class IterTy, class FuncTy>
void parallel_for_each(IterTy begin, IterTy end, FuncTy func)
{
   // TaskGroup has a relatively high overhead, so we want to reduce
   // the number of spawn() calls. We'll create up to 1024 tasks here.
   // (Note that 1024 is an arbitrary number. This code probably needs
   // improving to take the number of available cores into account.)
   ptrdiff_t taskSize = std::distance(begin, end) / 1024;
   if (taskSize == 0)
      taskSize = 1;

   TaskGroup taskGroup;
   while (taskSize < std::distance(begin, end)) {
      taskGroup.spawn([=, &func] { std::for_each(begin, begin + taskSize, func); });
      begin += taskSize;
   }
   std::for_each(begin, end, func);
}

template <class IndexTy, class FuncTy>
void parallel_for_each_n(IndexTy begin, IndexTy end, FuncTy func)
{
   ptrdiff_t taskSize = (end - begin) / 1024;
   if (taskSize == 0) {
      taskSize = 1;
   }
   TaskGroup taskGroup;
   IndexTy index = begin;
   for (; index + taskSize < end; index += taskSize) {
      taskGroup.spawn([=, &func] {
         for (IndexTy j = index, jend = index + taskSize; j != jend; ++j) {
            func(j);
         }
      });
   }
   for (IndexTy j = index; j < end; ++j) {
      func(j);
   }
}

#endif

template <typename Iter>
using DefComparator =
std::less<typename std::iterator_traits<Iter>::value_type>;

} // namespace internal

// sequential algorithm implementations.
template <class Policy, class RandomAccessIterator,
          class Comparator = internal::DefComparator<RandomAccessIterator>>
void sort(Policy policy, RandomAccessIterator start, RandomAccessIterator end,
          const Comparator &comp = Comparator())
{
   static_assert(is_execution_policy<Policy>::value,
                 "Invalid execution policy!");
   std::sort(start, end, comp);
}

template <class Policy, class IterTy, class FuncTy>
void for_each(Policy policy, IterTy begin, IterTy end, FuncTy func)
{
   static_assert(is_execution_policy<Policy>::value,
                 "Invalid execution policy!");
   std::for_each(begin, end, func);
}

template <class Policy, class IndexTy, class FuncTy>
void for_each_n(Policy policy, IndexTy begin, IndexTy end, FuncTy func)
{
   static_assert(is_execution_policy<Policy>::value,
                 "Invalid execution policy!");
   for (IndexTy i = begin; i != end; ++i) {
      func(i);
   }
}

// Parallel algorithm implementations, only available when LLVM_ENABLE_THREADS
// is true.
template <class RandomAccessIterator,
          class Comparator = internal::DefComparator<RandomAccessIterator>>
void sort(parallel_execution_policy policy, RandomAccessIterator start,
          RandomAccessIterator end, const Comparator &comp = Comparator())
{
   internal::parallel_sort(start, end, comp);
}

template <class IterTy, class FuncTy>
void for_each(parallel_execution_policy policy, IterTy begin, IterTy end,
              FuncTy func)
{
   internal::parallel_for_each(begin, end, func);
}

template <class IndexTy, class FuncTy>
void for_each_n(parallel_execution_policy policy, IndexTy begin, IndexTy end,
                FuncTy func)
{
   internal::parallel_for_each_n(begin, end, func);
}

} // namespace parallel

} // utils
} // basic

#endif // POLAR_UTILS_PARALLEL_H
