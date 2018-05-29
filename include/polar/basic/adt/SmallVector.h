// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
// 
// Created by softboy on 2018/05/28.

#ifndef POLAR_BASIC_ADT_SMALL_VECTOR_H
#define POLAR_BASIC_ADT_SMALL_VECTOR_H

#include "polar/basic/adt/IteratorRange.h"
#include "polar/utils/AlignOf.h"
#include "polar/utils/MathExtras.h"
#include "polar/utils/TypeTraits.h"
#include "polar/utils/ErrorHandling.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace polar {
namespace basic {

using utils = polar::utils;

/// This is all the non-templated stuff common to all SmallVectors.
class SmallVectorBase
{
protected:
   void *m_beginX;
   void *m_endX;
   void *m_capacityX;
   
protected:
   SmallVectorBase(void *firstEl, size_t size)
      : m_beginX(firstEl),
        m_endX(firstEl),
        m_capacityX((char*) firstEl + size)
   {}
   
   /// This is an implementation of the grow() method which only works
   /// on POD-like data types and is out of line to reduce code duplication.
   void growPod(void *firstEl, size_t minSizeInBytes, size_t tsize);
   
public:
   /// This returns size()*sizeof(T).
   size_t getSizeInBytes() const {
      return size_t((char*)m_endX - (char*)m_beginX);
   }
   
   /// capacity_in_bytes - This returns capacity()*sizeof(T).
   size_t getCapacityInBytes() const
   {
      return size_t((char*)m_capacityX - (char*)m_beginX);
   }
   
   POLAR_NODISCARD bool empty() const
   {
      return m_beginX == m_endX;
   }
};

/// This is the part of SmallVectorTemplateBase which does not depend on whether
/// the type T is a POD. The extra dummy template argument is used by ArrayRef
/// to avoid unnecessarily requiring T to be complete.
template <typename T, typename = void>
class SmallVectorTemplateCommon : public SmallVectorBase
{
private:
   template <typename, unsigned> friend struct SmallVectorStorage;
   
   // Allocate raw space for N elements of type T.  If T has a ctor or dtor, we
   // don't want it to be automatically run, so we need to represent the space as
   // something else.  Use an array of char of sufficient alignment.
   using U = AlignedCharArrayUnion<T>;
   U m_firstEl;
   // Space after 'firstEl' is clobbered, do not add any instance vars after it.
   
protected:
   SmallVectorTemplateCommon(size_t size) : SmallVectorBase(&firstEl, size)
   {}
   
   void growPod(size_t minSizeInBytes, size_t tsize)
   {
      SmallVectorBase::growPod(&m_firstEl, minSizeInBytes, tsize);
   }
   
   /// Return true if this is a smallvector which has not had dynamic
   /// memory allocated for it.
   bool isSmall() const
   {
      return m_beginX == static_cast<const void*>(&m_firstEl);
   }
   
   /// Put this vector in a state of being small.
   void resetToSmall()
   {
      m_beginX = m_endX = m_capacityX = &m_firstEl;
   }
   
   void setEnd(T *end)
   {
      m_endX = end;
   }
   
public:
   using size_type = size_t;
   using difference_type = ptrdiff_t;
   using value_type = T;
   using iterator = T *;
   using const_iterator = const T *;
   
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;
   using reverse_iterator = std::reverse_iterator<iterator>;
   
   using reference = T &;
   using const_reference = const T &;
   using pointer = T *;
   using const_pointer = const T *;
   
   // forward iterator creation methods.
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   iterator begin()
   {
      return (iterator)m_beginX;
   }
   
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   const_iterator begin() const
   {
      return (const_iterator)m_beginX;
   }
   
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   iterator end()
   {
      return (iterator)m_endX;
   }
   
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   const_iterator end() const
   {
      return (const_iterator)m_endX;
   }
   
protected:
   iterator getCapacityPtr()
   {
      return (iterator)m_capacityX;
   }
   
   const_iterator getCapacityPtr() const
   {
      return (const_iterator)m_capacityX;
   }
   
public:
   // reverse iterator creation methods.
   reverse_iterator rbegin()
   {
      return reverse_iterator(end());
   }
   
   const_reverse_iterator rbegin() const
   {
      return const_reverse_iterator(end());
   }
   
   reverse_iterator rend()
   {
      return reverse_iterator(begin());
   }
   
   const_reverse_iterator rend() const
   {
      return const_reverse_iterator(begin());
   }
   
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   size_type getSize() const
   {
      return end() - begin();
   }
   
   size_type getMaxSize() const
   {
      return size_type(-1) / sizeof(T);
   }
   
   /// Return the total number of elements in the currently allocated buffer.
   size_t getCapacity() const
   {
      return capacityPtr() - begin();
   }
   
   /// Return a pointer to the vector's buffer, even if empty().
   pointer getData()
   { 
      return pointer(begin());
   }
   
   /// Return a pointer to the vector's buffer, even if empty().
   const_pointer getData() const
   {
      return const_pointer(begin());
   }
   
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   reference operator[](size_type idx)
   {
      assert(idx < getSize());
      return begin()[idx];
   }
   
   POLAR_ATTRIBUTE_ALWAYS_INLINE
   const_reference operator[](size_type idx) const
   {
      assert(idx < getSize());
      return begin()[idx];
   }
   
   reference getFront()
   {
      assert(!empty());
      return begin()[0];
   }
   
   inline reference front()
   {
      return getFront();
   }
   
   const_reference getFront() const
   {
      assert(!empty());
      return begin()[0];
   }
   
   inline const_reference front()
   {
      return getFront();
   }
   
   reference getBack()
   {
      assert(!empty());
      return end()[-1];
   }
   
   inline reference back()
   {
      return getBack();
   }
   
   const_reference getBack() const
   {
      assert(!empty());
      return end()[-1];
   }
   
   inline const_reference back()
   {
      return getBack();
   }
};

/// SmallVectorTemplateBase<isPodLike = false> - This is where we put method
/// implementations that are designed to work with non-POD-like T's.
template <typename T, bool isPodLike>
class SmallVectorTemplateBase : public SmallVectorTemplateCommon<T>
{
protected:
   SmallVectorTemplateBase(size_t size) : SmallVectorTemplateCommon<T>(size)
   {}
   
   static void destroyRange(T *start, T *end)
   {
      while (start != end) {
         --end;
         end->~T();
      }
   }
   
   /// Move the range [I, E) into the uninitialized memory starting with "Dest",
   /// constructing elements as needed.
   template<typename It1, typename It2>
   static void uninitializedMove(It1 Iter, It1 end, It2 dest)
   {
      std::uninitialized_copy(std::make_move_iterator(Iter),
                              std::make_move_iterator(end), dest);
   }
   
   /// Copy the range [I, E) onto the uninitialized memory starting with "Dest",
   /// constructing elements as needed.
   template<typename It1, typename It2>
   static void uninitializedCopy(It1 Iter, It1 end, It2 dest)
   {
      std::uninitialized_copy(Iter, end, dest);
   }
   
   /// Grow the allocated memory (without initializing new elements), doubling
   /// the size of the allocated memory. Guarantees space for at least one more
   /// element, or MinSize more elements if specified.
   void grow(size_t minSize = 0);
   
public:
   
   void pushBack(const T &element)
   {
      if (POLAR_UNLIKELY(m_endX >= m_capacityX)) {
         grow();
      } 
      ::new ((void*) end()) T(element);
      setEnd(end() + 1);
   }
   
   inline void push_back(const T &element)
   {
      pushBack(element);
   }
   
   void pushBack(T &&element)
   {
      if (POLAR_UNLIKELY(m_endX >= m_capacityX)) {
         grow();
      }
      ::new ((void*) end()) T(::std::move(element));
      setEnd(end() + 1);
   }
   
   inline void push_back(T &&element)
   {
      pushBack(std::move(element));
   }
   
   void popBack()
   {
      setEnd(end() - 1);
      end()->~T();
   }
   
   inline void pop_back()
   {
      popBack();
   }
};

// Define this out-of-line to dissuade the C++ compiler from inlining it.
template <typename T, bool isPodLike>
void SmallVectorTemplateBase<T, isPodLike>::grow(size_t minSize)
{
   size_t curCapacity = getCapacity();
   size_t curSize = getSize();
   // Always grow, even from zero.
   size_t newCapacity = size_t(utils::next_power_of_two(curCapacity+2));
   if (newCapacity < minSize) {
      newCapacity = minSize;
   }
   T *newElts = static_cast<T*>(malloc(newCapacity * sizeof(T)));
   if (newElts == nullptr) {
      utils::report_bad_alloc_error("Allocation of SmallVector element failed.");
   }
   // Move the elements over.
   uninitializedMove(begin(), end(), newElts);
   
   // Destroy the original elements.
   destroy_range(begin(), end());
   
   // If this wasn't grown from the inline copy, deallocate the old space.
   if (!isSmall()) {
      free(begin());
   }
   setEnd(NewElts + curSize);
   m_beginX = NewElts;
   m_capacityX = begin() + newCapacity;
}


/// SmallVectorTemplateBase<isPodLike = true> - This is where we put method
/// implementations that are designed to work with POD-like T's.
template <typename T>
class SmallVectorTemplateBase<T, true> : public SmallVectorTemplateCommon<T>
{
protected:
   SmallVectorTemplateBase(size_t size) : SmallVectorTemplateCommon<T>(size)
   {}
   
   // No need to do a destroy loop for POD's.
   static void destroyRange(T *, T *)
   {}
   
   /// Move the range [I, E) onto the uninitialized memory
   /// starting with "Dest", constructing elements into it as needed.
   template<typename It1, typename It2>
   static void uninitialized_move(It1 iter, It1 end, It2 dest)
   {
      // Just do a copy.
      uninitializedCopy(iter, end, dest);
   }
   
   /// Copy the range [I, E) onto the uninitialized memory
   /// starting with "Dest", constructing elements into it as needed.
   template<typename It1, typename It2>
   static void uninitializedCopy(It1 iter, It1 end, It2 dest)
   {
      // Arbitrary iterator types; just use the basic implementation.
      std::uninitialized_copy(iter, end, dest);
   }
   
   /// Copy the range [I, E) onto the uninitialized memory
   /// starting with "Dest", constructing elements into it as needed.
   template <typename T1, typename T2>
   static void uninitializedCopy(
         T1 *iter, T1 *end, T2 *dest,
         typename std::enable_if<std::is_same<typename std::remove_const<T1>::type,
         T2>::value>::type * = nullptr)
   {
      // Use memcpy for PODs iterated by pointers (which includes SmallVector
      // iterators): std::uninitialized_copy optimizes to memmove, but we can
      // use memcpy here. Note that iter and end are iterators and thus might be
      // invalid for memcpy if they are equal.
      if (iter != end) {
         memcpy(dest, iter, (end - iter) * sizeof(T));
      }
   }
   
   /// Double the size of the allocated memory, guaranteeing space for at
   /// least one more element or MinSize if specified.
   void grow(size_t minSize = 0)
   {
      growPod(minSize * sizeof(T), sizeof(T));
   }
   
public:
   void pushBack(const T &element)
   {
      if (POLAR_UNLIKELY(m_endX >= m_capacityX)) {
         grow();
      }
      memcpy(end(), &element, sizeof(T));
      setEnd(end() + 1);
   }
   
   inline void push_back(const T &element)
   {
      pushBack(element);
   }
   
   void popBack()
   {
      setEnd(end() - 1);
   }
   
   inline void pop_back()
   {
      popBack();
   }
};

/// This class consists of common code factored out of the SmallVector class to
/// reduce code duplication based on the SmallVector 'N' template parameter.
template <typename T>
class SmallVectorImpl : public SmallVectorTemplateBase<T, isPodLike<T>::value>
{
   using SuperClass = SmallVectorTemplateBase<T, isPodLike<T>::value>;
   
public:
   using iterator = typename SuperClass::iterator;
   using const_iterator = typename SuperClass::const_iterator;
   using size_type = typename SuperClass::size_type;
   
protected:
   // Default ctor - Initialize to empty.
   explicit SmallVectorImpl(unsigned N)
      : SmallVectorTemplateBase<T, isPodLike<T>::value>(N*sizeof(T))
   {
   }
   
public:
   SmallVectorImpl(const SmallVectorImpl &) = delete;
   
   ~SmallVectorImpl()
   {
      // Destroy the constructed elements in the vector.
      destroyRange(begin(), end());
      
      // If this wasn't grown from the inline copy, deallocate the old space.
      if (!isSmall()) {
         free(begin());
      }
   }
   
   void clear()
   {
      destroyRange(begin(), end());
      m_endX = m_beginX;
   }
   
   void resize(size_type size)
   {
      if (size < getSize()) {
         destroyRange(begin() + size, end());
         setEnd(begin() + size);
      } else if (size > getSize()) {
         if (getCapacity() < N) {
            grow(size);
         }
         for (auto iter = end(), end = begin() + size; iter != end; ++iter) {
            new (&*iter) T();
         }
         setEnd(begin() + size);
      }
   }
   
   void resize(size_type size, const T &newValue)
   {
      if (size < getSize()) {
         destroy_range(begin() + size, end());
         setEnd(begin() + size);
      } else if (size > getSize()) {
         if (getCapacity() < size) {
            grow(size);
         }
         std::uninitialized_fill(end(), begin() + size, newValue);
         setEnd(begin() + size);
      }
   }
   
   void reserve(size_type size)
   {
      if (getCapacity() < size) {
         grow(size);
      }    
   }
   
   POLAR_NODISCARD T popBackValue()
   {
      T result = ::std::move(getBack());
      popBack();
      return result;
   }
   
   void swap(SmallVectorImpl &rhs);
   
   /// Add the specified range to the end of the SmallVector.
   template <typename InputIter,
             typename = typename std::enable_if<std::is_convertible<
                                                   typename std::iterator_traits<InputIter>::iterator_category,
                                                   std::input_iterator_tag>::value>::type>
   void append(InputIter start, InputIter end)
   {
      size_type numInputs = std::distance(start, end);
      // Grow allocated space if needed.
      if (numInputs > size_type(getCapacityPtr() - this->end())) {
         grow(getSize() + numInputs);
      }
      // Copy the new elements over.
      uninitialized_copy(start, end, this->end());
      setEnd(this->end() + numInputs);
   }
   
   /// Add the specified range to the end of the SmallVector.
   void append(size_type numInputs, const T &element)
   {
      // Grow allocated space if needed.
      if (numInputs > size_type(getCapacityPtr() - end())) {
         grow(getSize() + numInputs);
      }
      // Copy the new elements over.
      std::uninitialized_fill_n(end(), numInputs, element);
      setEnd(end() + numInputs);
   }
   
   void append(std::initializer_list<T> elements)
   {
      append(elements.begin(), elements.end());
   }
   
   // FIXME: Consider assigning over existing elements, rather than clearing &
   // re-initializing them - for all assign(...) variants.
   
   void assign(size_type numElts, const T &element) {
      clear();
      if (getCapacity() < numElts) {
         grow(numElts);
      }
      setEnd(begin() + numElts);
      std::uninitialized_fill(begin(), end(), element);
   }
   
   template <typename InIter,
             typename = typename std::enable_if<std::is_convertible<
                                                   typename std::iterator_traits<InIter>::iterator_category,
                                                   std::input_iterator_tag>::value>::type>
   void assign(InIter start, InIter end)
   {
      clear();
      append(start, end);
   }
   
   void assign(std::initializer_list<T> elements)
   {
      clear();
      append(elements);
   }
   
   iterator erase(const_iterator citer)
   {
      // Just cast away constness because this is a non-const member function.
      iterator iter = const_cast<iterator>(citer);
      
      assert(iter >= begin() && "Iterator to erase is out of bounds.");
      assert(iter < end() && "Erasing at past-the-end iterator.");
      
      iterator removed = iter;
      // Shift all elts down one.
      std::move(iter + 1, end(), iter);
      // Drop the last elt.
      pop_back();
      return(removed);
   }
   
   iterator erase(const_iterator cstart, const_iterator cend) 
   {
      // Just cast away constness because this is a non-const member function.
      iterator startIter = const_cast<iterator>(cstart);
      iterator endIter = const_cast<iterator>(cend);
      
      assert(startIter >= begin() && "Range to erase is out of bounds.");
      assert(startIter <= endIter && "Trying to erase invalid range.");
      assert(endIter <= end() && "Trying to erase past the end.");
      
      iterator removed = startIter;
      // Shift all elts down.
      iterator iter = std::move(endIter, end(), startIter);
      // Drop the last elts.
      destroyRange(iter, end());
      setEnd(iter);
      return(removed);
   }
   
   iterator insert(iterator iter, T &&element)
   {
      if (iter == end()) {  // Important special case for empty vector.
         push_back(::std::move(element));
         return end() - 1;
      }
      
      assert(iter >= begin() && "Insertion iterator is out of bounds.");
      assert(iter <= end() && "Inserting past the end of the vector.");
      
      if (m_endX >= m_capacityX) {
         size_t EltNo = iter - begin();
         grow();
         iter = begin() + EltNo;
      }
      
      ::new ((void*) end()) T(::std::move(back()));
      // Push everything else over.
      std::move_backward(iter, end() - 1, end());
      setEnd(end() + 1);
      
      // If we just moved the element we're inserting, be sure to update
      // the reference.
      T *eltPtr = &element;
      if (iter <= eltPtr && eltPtr < m_endX) {
         ++eltPtr;
      }
      *iter = ::std::move(*eltPtr);
      return iter;
   }
   
   iterator insert(iterator iter, const T &element)
   {
      if (iter == end()) {  // Important special case for empty vector.
         push_back(element);
         return end() - 1;
      }
      
      assert(iter >= begin() && "Insertion iterator is out of bounds.");
      assert(iter <= end() && "Inserting past the end of the vector.");
      
      if (m_endX >= m_capacityX) {
         size_t eltNo = iter - begin();
         grow();
         iter = begin() + eltNo;
      }
      ::new ((void*) end()) T(std::move(back()));
      // Push everything else over.
      std::move_backward(iter, end() - 1, end());
      setEnd(end() + 1);
      
      // If we just moved the element we're inserting, be sure to update
      // the reference.
      const T *eltPtr = &element;
      if (iter <= eltPtr && eltPtr < m_endX) {
         ++eltPtr;
      }
      *iter = *eltPtr;
      return iter;
   }
   
   iterator insert(iterator iter, size_type numToInsert, const T &element)
   {
      // Convert iterator to elt# to avoid invalidating iterator when we reserve()
      size_t InsertElt = iter - begin();
      
      if (iter == end()) {  // Important special case for empty vector.
         append(numToInsert, element);
         return begin() + InsertElt;
      }
      
      assert(iter >= begin() && "Insertion iterator is out of bounds.");
      assert(iter <= end() && "Inserting past the end of the vector.");
      
      // Ensure there is enough space.
      reserve(getSize() + numToInsert);
      
      // Uninvalidate the iterator.
      iter = begin() + InsertElt;
      
      // If there are more elements between the insertion point and the end of the
      // range than there are being inserted, we can use a simple approach to
      // insertion.  Since we already reserved space, we know that this won't
      // reallocate the vector.
      if (size_t(end() - iter) >= numToInsert) {
         T *oldEnd = end();
         append(std::move_iterator<iterator>(end() - numToInsert),
                std::move_iterator<iterator>(end()));
         
         // Copy the existing elements that get replaced.
         std::move_backward(iter, oldEnd - numToInsert, oldEnd);
         
         std::fill_n(iter, numToInsert, element);
         return iter;
      }
      
      // Otherwise, we're inserting more elements than exist already, and we're
      // not inserting at the end.
      
      // Move over the elements that we're about to overwrite.
      T *oldEnd = end();
      setEnd(end() + numToInsert);
      size_t numOverwritten = oldEnd - iter;
      uninitializedMove(iter, oldEnd, end() - numOverwritten);
      
      // Replace the overwritten part.
      std::fill_n(I, numOverwritten, element);
      
      // Insert the non-overwritten middle part.
      std::uninitialized_fill_n(oldEnd, numToInsert - numOverwritten, element);
      return iter;
   }
   
   template <typename ItTy,
             typename = typename std::enable_if<std::is_convertible<
                                                   typename std::iterator_traits<ItTy>::iterator_category,
                                                   std::input_iterator_tag>::value>::type>
   iterator insert(iterator iter, ItTy from, ItTy to)
   {
      // Convert iterator to elt# to avoid invalidating iterator when we reserve()
      size_t insertElt = iter - begin();
      
      if (iter == end()) {  // Important special case for empty vector.
         append(from, to);
         return begin() + insertElt;
      }
      
      assert(iter >= begin() && "Insertion iterator is out of bounds.");
      assert(iter <= end() && "Inserting past the end of the vector.");
      
      size_t numToInsert = std::distance(from, to);
      
      // Ensure there is enough space.
      reserve(getSize() + numToInsert);
      
      // Uninvalidate the iterator.
      I = begin() + insertElt;
      
      // If there are more elements between the insertion point and the end of the
      // range than there are being inserted, we can use a simple approach to
      // insertion.  Since we already reserved space, we know that this won't
      // reallocate the vector.
      if (size_t(end() - iter) >= numToInsert) {
         T *oldEnd = end();
         append(std::move_iterator<iterator>(end() - numToInsert),
                std::move_iterator<iterator>(end()));
         
         // Copy the existing elements that get replaced.
         std::move_backward(iter, oldEnd - numToInsert, oldEnd);
         
         std::copy(from, to, iter);
         return iter;
      }
      
      // Otherwise, we're inserting more elements than exist already, and we're
      // not inserting at the end.
      
      // Move over the elements that we're about to overwrite.
      T *oldEnd = end();
      setEnd(end() + numToInsert);
      size_t numOverwritten = oldEnd - iter;
      uninitializedMove(iter, oldEnd, end() - numOverwritten);
      
      // Replace the overwritten part.
      for (T *targetIter = iter; numOverwritten > 0; --numOverwritten) {
         *targetIter = *from;
         ++targetIter;
         ++from;
      }
      
      // Insert the non-overwritten middle part.
      uninitializedCopy(from, to, oldEnd);
      return iter;
   }
   
   void insert(iterator iter, std::initializer_list<T> elements)
   {
      insert(iter, elements.begin(), elements.end());
   }
   
   template <typename... ArgTypes>
   void emplaceBack(ArgTypes &&... args)
   {
      if (POLAR_UNLIKELY(m_endX >= m_capacityX)) {
         grow();
      }
      ::new ((void *)end()) T(std::forward<ArgTypes>(args)...);
      setEnd(end() + 1);
   }
   
   template <typename... ArgTypes>
   inline void emplace_back(ArgTypes &&... args)
   {
      emplaceBack(std::forward<ArgTypes>(args)...);
   }
   
   SmallVectorImpl &operator=(const SmallVectorImpl &rhs);
   
   SmallVectorImpl &operator=(SmallVectorImpl &&rhs);
   
   bool operator==(const SmallVectorImpl &rhs) const
   {
      if (getSize() != rhs.getSize()) {
         return false;
      }
      return std::equal(begin(), end(), rhs.begin());
   }
   
   bool operator!=(const SmallVectorImpl &rhs) const
   {
      return !(*this == rhs);
   }
   
   bool operator<(const SmallVectorImpl &rhs) const
   {
      return std::lexicographical_compare(begin(), end(),
                                          rhs.begin(), rhs.end());
   }
   
   /// Set the array size to \p N, which the current array must have enough
   /// capacity for.
   ///
   /// This does not construct or destroy any elements in the vector.
   ///
   /// Clients can use this in conjunction with capacity() to write past the end
   /// of the buffer when they know that more elements are available, and only
   /// update the size later. This avoids the cost of value initializing elements
   /// which will only be overwritten.
   void setSize(size_type size)
   {
      assert(size <= capacity());
      setEnd(begin() + size);
   }
};

template <typename T>
void SmallVectorImpl<T>::swap(SmallVectorImpl<T> &rhs)
{
   if (this == &rhs) {
      return;
   }
   // We can only avoid copying elements if neither vector is small.
   if (!isSmall() && !rhs.isSmall()) {
      std::swap(m_beginX, rhs.m_beginX);
      std::swap(m_endX, rhs.m_endX);
      std::swap(m_capacityX, rhs.m_capacityX);
      return;
   }
   if (rhs.size() > getCapacity()) {
      grow(rhs.getSize());
   }
   if (size() > rhs.getCapacity()) {
      rhs.grow(getSize());
   }
   // Swap the shared elements.
   size_t numShared = getSize();
   if (numShared > rhs.getSize()) {
      numShared = rhs.getSize();
   }
   for (size_type i = 0; i != numShared; ++i) {
      std::swap((*this)[i], rhs[i]);
   }
   // Copy over the extra elts.
   if (getSize() > rhs.getSize()) {
      size_t eltDiff = getSize() - rhs.getSize();
      uninitializedCopy(begin() + numShared, end(), rhs.end());
      rhs.setEnd(rhs.end() + eltDiff);
      destroyRange(begin() + numShared, end());
      setEnd(begin() + numShared);
   } else if (rhs.getSize() > getSize()) {
      size_t eltDiff = rhs.getSize() - getSize();
      uninitializedCopy(rhs.begin() + numShared, rhs.end(), end());
      setEnd(end() + eltDiff);
      destroyRange(rhs.begin() + numShared, rhs.end());
      rhs.setEnd(rhs.begin() + numShared);
   }
}

template <typename T>
SmallVectorImpl<T> &SmallVectorImpl<T>::operator= (const SmallVectorImpl<T> &rhs)
{
   // Avoid self-assignment.
   if (this == &rhs) {
      return *this;
   }
   // If we already have sufficient space, assign the common elements, then
   // destroy any excess.
   size_t rhsSize = rhs.getSize();
   size_t curSize = getSize();
   if (curSize >= rhsSize) {
      // Assign common elements.
      iterator newEnd;
      if (rhsSize) {
         newEnd = std::copy(rhs.begin(), rhs.begin() + rhsSize, begin());
      } else {
         newEnd = begin();
      }
      // Destroy excess elements.
      destroyRange(newEnd, end());      
      // Trim.
      setEnd(newEnd);
      return *this;
   }
   
   // If we have to grow to have enough elements, destroy the current elements.
   // This allows us to avoid copying them during the grow.
   // FIXME: don't do this if they're efficiently moveable.
   if (getCapacity() < rhsSize) {
      // Destroy current elements.
      destroyRange(begin(), end());
      setEnd(begin());
      curSize = 0;
      grow(rhsSize);
   } else if (curSize) {
      // Otherwise, use assignment for the already-constructed elements.
      std::copy(rhs.begin(), rhs.begin() + curSize, begin());
   }
   
   // Copy construct the new elements in place.
   uninitializedCopy(rhs.begin() + curSize, rhs.end(),
                     begin() + curSize);
   
   // Set end.
   setEnd(begin() + rhsSize);
   return *this;
}

template <typename T>
SmallVectorImpl<T> &SmallVectorImpl<T>::operator=(SmallVectorImpl<T> &&rhs)
{
   // Avoid self-assignment.
   if (this == &rhs) {
      return *this;
   }
   // If the RHS isn't small, clear this vector and then steal its buffer.
   if (!rhs.isSmall()) {
      destroyRange(begin(), end());
      if (!isSmall()) {
         free(begin());
      }
      m_beginX = rhs.m_beginX;
      m_endX = rhs.m_endX;
      m_capacityX = rhs.m_capacityX;
      rhs.resetToSmall();
      return *this;
   }
   
   // If we already have sufficient space, assign the common elements, then
   // destroy any excess.
   size_t rhsSize = rhs.getSize();
   size_t curSize = getSize();
   if (curSize >= rhsSize) {
      // Assign common elements.
      iterator newEnd = begin();
      if (rhsSize) {
         newEnd = std::move(rhs.begin(), rhs.end(), newEnd);
      }
      // Destroy excess elements and trim the bounds.
      destroyRange(newEnd, end());
      setEnd(newEnd);
      // Clear the RHS.
      rhs.clear();
      return *this;
   }
   
   // If we have to grow to have enough elements, destroy the current elements.
   // This allows us to avoid copying them during the grow.
   // FIXME: this may not actually make any sense if we can efficiently move
   // elements.
   if (getCapacity() < rhsSize) {
      // Destroy current elements.
      destroyRange(begin(), end());
      setEnd(begin());
      curSize = 0;
      grow(rhsSize);
   } else if (curSize) {
      // Otherwise, use assignment for the already-constructed elements.
      std::move(rhs.begin(), rhs.begin() + curSize, begin());
   }
   
   // Move-construct the new elements in place.
   uninitializedMove(rhs.begin() + curSize, rhs.end(),
                     begin() + curSize);
   // Set end.
   setEnd(begin() + rhsSize);
   rhs.clear();
   return *this;
}

/// Storage for the SmallVector elements which aren't contained in
/// SmallVectorTemplateCommon. There are 'N-1' elements here. The remaining '1'
/// element is in the base class. This is specialized for the N=1 and N=0 cases
/// to avoid allocating unnecessary storage.
template <typename T, unsigned N>
struct SmallVectorStorage
{
   typename SmallVectorTemplateCommon<T>::U InlineElts[N - 1];
};

template <typename T> struct SmallVectorStorage<T, 1>
{};

template <typename T> struct SmallVectorStorage<T, 0>
{};

/// This is a 'vector' (really, a variable-sized array), optimized
/// for the case when the array is small.  It contains some number of elements
/// in-place, which allows it to avoid heap allocation when the actual number of
/// elements is below that threshold.  This allows normal "small" cases to be
/// fast without losing generality for large inputs.
///
/// Note that this does not attempt to be exception safe.
///
template <typename T, unsigned N>
class SmallVector : public SmallVectorImpl<T>
{
   /// Inline space for elements which aren't stored in the base class.
   SmallVectorStorage<T, N> m_storage;
   
public:
   SmallVector() : SmallVectorImpl<T>(N)
   {}
   
   explicit SmallVector(size_t size, const T &value = T())
      : SmallVectorImpl<T>(N)
   {
      assign(size, value);
   }
   
   template <typename ItTy,
             typename = typename std::enable_if<std::is_convertible<
                                                   typename std::iterator_traits<ItTy>::iterator_category,
                                                   std::input_iterator_tag>::value>::type>
   SmallVector(ItTy start, ItTy end) : SmallVectorImpl<T>(N)
   {
      append(start, end);
   }
   
   template <typename RangeTy>
   explicit SmallVector(const IteratorRange<RangeTy> &range)
      : SmallVectorImpl<T>(N)
   {
      append(range.begin(), range.end());
   }
   
   SmallVector(std::initializer_list<T> elements) : SmallVectorImpl<T>(N)
   {
      assign(elements);
   }
   
   SmallVector(const SmallVector &rhs) : SmallVectorImpl<T>(N)
   {
      if (!rhs.empty()) {
         SmallVectorImpl<T>::operator=(rhs);
      }
   }
   
   const SmallVector &operator=(const SmallVector &rhs)
   {
      SmallVectorImpl<T>::operator=(rhs);
      return *this;
   }
   
   SmallVector(SmallVector &&rhs) : SmallVectorImpl<T>(N) {
      if (!rhs.empty()) {
         SmallVectorImpl<T>::operator=(::std::move(rhs));
      }
   }
   
   SmallVector(SmallVectorImpl<T> &&rhs) : SmallVectorImpl<T>(N)
   {
      if (!rhs.empty())
         SmallVectorImpl<T>::operator=(::std::move(rhs));
   }
   
   const SmallVector &operator=(SmallVector &&rhs)
   {
      SmallVectorImpl<T>::operator=(::std::move(rhs));
      return *this;
   }
   
   const SmallVector &operator=(SmallVectorImpl<T> &&rhs)
   {
      SmallVectorImpl<T>::operator=(::std::move(rhs));
      return *this;
   }
   
   const SmallVector &operator=(std::initializer_list<T> elements)
   {
      assign(elements);
      return *this;
   }
};

template <typename T, unsigned N>
inline size_t capacity_in_bytes(const SmallVector<T, N> &vector)
{
   return vector.getCapacityInBytes();
}

} // basic
} // polar

namespace std {

/// Implement std::swap in terms of SmallVector swap.
template<typename T>
inline void
swap(::polar::basic::SmallVectorImpl<T> &lhs, ::polar::basic::SmallVectorImpl<T> &rhs)
{
   lhs.swap(rhs);
}

/// Implement std::swap in terms of SmallVector swap.
template<typename T, unsigned N>
inline void
swap(::polar::basic::SmallVector<T, N> &lhs, ::polar::basic::SmallVector<T, N> &rhs)
{
   lhs.swap(rhs);
}

} // end namespace std

#endif // POLAR_BASIC_ADT_SMALL_VECTOR_H
