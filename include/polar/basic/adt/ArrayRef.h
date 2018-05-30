// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
// 
// Created by softboy on 2018/05/29.

#ifndef POLAR_BASIC_ADT_ARRAY_REF_H
#define POLAR_BASIC_ADT_ARRAY_REF_H

#include "polar/basic/adt/Hashing.h"
#include "polar/basic/adt/None.h"
#include "polar/basic/adt/SmallVector.h"
#include "polar/basic/adt/StlExtras.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>
#include <vector>

namespace polar {
namespace basic {

/// ArrayRef - Represent a constant reference to an array (0 or more elements
/// consecutively in memory), i.e. a start pointer and a length.  It allows
/// various APIs to take consecutive elements easily and conveniently.
///
/// This class does not own the underlying data, it is expected to be used in
/// situations where the data resides in some other buffer, whose lifetime
/// extends past that of the ArrayRef. For this reason, it is not in general
/// safe to store an ArrayRef.
///
/// This is intended to be trivially copyable, so it should be passed by
/// value.
template<typename T>
class POLAR_NODISCARD ArrayRef
{
public:
   using iterator = const T *;
   using const_iterator = const T *;
   using size_type = size_t;
   using reverse_iterator = std::reverse_iterator<iterator>;
   
private:
   /// The start of the array, in an external buffer.
   const T *m_data = nullptr;
   
   /// The number of elements.
   size_type m_length = 0;
   
public:
   /// @name Constructors
   /// @{
   
   /// Construct an empty ArrayRef.
   /*implicit*/ ArrayRef() = default;
   
   /// Construct an empty ArrayRef from None.
   /*implicit*/ ArrayRef(NoneType)
   {}
   
   /// Construct an ArrayRef from a single element.
   /*implicit*/ ArrayRef(const T &OneElt)
      : Data(&OneElt), Length(1)
   {}
   
   /// Construct an ArrayRef from a pointer and length.
   /*implicit*/ ArrayRef(const T *data, size_t length)
      : m_data(data), m_length(length) {}
   
   /// Construct an ArrayRef from a range.
   ArrayRef(const T *begin, const T *end)
      : m_data(begin), m_length(end - begin)
   {}
   
   /// Construct an ArrayRef from a SmallVector. This is templated in order to
   /// avoid instantiating SmallVectorTemplateCommon<T> whenever we
   /// copy-construct an ArrayRef.
   template<typename U>
   /*implicit*/ ArrayRef(const SmallVectorTemplateCommon<T, U> &vector)
      : m_data(vector.getData()), Length(vector.getSize())
   {}
   
   /// Construct an ArrayRef from a std::vector.
   template<typename A>
   /*implicit*/ ArrayRef(const std::vector<T, A> &vector)
      : m_data(vector.data()), m_length(vector.size())
   {}
   
   /// Construct an ArrayRef from a std::array
   template <size_t N>
   /*implicit*/ constexpr ArrayRef(const std::array<T, N> &array)
      : m_data(array.data()), m_length(N)
   {}
   
   /// Construct an ArrayRef from a C array.
   template <size_t N>
   /*implicit*/ constexpr ArrayRef(const T (&array)[N]) 
      : m_data(array), m_length(N)
   {}
   
   /// Construct an ArrayRef from a std::initializer_list.
   /*implicit*/ ArrayRef(const std::initializer_list<T> &vector)
      : m_data(vector.begin() == vector.end() ? (T*)nullptr : vector.begin()),
        m_length(vector.size())
   {}
   
   /// Construct an ArrayRef<const T*> from ArrayRef<T*>. This uses SFINAE to
   /// ensure that only ArrayRefs of pointers can be converted.
   template <typename U>
   ArrayRef(
         const ArrayRef<U *> &array,
         typename std::enable_if<
         std::is_convertible<U *const *, T const *>::value>::type * = nullptr)
      : m_data(array.getData()), m_length(array.getSize()) {}
   
   /// Construct an ArrayRef<const T*> from a SmallVector<T*>. This is
   /// templated in order to avoid instantiating SmallVectorTemplateCommon<T>
   /// whenever we copy-construct an ArrayRef.
   template<typename U, typename DummyT>
   /*implicit*/ ArrayRef(
         const SmallVectorTemplateCommon<U *, DummyT> &vector,
         typename std::enable_if<
         std::is_convertible<U *const *, T const *>::value>::type * = nullptr)
      : m_data(vector.getData()), m_length(vector.getSize())
   {}
   
   /// Construct an ArrayRef<const T*> from std::vector<T*>. This uses SFINAE
   /// to ensure that only vectors of pointers can be converted.
   template<typename U, typename A>
   ArrayRef(const std::vector<U *, A> &vector,
            typename std::enable_if<
            std::is_convertible<U *const *, T const *>::value>::type* = nullptr)
      : m_data(vector.getData()), m_length(vector.getSize()) {}
   
   /// @}
   /// @name Simple Operations
   /// @{
   
   iterator begin() const
   {
      return m_data;
   }
   
   iterator end() const
   {
      return m_data + m_length;
   }
   
   reverse_iterator rbegin() const
   {
      return reverse_iterator(end());
   }
   
   reverse_iterator rend() const
   {
      return reverse_iterator(begin());
   }
   
   /// empty - Check if the array is empty.
   bool empty() const
   {
      return m_length == 0;
   }
   
   const T *getData() const
   {
      return m_data;
   }
   
   /// size - Get the array size.
   size_t getSize() const
   {
      return m_length;
   }
   
   /// front - Get the first element.
   const T &getFront() const
   {
      assert(!empty());
      return m_data[0];
   }
   
   /// back - Get the last element.
   const T &getBack() const
   {
      assert(!empty());
      return m_data[Length-1];
   }
   
   const T &front() const
   {
      return getFront();
   }
   
   /// back - Get the last element.
   const T &back() const
   {
      return getBack();
   }
      
   // copy - Allocate copy in Allocator and return ArrayRef<T> to it.
   template <typename Allocator> ArrayRef<T> copy(Allocator &allocator)
   {
      T *buffer = allocator.template Allocate<T>(m_length);
      std::uninitialized_copy(begin(), end(), buffer);
      return ArrayRef<T>(buffer, m_length);
   }
   
   /// equals - Check for element-wise equality.
   bool equals(ArrayRef rhs) const
   {
      if (m_length != rhs.m_length) {
         return false;
      }
      return std::equal(begin(), end(), rhs.begin());
   }
   
   /// slice(n, m) - Chop off the first N elements of the array, and keep M
   /// elements in the array.
   ArrayRef<T> slice(size_t start, size_t size) const
   {
      assert(start + size <= getSize() && "Invalid specifier");
      return ArrayRef<T>(getData() + start, size);
   }
   
   /// slice(n) - Chop off the first N elements of the array.
   ArrayRef<T> slice(size_t size) const
   {
      return slice(size, getSize() - size);
   }
   
   /// \brief Drop the first \p N elements of the array.
   ArrayRef<T> dropFront(size_t size = 1) const
   {
      assert(getSize() >= size && "Dropping more elements than exist");
      return slice(size, getSize() - size);
   }
   
   /// \brief Drop the last \p N elements of the array.
   ArrayRef<T> drop_back(size_t size = 1) const
   {
      assert(getSize() >= N && "Dropping more elements than exist");
      return slice(0, getSize() - size);
   }
   
   /// \brief Return a copy of *this with the first N elements satisfying the
   /// given predicate removed.
   template <class PredicateT> ArrayRef<T> dropWhile(PredicateT pred) const
   {
      return ArrayRef<T>(Utils::find_last_set(*this, pred), end());
   }
   
   /// \brief Return a copy of *this with the first N elements not satisfying
   /// the given predicate removed.
   template <class PredicateT> ArrayRef<T> drop_until(PredicateT Pred) const {
      return ArrayRef<T>(find_if(*this, Pred), end());
   }
   
   /// \brief Return a copy of *this with only the first \p N elements.
   ArrayRef<T> take_front(size_t N = 1) const {
      if (N >= size())
         return *this;
      return drop_back(size() - N);
   }
   
   /// \brief Return a copy of *this with only the last \p N elements.
   ArrayRef<T> take_back(size_t N = 1) const {
      if (N >= size())
         return *this;
      return drop_front(size() - N);
   }
   
   /// \brief Return the first N elements of this Array that satisfy the given
   /// predicate.
   template <class PredicateT> ArrayRef<T> take_while(PredicateT Pred) const {
      return ArrayRef<T>(begin(), find_if_not(*this, Pred));
   }
   
   /// \brief Return the first N elements of this Array that don't satisfy the
   /// given predicate.
   template <class PredicateT> ArrayRef<T> take_until(PredicateT Pred) const {
      return ArrayRef<T>(begin(), find_if(*this, Pred));
   }
   
   /// @}
   /// @name Operator Overloads
   /// @{
   const T &operator[](size_t Index) const {
      assert(Index < Length && "Invalid index!");
      return Data[Index];
   }
   
   /// Disallow accidental assignment from a temporary.
   ///
   /// The declaration here is extra complicated so that "arrayRef = {}"
   /// continues to select the move assignment operator.
   template <typename U>
   typename std::enable_if<std::is_same<U, T>::value, ArrayRef<T>>::type &
   operator=(U &&Temporary) = delete;
   
   /// Disallow accidental assignment from a temporary.
   ///
   /// The declaration here is extra complicated so that "arrayRef = {}"
   /// continues to select the move assignment operator.
   template <typename U>
   typename std::enable_if<std::is_same<U, T>::value, ArrayRef<T>>::type &
   operator=(std::initializer_list<U>) = delete;
   
   /// @}
   /// @name Expensive Operations
   /// @{
   std::vector<T> vec() const {
      return std::vector<T>(Data, Data+Length);
   }
   
   /// @}
   /// @name Conversion operators
   /// @{
   operator std::vector<T>() const {
      return std::vector<T>(Data, Data+Length);
   }
   
   /// @}
};

/// MutableArrayRef - Represent a mutable reference to an array (0 or more
/// elements consecutively in memory), i.e. a start pointer and a length.  It
/// allows various APIs to take and modify consecutive elements easily and
/// conveniently.
///
/// This class does not own the underlying data, it is expected to be used in
/// situations where the data resides in some other buffer, whose lifetime
/// extends past that of the MutableArrayRef. For this reason, it is not in
/// general safe to store a MutableArrayRef.
///
/// This is intended to be trivially copyable, so it should be passed by
/// value.
template<typename T>
class POLAR_NODISCARD MutableArrayRef : public ArrayRef<T> {
public:
   using iterator = T *;
   using reverse_iterator = std::reverse_iterator<iterator>;
   
   /// Construct an empty MutableArrayRef.
   /*implicit*/ MutableArrayRef() = default;
   
   /// Construct an empty MutableArrayRef from None.
   /*implicit*/ MutableArrayRef(NoneType) : ArrayRef<T>() {}
   
   /// Construct an MutableArrayRef from a single element.
   /*implicit*/ MutableArrayRef(T &OneElt) : ArrayRef<T>(OneElt) {}
   
   /// Construct an MutableArrayRef from a pointer and length.
   /*implicit*/ MutableArrayRef(T *data, size_t length)
      : ArrayRef<T>(data, length) {}
   
   /// Construct an MutableArrayRef from a range.
   MutableArrayRef(T *begin, T *end) : ArrayRef<T>(begin, end) {}
   
   /// Construct an MutableArrayRef from a SmallVector.
   /*implicit*/ MutableArrayRef(SmallVectorImpl<T> &Vec)
      : ArrayRef<T>(Vec) {}
   
   /// Construct a MutableArrayRef from a std::vector.
   /*implicit*/ MutableArrayRef(std::vector<T> &Vec)
      : ArrayRef<T>(Vec) {}
   
   /// Construct an ArrayRef from a std::array
   template <size_t N>
   /*implicit*/ constexpr MutableArrayRef(std::array<T, N> &Arr)
      : ArrayRef<T>(Arr) {}
   
   /// Construct an MutableArrayRef from a C array.
   template <size_t N>
   /*implicit*/ constexpr MutableArrayRef(T (&Arr)[N]) : ArrayRef<T>(Arr) {}
   
   T *data() const { return const_cast<T*>(ArrayRef<T>::data()); }
   
   iterator begin() const { return data(); }
   iterator end() const { return data() + this->size(); }
   
   reverse_iterator rbegin() const { return reverse_iterator(end()); }
   reverse_iterator rend() const { return reverse_iterator(begin()); }
   
   /// front - Get the first element.
   T &front() const {
      assert(!this->empty());
      return data()[0];
   }
   
   /// back - Get the last element.
   T &back() const {
      assert(!this->empty());
      return data()[this->size()-1];
   }
   
   /// slice(n, m) - Chop off the first N elements of the array, and keep M
   /// elements in the array.
   MutableArrayRef<T> slice(size_t N, size_t M) const {
      assert(N + M <= this->size() && "Invalid specifier");
      return MutableArrayRef<T>(this->data() + N, M);
   }
   
   /// slice(n) - Chop off the first N elements of the array.
   MutableArrayRef<T> slice(size_t N) const {
      return slice(N, this->size() - N);
   }
   
   /// \brief Drop the first \p N elements of the array.
   MutableArrayRef<T> drop_front(size_t N = 1) const {
      assert(this->size() >= N && "Dropping more elements than exist");
      return slice(N, this->size() - N);
   }
   
   MutableArrayRef<T> drop_back(size_t N = 1) const {
      assert(this->size() >= N && "Dropping more elements than exist");
      return slice(0, this->size() - N);
   }
   
   /// \brief Return a copy of *this with the first N elements satisfying the
   /// given predicate removed.
   template <class PredicateT>
   MutableArrayRef<T> drop_while(PredicateT Pred) const {
      return MutableArrayRef<T>(find_if_not(*this, Pred), end());
   }
   
   /// \brief Return a copy of *this with the first N elements not satisfying
   /// the given predicate removed.
   template <class PredicateT>
   MutableArrayRef<T> drop_until(PredicateT Pred) const {
      return MutableArrayRef<T>(find_if(*this, Pred), end());
   }
   
   /// \brief Return a copy of *this with only the first \p N elements.
   MutableArrayRef<T> take_front(size_t N = 1) const {
      if (N >= this->size())
         return *this;
      return drop_back(this->size() - N);
   }
   
   /// \brief Return a copy of *this with only the last \p N elements.
   MutableArrayRef<T> take_back(size_t N = 1) const {
      if (N >= this->size())
         return *this;
      return drop_front(this->size() - N);
   }
   
   /// \brief Return the first N elements of this Array that satisfy the given
   /// predicate.
   template <class PredicateT>
   MutableArrayRef<T> take_while(PredicateT Pred) const {
      return MutableArrayRef<T>(begin(), find_if_not(*this, Pred));
   }
   
   /// \brief Return the first N elements of this Array that don't satisfy the
   /// given predicate.
   template <class PredicateT>
   MutableArrayRef<T> take_until(PredicateT Pred) const {
      return MutableArrayRef<T>(begin(), find_if(*this, Pred));
   }
   
   /// @}
   /// @name Operator Overloads
   /// @{
   T &operator[](size_t Index) const {
      assert(Index < this->size() && "Invalid index!");
      return data()[Index];
   }
};

/// This is a MutableArrayRef that owns its array.
template <typename T> class OwningArrayRef : public MutableArrayRef<T> {
public:
   OwningArrayRef() = default;
   OwningArrayRef(size_t Size) : MutableArrayRef<T>(new T[Size], Size) {}
   
   OwningArrayRef(ArrayRef<T> Data)
      : MutableArrayRef<T>(new T[Data.size()], Data.size()) {
      std::copy(Data.begin(), Data.end(), this->begin());
   }
   
   OwningArrayRef(OwningArrayRef &&Other) { *this = Other; }
   
   OwningArrayRef &operator=(OwningArrayRef &&Other) {
      delete[] this->data();
      this->MutableArrayRef<T>::operator=(Other);
      Other.MutableArrayRef<T>::operator=(MutableArrayRef<T>());
      return *this;
   }
   
   ~OwningArrayRef() { delete[] this->data(); }
};

/// @name ArrayRef Convenience constructors
/// @{

/// Construct an ArrayRef from a single element.
template<typename T>
ArrayRef<T> makeArrayRef(const T &OneElt) {
   return OneElt;
}

/// Construct an ArrayRef from a pointer and length.
template<typename T>
ArrayRef<T> makeArrayRef(const T *data, size_t length) {
   return ArrayRef<T>(data, length);
}

/// Construct an ArrayRef from a range.
template<typename T>
ArrayRef<T> makeArrayRef(const T *begin, const T *end) {
   return ArrayRef<T>(begin, end);
}

/// Construct an ArrayRef from a SmallVector.
template <typename T>
ArrayRef<T> makeArrayRef(const SmallVectorImpl<T> &Vec) {
   return Vec;
}

/// Construct an ArrayRef from a SmallVector.
template <typename T, unsigned N>
ArrayRef<T> makeArrayRef(const SmallVector<T, N> &Vec) {
   return Vec;
}

/// Construct an ArrayRef from a std::vector.
template<typename T>
ArrayRef<T> makeArrayRef(const std::vector<T> &Vec) {
   return Vec;
}

/// Construct an ArrayRef from an ArrayRef (no-op) (const)
template <typename T> ArrayRef<T> makeArrayRef(const ArrayRef<T> &Vec) {
   return Vec;
}

/// Construct an ArrayRef from an ArrayRef (no-op)
template <typename T> ArrayRef<T> &makeArrayRef(ArrayRef<T> &Vec) {
   return Vec;
}

/// Construct an ArrayRef from a C array.
template<typename T, size_t N>
ArrayRef<T> makeArrayRef(const T (&Arr)[N]) {
   return ArrayRef<T>(Arr);
}

/// Construct a MutableArrayRef from a single element.
template<typename T>
MutableArrayRef<T> makeMutableArrayRef(T &OneElt) {
   return OneElt;
}

/// Construct a MutableArrayRef from a pointer and length.
template<typename T>
MutableArrayRef<T> makeMutableArrayRef(T *data, size_t length) {
   return MutableArrayRef<T>(data, length);
}

/// @}
/// @name ArrayRef Comparison Operators
/// @{

template<typename T>
inline bool operator==(ArrayRef<T> LHS, ArrayRef<T> RHS) {
   return LHS.equals(RHS);
}

template<typename T>
inline bool operator!=(ArrayRef<T> LHS, ArrayRef<T> RHS) {
   return !(LHS == RHS);
}

/// @}

// ArrayRefs can be treated like a POD type.
template <typename T> struct isPodLike;
template <typename T> struct isPodLike<ArrayRef<T>> {
   static const bool value = true;
};

template <typename T> hash_code hash_value(ArrayRef<T> S) {
   return hash_combine_range(S.begin(), S.end());
}

} // basic
} // polar

#endif // POLAR_BASIC_ADT_ARRAY_REF_H
