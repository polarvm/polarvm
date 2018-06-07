// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
// 
// Created by softboy on 2018/05/31.

#ifndef POLAR_UTILS_ERROR_OPTIONAL_ERROR_H
#define POLAR_UTILS_ERROR_OPTIONAL_ERROR_H

#include "polar/utils/AlignOf.h"
#include <cassert>
#include <system_error>
#include <type_traits>
#include <utility>

namespace polar {
namespace utils {

/// \brief Stores a reference that can be changed.
template <typename T>
class ReferenceStorage
{
   T *m_storage;
   
public:
   ReferenceStorage(T &ref) : m_storage(&ref)
   {}
   
   operator T &() const 
   {
      return *m_storage;
   }
   
   T &get() const
   {
      return *m_storage;
   }
};

/// \brief Represents either an error or a value T.
///
/// OptionalError<T> is a pointer-like class that represents the result of an
/// operation. The result is either an error, or a value of type T. This is
/// designed to emulate the usage of returning a pointer where nullptr indicates
/// failure. However instead of just knowing that the operation failed, we also
/// have an error_code and optional user data that describes why it failed.
///
/// It is used like the following.
/// \code
///   OptionalError<Buffer> getBuffer();
///
///   auto buffer = getBuffer();
///   if (error_code ec = buffer.getError())
///     return ec;
///   buffer->write("adena");
/// \endcode
///
///
/// Implicit conversion to bool returns true if there is a usable value. The
/// unary * and -> operators provide pointer like access to the value. Accessing
/// the value when there is an error has undefined behavior.
///
/// When T is a reference type the behavior is slightly different. The reference
/// is held in a std::reference_wrapper<std::remove_reference<T>::type>, and
/// there is special handling to make operator -> work as if T was not a
/// reference.
///
/// T cannot be a rvalue reference.
template<class T>
class OptionalError
{
   template <class OtherType> friend class OptionalError;
   
   static const bool isRef = std::is_reference<T>::value;
   
   using wrap = ReferenceStorage<typename std::remove_reference<T>::type>;
   
public:
   using storage_type = typename std::conditional<isRef, wrap, T>::type;
   
private:
   using reference = typename std::remove_reference<T>::type &;
   using const_reference = const typename std::remove_reference<T>::type &;
   using pointer = typename std::remove_reference<T>::type *;
   using const_pointer = const typename std::remove_reference<T>::type *;
   
public:
   template <class E>
   OptionalError(E ErrorCode,
                 typename std::enable_if<std::is_error_code_enum<E>::value ||
                 std::is_error_condition_enum<E>::value,
                 void *>::type = nullptr)
      : m_hasError(true)
   {
      new (getErrorStorage()) std::error_code(make_error_code(ErrorCode));
   }
   
   OptionalError(std::error_code errorCode) : m_hasError(true)
   {
      new (getErrorStorage()) std::error_code(errorCode);
   }
   
   template <class OtherType>
   OptionalError(OtherType &&value,
                 typename std::enable_if<std::is_convertible<OtherType, T>::value>::type
                 * = nullptr)
      : m_hasError(false)
   {
      new (getStorage()) storage_type(std::forward<OtherType>(value));
   }
   
   OptionalError(const OptionalError &other)
   {
      copyConstruct(other);
   }
   
   template <class OtherType>
   OptionalError(
         const OptionalError<OtherType> &other,
         typename std::enable_if<std::is_convertible<OtherType, T>::value>::type * =
         nullptr)
   {
      copyConstruct(other);
   }
   
   template <class OtherType>
   explicit OptionalError(
         const OptionalError<OtherType> &other,
         typename std::enable_if<
         !std::is_convertible<OtherType, const T &>::value>::type * = nullptr)
   {
      copyConstruct(other);
   }
   
   OptionalError(OptionalError &&other)
   {
      moveConstruct(std::move(other));
   }
   
   template <class OtherType>
   OptionalError(
         OptionalError<OtherType> &&other,
         typename std::enable_if<std::is_convertible<OtherType, T>::value>::type * =
         nullptr)
   {
      moveConstruct(std::move(other));
   }
   
   // This might eventually need SFINAE but it's more complex than is_convertible
   // & I'm too lazy to write it right now.
   template <class OtherType>
   explicit OptionalError(
         OptionalError<OtherType> &&other,
         typename std::enable_if<!std::is_convertible<OtherType, T>::value>::type * =
         nullptr)
   {
      moveConstruct(std::move(other));
   }
   
   OptionalError &operator=(const OptionalError &other)
   {
      copyAssign(other);
      return *this;
   }
   
   OptionalError &operator=(OptionalError &&other)
   {
      moveAssign(std::move(other));
      return *this;
   }
   
   ~OptionalError()
   {
      if (!m_hasError) {
         getStorage()->~storage_type();
      }
   }
   
   /// \brief Return false if there is an error.
   explicit operator bool() const
   {
      return !m_hasError;
   }
   
   reference get()
   {
      return *getStorage();
   }
   
   const_reference get() const
   {
      return const_cast<OptionalError<T> *>(this)->get();
   }
   
   std::error_code getError() const
   {
      return m_hasError ? *getErrorStorage() : std::error_code();
   }
   
   pointer operator ->()
   {
      return toPointer(getStorage());
   }
   
   const_pointer operator->() const
   {
      return toPointer(getStorage());
   }
   
   reference operator *()
   {
      return *getStorage();
   }
   
   const_reference operator*() const
   {
      return *getStorage();
   }
   
private:
   template <class OtherType>
   void copyConstruct(const OptionalError<OtherType> &other)
   {
      if (!other.m_hasError) {
         // Get the other value.
         m_hasError = false;
         new (getStorage()) storage_type(*other.getStorage());
      } else {
         // Get other's error.
         m_hasError = true;
         new (getErrorStorage()) std::error_code(Other.getError());
      }
   }
   
   template <class T1>
   static bool compareThisIfSameType(const T1 &lhs, const T1 &rhs)
   {
      return &lhs == &rhs;
   }
   
   template <class T1, class T2>
   static bool compareThisIfSameType(const T1 &lhs, const T2 &rhs)
   {
      return false;
   }
   
   template <class OtherType>
   void copyAssign(const OptionalError<OtherType> &other)
   {
      if (compareThisIfSameType(*this, other)) {
         return;
      }
      this->~OptionalError();
      new (this) OptionalError(other);
   }
   
   template <class OtherType>
   void moveConstruct(OptionalError<OtherType> &&other)
   {
      if (!Other.m_hasError) {
         // Get the other value.
         m_hasError = false;
         new (getStorage()) storage_type(std::move(*other.getStorage()));
      } else {
         // Get other's error.
         m_hasError = true;
         new (getErrorStorage()) std::error_code(other.getError());
      }
   }
   
   template <class OtherType>
   void moveAssign(OptionalError<OtherType> &&other)
   {
      if (compareThisIfSameType(*this, other)) {
         return;
      }
      this->~OptionalError();
      new (this) OptionalError(std::move(other));
   }
   
   pointer toPointer(pointer value)
   {
      return value;
   }
   
   const_pointer toPointer(const_pointer value) const
   {
      return value;
   }
   
   pointer toPointer(wrap *value)
   {
      return &value->get();
   }
   
   const_pointer toPointer(const wrap *Val) const
   {
      return &Val->get();
   }
   
   storage_type *getStorage()
   {
      assert(!m_hasError && "Cannot get value when an error exists!");
      return reinterpret_cast<storage_type*>(m_tstorage.m_buffer);
   }
   
   const storage_type *getStorage() const
   {
      assert(!m_hasError && "Cannot get value when an error exists!");
      return reinterpret_cast<const storage_type*>(m_tstorage.m_buffer);
   }
   
   std::error_code *getErrorStorage()
   {
      assert(m_hasError && "Cannot get error when a value exists!");
      return reinterpret_cast<std::error_code *>(m_errorStorage.m_buffer);
   }
   
   const std::error_code *getErrorStorage() const
   {
      return const_cast<OptionalError<T> *>(this)->getErrorStorage();
   }
   
   union {
      AlignedCharArrayUnion<storage_type> m_tstorage;
      AlignedCharArrayUnion<std::error_code> m_errorStorage;
   };
   bool m_hasError : 1;
};

template <class T, class E>
typename std::enable_if<std::is_error_code_enum<E>::value ||
std::is_error_condition_enum<E>::value,
bool>::type
operator==(const OptionalError<T> &error, E code)
{
   return error.getError() == code;
}

} // utils
} // polar

#endif // POLAR_UTILS_ERROR_OPTIONAL_ERROR_H
