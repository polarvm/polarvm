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

#ifndef POLAR_UTILS_ERROR_ERROR_TYPE_H
#define POLAR_UTILS_ERROR_ERROR_TYPE_H

#include "polar/basic/adt/SmallVector.h"
#include "polar/basic/adt/StlExtras.h"
#include "polar/basic/adt/StringExtras.h"
#include "polar/basic/adt/Twine.h"
// unittest mark
// #include "polar/global/AbiBreaking.h"
#include "polar/utils/AlignOf.h"
#include "polar/utils/Debug.h"
#include "polar/utils/ErrorHandling.h"
#include "polar/utils/OptionalError.h"
#include "polar/utils/RawOutStream.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <memory>
#include <new>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

namespace polar {
namespace utils {

class ErrorSuccess;

/// Base class for error info classes. Do not extend this directly: Extend
/// the ErrorInfo template subclass instead.
class ErrorInfoBase
{
public:
   virtual ~ErrorInfoBase() = default;

   /// Print an error message to an output stream.
   virtual void log(RawOutStream &OS) const = 0;

   /// Return the error message as a string.
   virtual std::string message() const
   {
      std::string msg;
      RawStringOutStream outStream(msg);
      log(outStream);
      return outStream.getStr();
   }

   /// Convert this error to a std::error_code.
   ///
   /// This is a temporary crutch to enable interaction with code still
   /// using std::error_code. It will be removed in the future.
   virtual std::error_code convertToErrorCode() const = 0;

   // Returns the class ID for this type.
   static const void *getClassId()
   {
      return &m_id;
   }

   // Returns the class ID for the dynamic type of this ErrorInfoBase instance.
   virtual const void *getDynamicClassId() const = 0;

   // Check whether this instance is a subclass of the class identified by
   // ClassID.
   virtual bool isA(const void *const classId) const
   {
      return classId == getClassId();
   }

   // Check whether this instance is a subclass of ErrorInfoType.
   template <typename ErrorInfoType> bool isA() const
   {
      return isA(ErrorInfoType::getClassId());
   }

private:
   virtual void anchor();

   static char m_id;
};

/// Lightweight error class with error context and mandatory checking.
///
/// Instances of this class wrap a ErrorInfoBase pointer. Failure states
/// are represented by setting the pointer to a ErrorInfoBase subclass
/// instance containing information describing the failure. Success is
/// represented by a null pointer value.
///
/// Instances of Error also contains a 'Checked' flag, which must be set
/// before the destructor is called, otherwise the destructor will trigger a
/// runtime error. This enforces at runtime the requirement that all Error
/// instances be checked or returned to the caller.
///
/// There are two ways to set the checked flag, depending on what state the
/// Error instance is in. For Error instances indicating success, it
/// is sufficient to invoke the boolean conversion operator. E.g.:
///
///   @code{.cpp}
///   Error foo(<...>);
///
///   if (auto E = foo(<...>))
///     return E; // <- Return E if it is in the error state.
///   // We have verified that E was in the success state. It can now be safely
///   // destroyed.
///   @endcode
///
/// A success value *can not* be dropped. For example, just calling 'foo(<...>)'
/// without testing the return value will raise a runtime error, even if foo
/// returns success.
///
/// For Error instances representing failure, you must use either the
/// handle_errors or handle_all_errors function with a typed handler. E.g.:
///
///   @code{.cpp}
///   class MyErrorInfo : public ErrorInfo<MyErrorInfo> {
///     // Custom error info.
///   };
///
///   Error foo(<...>) { return make_error<MyErrorInfo>(...); }
///
///   auto E = foo(<...>); // <- foo returns failure with MyErrorInfo.
///   auto NewE =
///     handle_errors(E,
///       [](const MyErrorInfo &M) {
///         // Deal with the error.
///       },
///       [](std::unique_ptr<OtherError> M) -> Error {
///         if (canHandle(*M)) {
///           // handle error.
///           return Error::success();
///         }
///         // Couldn't handle this error instance. Pass it up the stack.
///         return Error(std::move(M));
///       );
///   // Note - we must check or return NewE in case any of the handlers
///   // returned a new error.
///   @endcode
///
/// The handle_all_errors function is identical to handle_errors, except
/// that it has a void return type, and requires all errors to be handled and
/// no new errors be returned. It prevents errors (assuming they can all be
/// handled) from having to be bubbled all the way to the top-level.
///
/// *All* Error instances must be checked before destruction, even if
/// they're moved-assigned or constructed from Success values that have already
/// been checked. This enforces checking through all levels of the call stack.
class POLAR_NODISCARD Error
{
   // ErrorList needs to be able to yank ErrorInfoBase pointers out of this
   // class to add to the error list.
   friend class ErrorList;

   // handle_errors needs to be able to set the Checked flag.
   template <typename... HandlerTs>
   friend Error handle_errors(Error error, HandlerTs &&... handlers);

   // Expected<T> needs to be able to steal the payload when constructed from an
   // error.
   template <typename T> friend class Expected;

protected:
   /// Create a success value. Prefer using 'Error::success()' for readability
   Error()
   {
      setPtr(nullptr);
      setChecked(false);
   }

public:
   /// Create a success value.
   static ErrorSuccess getSuccess();

   // Errors are not copy-constructable.
   Error(const Error &other) = delete;

   /// Move-construct an error value. The newly constructed error is considered
   /// unchecked, even if the source error had been checked. The original error
   /// becomes a checked Success value, regardless of its original state.
   Error(Error &&other)
   {
      setChecked(true);
      *this = std::move(other);
   }

   /// Create an error value. Prefer using the 'make_error' function, but
   /// this constructor can be useful when "re-throwing" errors from handlers.
   Error(std::unique_ptr<ErrorInfoBase> payload)
   {
      setPtr(payload.release());
      setChecked(false);
   }

   // Errors are not copy-assignable.
   Error &operator=(const Error &other) = delete;

   /// Move-assign an error value. The current error must represent success, you
   /// you cannot overwrite an unhandled error. The current error is then
   /// considered unchecked. The source error becomes a checked success value,
   /// regardless of its original state.
   Error &operator=(Error &&other)
   {
      // Don't allow overwriting of unchecked values.
      assertIsChecked();
      setPtr(other.getPtr());

      // This Error is unchecked, even if the source error was checked.
      setChecked(false);

      // Null out other's payload and set its checked bit.
      other.setPtr(nullptr);
      other.setChecked(true);

      return *this;
   }

   /// Destroy a Error. Fails with a call to abort() if the error is
   /// unchecked.
   ~Error()
   {
      assertIsChecked();
      delete getPtr();
   }

   /// Bool conversion. Returns true if this Error is in a failure state,
   /// and false if it is in an accept state. If the error is in a Success state
   /// it will be considered checked.
   explicit operator bool()
   {
      setChecked(getPtr() == nullptr);
      return getPtr() != nullptr;
   }

   /// Check whether one error is a subclass of another.
   template <typename ErrorType> bool isA() const
   {
      return getPtr() && getPtr()->isA(ErrorType::getClassId());
   }

   /// Returns the dynamic class id of this error, or null if this is a success
   /// value.
   const void* getDynamicClassId() const
   {
      if (!getPtr()) {
         return nullptr;
      }
      return getPtr()->getDynamicClassId();
   }

private:
#if POLAR_ENABLE_ABI_BREAKING_CHECKS
   // assertIsChecked() happens very frequently, but under normal circumstances
   // is supposed to be a no-op.  So we want it to be inlined, but having a bunch
   // of debug prints can cause the function to be too large for inlining.  So
   // it's important that we define this function out of line so that it can't be
   // inlined.
   POLAR_ATTRIBUTE_NORETURN
   void fatalUncheckedError() const;
#endif

   void assertIsChecked()
   {
#if POLAR_ENABLE_ABI_BREAKING_CHECKS
      if (POLAR_UNLIKELY(!getChecked() || getPtr())) {
         fatalUncheckedError();
      }
#endif
   }

   ErrorInfoBase *getPtr() const
   {
      return reinterpret_cast<ErrorInfoBase*>(
               reinterpret_cast<uintptr_t>(m_payload) &
               ~static_cast<uintptr_t>(0x1));
   }

   void setPtr(ErrorInfoBase *errorInfo)
   {
#if POLAR_ENABLE_ABI_BREAKING_CHECKS
      m_payload = reinterpret_cast<ErrorInfoBase*>(
               (reinterpret_cast<uintptr_t>(errorInfo) &
                ~static_cast<uintptr_t>(0x1)) |
               (reinterpret_cast<uintptr_t>(m_payload) & 0x1));
#else
      m_payload = errorInfo;
#endif
   }

   bool getChecked() const
   {
#if POLAR_ENABLE_ABI_BREAKING_CHECKS
      return (reinterpret_cast<uintptr_t>(m_payload) & 0x1) == 0;
#else
      return true;
#endif
   }

   void setChecked(bool value)
   {
      m_payload = reinterpret_cast<ErrorInfoBase*>(
               (reinterpret_cast<uintptr_t>(m_payload) &
                ~static_cast<uintptr_t>(0x1)) |
               (value ? 0 : 1));
   }

   std::unique_ptr<ErrorInfoBase> takePayload()
   {
      std::unique_ptr<ErrorInfoBase> temp(getPtr());
      setPtr(nullptr);
      setChecked(true);
      return temp;
   }

   ErrorInfoBase *m_payload = nullptr;
};

/// Subclass of Error for the sole purpose of identifying the success path in
/// the type system. This allows to catch invalid conversion to Expected<T> at
/// compile time.
class ErrorSuccess : public Error
{};

inline ErrorSuccess Error::getSuccess()
{
   return ErrorSuccess();
}

/// Make a Error instance representing failure using the given error info
/// type.
template <typename ErrorType, typename... ArgTs>
Error make_error(ArgTs &&... args)
{
   return Error(std::make_unique<ErrorType>(std::forward<ArgTs>(args)...));
}

/// Base class for user error types. Users should declare their error types
/// like:
///
/// class MyError : public ErrorInfo<MyError> {
///   ....
/// };
///
/// This class provides an implementation of the ErrorInfoBase::kind
/// method, which is used by the Error RTTI system.
template <typename ThisErrorType, typename ParentErrorType = ErrorInfoBase>
class ErrorInfo : public ParentErrorType
{
public:
   static const void *getClassId()
   {
      return &ThisErrorType::m_id;
   }

   const void *getDynamicClassId() const override
   {
      return &ThisErrorType::m_id;
   }

   bool isA(const void *const classId) const override
   {
      return classId == getClassId() || ParentErrorType::isA(classId);
   }
};

/// Special ErrorInfo subclass representing a list of ErrorInfos.
/// Instances of this class are constructed by joinError.
class ErrorList final : public ErrorInfo<ErrorList>
{
   // handle_errors needs to be able to iterate the payload list of an
   // ErrorList.
   template <typename... HandlerTs>
   friend Error handle_errors(Error error, HandlerTs &&... handlers);

   // joinErrors is implemented in terms of join.
   friend Error joinErrors(Error, Error);

public:
   void log(RawOutStream &outStream) const override
   {
      outStream << "Multiple errors:\n";
      for (auto &errorPayload : m_payloads) {
         errorPayload->log(outStream);
         outStream << "\n";
      }
   }

   std::error_code convertToErrorCode() const override;

   // Used by ErrorInfo::classId.
   static char m_id;

private:
   ErrorList(std::unique_ptr<ErrorInfoBase> payload1,
             std::unique_ptr<ErrorInfoBase> payload2)
   {
      assert(!payload1->isA<ErrorList>() && !payload2->isA<ErrorList>() &&
             "ErrorList constructor payloads should be singleton errors");
      m_payloads.push_back(std::move(payload1));
      m_payloads.push_back(std::move(payload2));
   }

   static Error join(Error error1, Error error2)
   {
      if (!error1) {
         return error2;
      }

      if (!error2) {
         return error1;
      }

      if (error1.isA<ErrorList>()) {
         auto &e1List = static_cast<ErrorList &>(*error1.getPtr());
         if (error2.isA<ErrorList>()) {
            auto e2Payload = error2.takePayload();
            auto &e2List = static_cast<ErrorList &>(*e2Payload);
            for (auto &payload : e2List.m_payloads) {
               e1List.m_payloads.push_back(std::move(payload));
            }
         } else {
            e1List.m_payloads.push_back(error2.takePayload());
         }
         return error1;
      }
      if (error2.isA<ErrorList>()) {
         auto &e2List = static_cast<ErrorList &>(*error2.getPtr());
         e2List.m_payloads.insert(e2List.m_payloads.begin(), error1.takePayload());
         return error2;
      }
      return Error(std::unique_ptr<ErrorList>(
                      new ErrorList(error1.takePayload(), error2.takePayload())));
   }

   std::vector<std::unique_ptr<ErrorInfoBase>> m_payloads;
};

/// Concatenate errors. The resulting Error is unchecked, and contains the
/// ErrorInfo(s), if any, contained in E1, followed by the
/// ErrorInfo(s), if any, contained in E2.
inline Error joinErrors(Error error1, Error error2)
{
   return ErrorList::join(std::move(error1), std::move(error2));
}

/// Tagged union holding either a T or a Error.
///
/// This class parallels ErrorOr, but replaces error_code with Error. Since
/// Error cannot be copied, this class replaces getError() with
/// takeError(). It also adds an bool errorIsA<ErrorType>() method for testing the
/// error class type.
template <class T> class POLAR_NODISCARD Expected
{
   template <class T1> friend class ExpectedAsOutParameter;
   template <class OtherType> friend class Expected;

   static const bool isRef = std::is_reference<T>::value;

   using wrap = ReferenceStorage<typename std::remove_reference<T>::type>;

   using error_type = std::unique_ptr<ErrorInfoBase>;

public:
   using storage_type = typename std::conditional<isRef, wrap, T>::type;
   using value_type = T;

private:
   using reference = typename std::remove_reference<T>::type &;
   using const_reference = const typename std::remove_reference<T>::type &;
   using pointer = typename std::remove_reference<T>::type *;
   using const_pointer = const typename std::remove_reference<T>::type *;

public:
   /// Create an Expected<T> error value from the given Error.
   Expected(Error error)
      : m_hasError(true)
   #if POLAR_ENABLE_ABI_BREAKING_CHECKS
      // Expected is unchecked upon construction in Debug builds.
      , m_unchecked(true)
   #endif
   {
      assert(error && "Cannot create Expected<T> from Error success value.");
      new (getErrorStorage()) error_type(error.takePayload());
   }

   /// Forbid to convert from Error::success() implicitly, this avoids having
   /// Expected<T> foo() { return Error::success(); } which compiles otherwise
   /// but triggers the assertion above.
   Expected(ErrorSuccess) = delete;

   /// Create an Expected<T> success value from the given OtherType value, which
   /// must be convertible to T.
   template <typename OtherType>
   Expected(OtherType &&value,
            typename std::enable_if<std::is_convertible<OtherType, T>::value>::type
            * = nullptr)
      : m_hasError(false)
   #if POLAR_ENABLE_ABI_BREAKING_CHECKS
      // Expected is unchecked upon construction in Debug builds.
      , m_unchecked(true)
   #endif
   {
      new (getStorage()) storage_type(std::forward<OtherType>(value));
   }

   /// Move construct an Expected<T> value.
   Expected(Expected &&other) { moveConstruct(std::move(other)); }

   /// Move construct an Expected<T> value from an Expected<OtherType>, where OtherType
   /// must be convertible to T.
   template <class OtherType>
   Expected(Expected<OtherType> &&other,
            typename std::enable_if<std::is_convertible<OtherType, T>::value>::type
            * = nullptr)
   {
      moveConstruct(std::move(other));
   }

   /// Move construct an Expected<T> value from an Expected<OtherType>, where OtherType
   /// isn't convertible to T.
   template <class OtherType>
   explicit Expected(
         Expected<OtherType> &&other,
         typename std::enable_if<!std::is_convertible<OtherType, T>::value>::type * =
         nullptr)
   {
      moveConstruct(std::move(other));
   }

   /// Move-assign from another Expected<T>.
   Expected &operator=(Expected &&other)
   {
      moveAssign(std::move(other));
      return *this;
   }

   /// Destroy an Expected<T>.
   ~Expected()
   {
      assertIsChecked();
      if (!m_hasError) {
         getStorage()->~storage_type();
      } else {
         getErrorStorage()->~error_type();
      }
   }

   /// \brief Return false if there is an error.
   explicit operator bool()
   {
#if POLAR_ENABLE_ABI_BREAKING_CHECKS
      m_unchecked = m_hasError;
#endif
      return !m_hasError;
   }

   /// \brief Returns a reference to the stored T value.
   reference get()
   {
      assertIsChecked();
      return *getStorage();
   }

   /// \brief Returns a const reference to the stored T value.
   const_reference get() const
   {
      assertIsChecked();
      return const_cast<Expected<T> *>(this)->get();
   }

   /// \brief Check that this Expected<T> is an error of type ErrorType.
   template <typename ErrorType>
   bool errorIsA() const
   {
      return m_hasError && (*getErrorStorage())->template isA<ErrorType>();
   }

   /// \brief Take ownership of the stored error.
   /// After calling this the Expected<T> is in an indeterminate state that can
   /// only be safely destructed. No further calls (beside the destructor) should
   /// be made on the Expected<T> vaule.
   Error takeError()
   {
#if POLAR_ENABLE_ABI_BREAKING_CHECKS
      m_unchecked = false;
#endif
      return m_hasError ? Error(std::move(*getErrorStorage())) : Error::getSuccess();
   }

   /// \brief Returns a pointer to the stored T value.
   pointer operator->()
   {
      assertIsChecked();
      return toPointer(getStorage());
   }

   /// \brief Returns a const pointer to the stored T value.
   const_pointer operator->() const
   {
      assertIsChecked();
      return toPointer(getStorage());
   }

   /// \brief Returns a reference to the stored T value.
   reference operator*()
   {
      assertIsChecked();
      return *getStorage();
   }

   /// \brief Returns a const reference to the stored T value.
   const_reference operator*() const
   {
      assertIsChecked();
      return *getStorage();
   }

private:
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

   template <class OtherType> void moveConstruct(Expected<OtherType> &&other)
   {
      m_hasError = other.m_hasError;
#if POLAR_ENABLE_ABI_BREAKING_CHECKS
      m_unchecked = true;
      other.m_unchecked = false;
#endif

      if (!m_hasError) {
         new (getStorage()) storage_type(std::move(*other.getStorage()));
      } else {
         new (getErrorStorage()) error_type(std::move(*other.getErrorStorage()));
      }
   }

   template <class OtherType> void moveAssign(Expected<OtherType> &&other)
   {
      assertIsChecked();
      if (compareThisIfSameType(*this, other)) {
         return;
      }
      this->~Expected();
      new (this) Expected(std::move(other));
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

   const_pointer toPointer(const wrap *value) const
   {
      return &value->get();
   }

   storage_type *getStorage()
   {
      assert(!m_hasError && "Cannot get value when an error exists!");
      return reinterpret_cast<storage_type *>(m_tstorage.m_buffer);
   }

   const storage_type *getStorage() const
   {
      assert(!m_hasError && "Cannot get value when an error exists!");
      return reinterpret_cast<const storage_type *>(m_tstorage.m_buffer);
   }

   error_type *getErrorStorage()
   {
      assert(m_hasError && "Cannot get error when a value exists!");
      return reinterpret_cast<error_type *>(m_errorStorage.m_buffer);
   }

   const error_type *getErrorStorage() const
   {
      assert(m_hasError && "Cannot get error when a value exists!");
      return reinterpret_cast<const error_type *>(m_errorStorage.m_buffer);
   }

   // Used by ExpectedAsOutParameter to reset the checked flag.
   void setUnchecked()
   {
#if POLAR_ENABLE_ABI_BREAKING_CHECKS
      m_unchecked = true;
#endif
   }

#if POLAR_ENABLE_ABI_BREAKING_CHECKS
   POLAR_ATTRIBUTE_NORETURN
   POLAR_ATTRIBUTE_NOINLINE
   void fatalUncheckedExpected() const
   {
      debug_stream() << "Expected<T> must be checked before access or destruction.\n";
      if (m_hasError) {
         debug_stream() << "Unchecked Expected<T> contained error:\n";
         (*getErrorStorage())->log(debug_stream());
      } else {
         debug_stream() << "Expected<T> value was in success state. (Note: Expected<T> "
                           "values in success mode must still be checked prior to being "
                           "destroyed).\n";
      }
      abort();
   }
#endif

   void assertIsChecked()
   {
#if POLAR_ENABLE_ABI_BREAKING_CHECKS
      if (POLAR_UNLIKELY(m_unchecked)) {
         fatalUncheckedExpected();
      }
#endif
   }

   union {
      AlignedCharArrayUnion<storage_type> m_tstorage;
      AlignedCharArrayUnion<error_type> m_errorStorage;
   };
   bool m_hasError : 1;
#if POLAR_ENABLE_ABI_BREAKING_CHECKS
   bool m_unchecked : 1;
#endif
};

/// Report a serious error, calling any installed error handler. See
/// ErrorHandling.h.
POLAR_ATTRIBUTE_NORETURN void report_fatal_error(Error error,
                                                 bool genCrashDiag = true);

/// Report a fatal error if Err is a failure value.
///
/// This function can be used to wrap calls to fallible functions ONLY when it
/// is known that the Error will always be a success value. E.g.
///
///   @code{.cpp}
///   // foo only attempts the fallible operation if DoFallibleOperation is
///   // true. If DoFallibleOperation is false then foo always returns
///   // Error::success().
///   Error foo(bool DoFallibleOperation);
///
///   cant_fail(foo(false));
///   @endcode
inline void cant_fail(Error error, const char *msg = nullptr)
{
   if (error) {
      if (!msg) {
         msg = "Failure value returned from cant_fail wrapped call";
      }
      polar_unreachable(msg);
   }
}

/// Report a fatal error if valueOrError is a failure value, otherwise unwraps and
/// returns the contained value.
///
/// This function can be used to wrap calls to fallible functions ONLY when it
/// is known that the Error will always be a success value. E.g.
///
///   @code{.cpp}
///   // foo only attempts the fallible operation if DoFallibleOperation is
///   // true. If DoFallibleOperation is false then foo always returns an int.
///   Expected<int> foo(bool DoFallibleOperation);
///
///   int X = cant_fail(foo(false));
///   @endcode
template <typename T>
T cant_fail(Expected<T> valueOrError, const char *msg = nullptr)
{
   if (valueOrError) {
      return std::move(*valueOrError);
   } else {
      if (!msg) {
         msg = "Failure value returned from cant_fail wrapped call";
      }
      polar_unreachable(msg);
   }
}

/// Report a fatal error if valueOrError is a failure value, otherwise unwraps and
/// returns the contained reference.
///
/// This function can be used to wrap calls to fallible functions ONLY when it
/// is known that the Error will always be a success value. E.g.
///
///   @code{.cpp}
///   // foo only attempts the fallible operation if DoFallibleOperation is
///   // true. If DoFallibleOperation is false then foo always returns a Bar&.
///   Expected<Bar&> foo(bool DoFallibleOperation);
///
///   Bar &X = cant_fail(foo(false));
///   @endcode
template <typename T>
T& cant_fail(Expected<T&> valueOrError, const char *msg = nullptr)
{
   if (valueOrError) {
      return *valueOrError;
   } else {
      if (!msg) {
         msg = "Failure value returned from cant_fail wrapped call";
      }
      polar_unreachable(msg);
   }
}

/// Helper for testing applicability of, and applying, handlers for
/// ErrorInfo types.
template <typename HandlerType>
class ErrorHandlerTraits
      : public ErrorHandlerTraits<decltype(
      &std::remove_reference<HandlerType>::type::operator())>
{};

// Specialization functions of the form 'Error (const ErrorType&)'.
template <typename ErrorType>
class ErrorHandlerTraits<Error (&)(ErrorType &)>
{
   public:
   static bool appliesTo(const ErrorInfoBase &errorInfo)
   {
      return errorInfo.template isA<ErrorType>();
   }

   template <typename HandlerType>
   static Error apply(HandlerType &&handler, std::unique_ptr<ErrorInfoBase> errorInfo)
   {
      assert(appliesTo(*errorInfo) && "Applying incorrect handler");
      return handler(static_cast<ErrorType &>(*errorInfo));
   }
};

// Specialization functions of the form 'void (const ErrorType&)'.
template <typename ErrorType>
class ErrorHandlerTraits<void (&)(ErrorType &)>
{
   public:
   static bool appliesTo(const ErrorInfoBase &errorInfo)
   {
      return errorInfo.template isA<ErrorType>();
   }

   template <typename HandlerType>
   static Error apply(HandlerType &&handler, std::unique_ptr<ErrorInfoBase> errorInfo)
   {
      assert(appliesTo(*errorInfo) && "Applying incorrect handler");
      handler(static_cast<ErrorType &>(*errorInfo));
      return Error::getSuccess();
   }
};

/// Specialization for functions of the form 'Error (std::unique_ptr<ErrorType>)'.
template <typename ErrorType>
class ErrorHandlerTraits<Error (&)(std::unique_ptr<ErrorType>)>
{
   public:
   static bool appliesTo(const ErrorInfoBase &errorInfo)
   {
      return errorInfo.template isA<ErrorType>();
   }

   template <typename HandlerType>
   static Error apply(HandlerType &&handler, std::unique_ptr<ErrorInfoBase> errorInfo)
   {
      assert(appliesTo(*errorInfo) && "Applying incorrect handler");
      std::unique_ptr<ErrorType> subE(static_cast<ErrorType *>(errorInfo.release()));
      return handler(std::move(subE));
   }
};

/// Specialization for functions of the form 'void (std::unique_ptr<ErrorType>)'.
template <typename ErrorType>
class ErrorHandlerTraits<void (&)(std::unique_ptr<ErrorType>)>
{
   public:
   static bool appliesTo(const ErrorInfoBase &errorInfo)
   {
      return errorInfo.template isA<ErrorType>();
   }

   template <typename HandlerType>
   static Error apply(HandlerType &&handler, std::unique_ptr<ErrorInfoBase> errorInfo)
   {
      assert(appliesTo(*errorInfo) && "Applying incorrect handler");
      std::unique_ptr<ErrorType> subE(static_cast<ErrorType *>(errorInfo.release()));
      handler(std::move(subE));
      return Error::getSuccess();
   }
};

// Specialization for member functions of the form 'RetT (const ErrorType&)'.
template <typename C, typename RetT, typename ErrorType>
class ErrorHandlerTraits<RetT (C::*)(ErrorType &)>
   : public ErrorHandlerTraits<RetT (&)(ErrorType &)>
{};

// Specialization for member functions of the form 'RetT (const ErrorType&) const'.
template <typename C, typename RetT, typename ErrorType>
class ErrorHandlerTraits<RetT (C::*)(ErrorType &) const>
   : public ErrorHandlerTraits<RetT (&)(ErrorType &)>
{};

// Specialization for member functions of the form 'RetT (const ErrorType&)'.
template <typename C, typename RetT, typename ErrorType>
class ErrorHandlerTraits<RetT (C::*)(const ErrorType &)>
   : public ErrorHandlerTraits<RetT (&)(ErrorType &)>
{};

// Specialization for member functions of the form 'RetT (const ErrorType&) const'.
template <typename C, typename RetT, typename ErrorType>
class ErrorHandlerTraits<RetT (C::*)(const ErrorType &) const>
   : public ErrorHandlerTraits<RetT (&)(ErrorType &)>
{};

/// Specialization for member functions of the form
/// 'RetT (std::unique_ptr<ErrorType>)'.
template <typename C, typename RetT, typename ErrorType>
class ErrorHandlerTraits<RetT (C::*)(std::unique_ptr<ErrorType>)>
   : public ErrorHandlerTraits<RetT (&)(std::unique_ptr<ErrorType>)>
{};

/// Specialization for member functions of the form
/// 'RetT (std::unique_ptr<ErrorType>) const'.
template <typename C, typename RetT, typename ErrorType>
class ErrorHandlerTraits<RetT (C::*)(std::unique_ptr<ErrorType>) const>
   : public ErrorHandlerTraits<RetT (&)(std::unique_ptr<ErrorType>)>
{};

inline Error handle_error_impl(std::unique_ptr<ErrorInfoBase> payload)
{
   return Error(std::move(payload));
}

template <typename HandlerType, typename... HandlerTs>
Error handle_error_impl(std::unique_ptr<ErrorInfoBase> payload,
                        HandlerType &&handler, HandlerTs &&... handlers)
{
   if (ErrorHandlerTraits<HandlerType>::appliesTo(*payload))
      return ErrorHandlerTraits<HandlerType>::apply(std::forward<HandlerType>(handler),
                                                    std::move(payload));
   return handle_error_impl(std::move(payload),
                            std::forward<HandlerTs>(handlers)...);
}

/// Pass the ErrorInfo(s) contained in E to their respective handlers. Any
/// unhandled errors (or Errors returned by handlers) are re-concatenated and
/// returned.
/// Because this function returns an error, its result must also be checked
/// or returned. If you intend to handle all errors use handle_all_errors
/// (which returns void, and will abort() on unhandled errors) instead.
template <typename... HandlerTs>
Error handle_errors(Error error, HandlerTs &&... handles)
{
   if (!error) {
      return Error::getSuccess();
   }
   std::unique_ptr<ErrorInfoBase> payload = error.takePayload();
   if (payload->isA<ErrorList>()) {
      ErrorList &list = static_cast<ErrorList &>(*payload);
      Error ret;
      for (auto &payload : list.m_payloads) {
         ret = ErrorList::join(
                  std::move(ret),
                  handle_error_impl(std::move(payload), std::forward<HandlerTs>(handles)...));
      }
      return ret;
   }

   return handle_error_impl(std::move(payload), std::forward<HandlerTs>(handles)...);
}

/// Behaves the same as handle_errors, except that it requires that all
/// errors be handled by the given handlers. If any unhandled error remains
/// after the handlers have run, report_fatal_error() will be called.
template <typename... HandlerTs>
void handle_all_errors(Error error, HandlerTs &&... handlers)
{
   cant_fail(handle_errors(std::move(error), std::forward<HandlerTs>(handlers)...));
}

/// Check that E is a non-error, then drop it.
/// If E is an error report_fatal_error will be called.
inline void handle_all_errors(Error error)
{
   cant_fail(std::move(error));
}

/// Handle any errors (if present) in an Expected<T>, then try a recovery path.
///
/// If the incoming value is a success value it is returned unmodified. If it
/// is a failure value then it the contained error is passed to handle_errors.
/// If handle_errors is able to handle the error then the recovery_path functor
/// is called to supply the final result. If handle_errors is not able to
/// handle all errors then the unhandled errors are returned.
///
/// This utility enables the follow pattern:
///
///   @code{.cpp}
///   enum FooStrategy { Aggressive, Conservative };
///   Expected<Foo> foo(FooStrategy S);
///
///   auto ResultOrErr =
///     handle_expected(
///       foo(Aggressive),
///       []() { return foo(Conservative); },
///       [](AggressiveStrategyError&) {
///         // Implicitly conusme this - we'll recover by using a conservative
///         // strategy.
///       });
///
///   @endcode
template <typename T, typename RecoveryFtor, typename... HandlerTs>
Expected<T> handle_expected(Expected<T> valueOrError, RecoveryFtor &&recovery_path,
                            HandlerTs &&... handlers) {
   if (valueOrError) {
      return valueOrError;
   }
   if (auto error = handle_errors(valueOrError.takeError(),
                                  std::forward<HandlerTs>(handlers)...)) {
      return std::move(error);
   }
   return recovery_path();
}

/// Log all errors (if any) in E to OS. If there are any errors, Errorm_banner
/// will be printed before the first one is logged. A newline will be printed
/// after each error.
///
/// This is useful in the base level of your program to allow clean termination
/// (allowing clean deallocation of resources, etc.), while reporting error
/// information to the user.
void log_all_unhandled_errors(Error error, RawOutStream &outStream, Twine errorm_banner);

/// Write all error messages (if any) in E to a string. The newline character
/// is used to separate error messages.
inline std::string to_string(Error error)
{
   SmallVector<std::string, 2> errors;
   handle_all_errors(std::move(error), [&errors](const ErrorInfoBase &errorInfo) {
      errors.push_back(errorInfo.message());
   });
   return polar::basic::join(errors.begin(), errors.end(), "\n");
}

/// Consume a Error without doing anything. This method should be used
/// only where an error can be considered a reasonable and expected return
/// value.
///
/// Uses of this method are potentially indicative of design problems: If it's
/// legitimate to do nothing while processing an "error", the error-producer
/// might be more clearly refactored to return an Optional<T>.
inline void consume_error(Error error)
{
   handle_all_errors(std::move(error), [](const ErrorInfoBase &) {});
}

/// Helper for Errors used as out-parameters.
///
/// This helper is for use with the Error-as-out-parameter idiom, where an error
/// is passed to a function or method by reference, rather than being returned.
/// In such cases it is helpful to set the checked bit on entry to the function
/// so that the error can be written to (unchecked Errors abort on assignment)
/// and clear the checked bit on exit so that clients cannot accidentally forget
/// to check the result. This helper performs these actions automatically using
/// RAII:
///
///   @code{.cpp}
///   Result foo(Error &Err) {
///     ErrorAsOutParameter ErrAsOutParam(&Err); // 'Checked' flag set
///     // <body of foo>
///     // <- 'Checked' flag auto-cleared when ErrAsOutParam is destructed.
///   }
///   @endcode
///
/// ErrorAsOutParameter takes an Error* rather than Error& so that it can be
/// used with optional Errors (Error pointers that are allowed to be null). If
/// ErrorAsOutParameter took an Error reference, an instance would have to be
/// created inside every condition that verified that Error was non-null. By
/// taking an Error pointer we can just create one instance at the top of the
/// function.
class ErrorAsOutParameter
{
public:
   ErrorAsOutParameter(Error *error) : m_error(error)
   {
      // Raise the checked bit if Err is success.
      if (error) {
         (void)!!*error;
      }
   }

   ~ErrorAsOutParameter()
   {
      // Clear the checked bit.
      if (m_error && !*m_error) {
         *m_error = Error::getSuccess();
      }
   }

private:
   Error *m_error;
};

/// Helper for Expected<T>s used as out-parameters.
///
/// See ErrorAsOutParameter.
template <typename T>
class ExpectedAsOutParameter
{
public:
   ExpectedAsOutParameter(Expected<T> *valueOrError)
      : m_valueOrError(valueOrError) {
      if (m_valueOrError) {
         (void)!!*m_valueOrError;
      }
   }

   ~ExpectedAsOutParameter() {
      if (m_valueOrError) {
         m_valueOrError->setUnchecked();
      }
   }

private:
   Expected<T> *m_valueOrError;
};

/// This class wraps a std::error_code in a Error.
///
/// This is useful if you're writing an interface that returns a Error
/// (or Expected) and you want to call code that still returns
/// std::error_codes.
class ECError : public ErrorInfo<ECError>
{
   friend Error error_code_to_error(std::error_code);

public:
   void setErrorCode(std::error_code errorCode)
   {
      this->m_errorCode = errorCode;
   }

   std::error_code convertToErrorCode() const override
   {
      return m_errorCode;
   }

   void log(RawOutStream &outStream) const override
   {
      outStream << m_errorCode.message();
   }

   // Used by ErrorInfo::m_classId.
   static char m_id;

protected:
   ECError() = default;
   ECError(std::error_code errorCode) : m_errorCode(errorCode)
   {}

   std::error_code m_errorCode;
};

/// The value returned by this function can be returned from convertToErrorCode
/// for Error values where no sensible translation to std::error_code exists.
/// It should only be used in this situation, and should never be used where a
/// sensible conversion to std::error_code is available, as attempts to convert
/// to/from this error will result in a fatal error. (i.e. it is a programmatic
///error to try to convert such a value).
std::error_code inconvertible_error_code();

/// Helper for converting an std::error_code to a Error.
Error error_code_to_error(std::error_code errorCode);

/// Helper for converting an ECError to a std::error_code.
///
/// This method requires that Err be Error() or an ECError, otherwise it
/// will trigger a call to abort().
std::error_code error_to_error_code(Error error);

/// Convert an OptionalError<T> to an Expected<T>.
template <typename T>
Expected<T> optional_error_to_expected(OptionalError<T> &&error)
{
   if (auto errorCode = error.getError()) {
      return error_code_to_error(errorCode);
   }
   return std::move(*error);
}

/// Convert an Expected<T> to an OptionalError<T>.
template <typename T>
OptionalError<T> expected_to_optional_error(Expected<T> &&expected)
{
   if (auto error = expected.takeError()) {
      return error_to_error_code(std::move(error));
   }
   return std::move(*expected);
}

/// This class wraps a string in an Error.
///
/// StringError is useful in cases where the client is not expected to be able
/// to consume the specific error message programmatically (for example, if the
/// error message is to be presented to the user).
class StringError : public ErrorInfo<StringError>
{
public:
   static char m_id;

   StringError(const Twine &twine, std::error_code errorCode);

   void log(RawOutStream &outStream) const override;
   std::error_code convertToErrorCode() const override;

   const std::string &getMessage() const
   {
      return m_msg;
   }

private:
   std::string m_msg;
   std::error_code m_errorCode;
};

/// Helper for check-and-exit error handling.
///
/// For tool use only. NOT FOR USE IN LIBRARY CODE.
///
class ExitOnError
{
public:
   /// Create an error on exit helper.
   ExitOnError(std::string banner = "", int defaultErrorExitCode = 1)
      : m_banner(std::move(banner)),
        m_getExitCode([=](const Error &) { return defaultErrorExitCode; })
   {}

   /// Set the m_banner string for any errors caught by operator().
   void setBanner(std::string banner)
   {
      this->m_banner = std::move(banner);
   }

   /// Set the exit-code mapper function.
   void setExitCodeMapper(std::function<int(const Error &)> getExitCode)
   {
      this->m_getExitCode = std::move(getExitCode);
   }

   /// Check Err. If it's in a failure state log the error(s) and exit.
   void operator()(Error error) const
   {
      checkError(std::move(error));
   }

   /// Check E. If it's in a success state then return the contained value. If
   /// it's in a failure state log the error(s) and exit.
   template <typename T>
   T operator()(Expected<T> &&expected) const
   {
      checkError(expected.takeError());
      return std::move(*expected);
   }

   /// Check E. If it's in a success state then return the contained reference. If
   /// it's in a failure state log the error(s) and exit.
   template <typename T>
   T& operator()(Expected<T&> &&expected) const
   {
      checkError(expected.takeError());
      return *expected;
   }

private:
   void checkError(Error error) const
   {
      if (error) {
         int exitCode = m_getExitCode(error);
         log_all_unhandled_errors(std::move(error), error_stream(), m_banner);
         exit(exitCode);
      }
   }

   std::string m_banner;
   std::function<int(const Error &)> m_getExitCode;
};

} // utils
} // polar

#endif // POLAR_UTILS_ERROR_ERROR_TYPE_H
