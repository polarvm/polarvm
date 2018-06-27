// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORstr.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/27.

#ifndef POLAR_UTILS_YAML_TRAITS_H
#define POLAR_UTILS_YAML_TRAITS_H

#include "polar/basic/adt/SmallVector.h"
#include "polar/basic/adt/StringExtras.h"
#include "polar/basic/adt/StringMap.h"
#include "polar/basic/adt/StringRef.h"
#include "polar/basic/adt/Twine.h"
#include "polar/utils/AlignOf.h"
#include "polar/utils/Allocator.h"
#include "polar/utils/Endian.h"
#include "polar/utils/SourceMgr.h"
#include "polar/utils/yaml/Parser.h"
#include "polar/utils/RawOutStream.h".h"
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <new>
#include <string>
#include <system_error>
#include <type_traits>
#include <vector>
#include <regex>
#include <optional>

namespace polar {
namespace yaml {

using polar::basic::is_alnum;
using polar::utils::RawStringOutStream;
using polar::utils::error_stream;
using polar::utils::internal::PackedEndianSpecificIntegral;
using polar::utils::AlignedCharArrayUnion;
using polar::basic::StringMap;
using polar::basic::SmallVector;
using polar::basic::SameType;

struct EmptyContext {};

/// This class should be specialized by any type that needs to be converted
/// to/from a YAML mapping.  For example:
///
///     struct MappingTraits<MyStruct> {
///       static void mapping(IO &io, MyStruct &s) {
///         io.mapRequired("name", s.name);
///         io.mapRequired("size", s.size);
///         io.mapOptional("age",  s.age);
///       }
///     };
template<class T>
struct MappingTraits
{
   // Must provide:
   // static void mapping(IO &io, T &fields);
   // Optionally may provide:
   // static StringRef validate(IO &io, T &fields);
   //
   // The optional flow flag will cause generated YAML to use a flow mapping
   // (e.g. { a: 0, b: 1 }):
   // static const bool flow = true;
};

/// This class is similar to MappingTraits<T> but allows you to pass in
/// additional context for each map operation.  For example:
///
///     struct MappingContextTraits<MyStruct, MyContext> {
///       static void mapping(IO &io, MyStruct &s, MyContext &c) {
///         io.mapRequired("name", s.name);
///         io.mapRequired("size", s.size);
///         io.mapOptional("age",  s.age);
///         ++c.TimesMapped;
///       }
///     };
template <class T, class Context>
struct MappingContextTraits
{
   // Must provide:
   // static void mapping(IO &io, T &fields, Context &context);
   // Optionally may provide:
   // static StringRef validate(IO &io, T &fields, Context &context);
   //
   // The optional flow flag will cause generated YAML to use a flow mapping
   // (e.g. { a: 0, b: 1 }):
   // static const bool flow = true;
};

/// This class should be specialized by any integral type that converts
/// to/from a YAML scalar where there is a one-to-one mapping between
/// in-memory values and a string in YAML.  For example:
///
///     struct ScalarEnumerationTraits<Colors> {
///         static void enumeration(IO &io, Colors &value) {
///           io.enumCase(value, "red",   cRed);
///           io.enumCase(value, "blue",  cBlue);
///           io.enumCase(value, "green", cGreen);
///         }
///       };
template<typename T>
struct ScalarEnumerationTraits
{
   // Must provide:
   // static void enumeration(IO &io, T &value);
};

/// This class should be specialized by any integer type that is a union
/// of bit values and the YAML representation is a flow sequence of
/// strings.  For example:
///
///      struct ScalarBitSetTraits<MyFlags> {
///        static void bitset(IO &io, MyFlags &value) {
///          io.bitSetCase(value, "big",   flagBig);
///          io.bitSetCase(value, "flat",  flagFlat);
///          io.bitSetCase(value, "round", flagRound);
///        }
///      };
template<typename T>
struct ScalarBitSetTraits
{
   // Must provide:
   // static void bitset(IO &io, T &value);
};

/// Describe which type of quotes should be used when quoting is necessary.
/// Some non-printable characters need to be double-quoted, while some others
/// are fine with simple-quoting, and some don't need any quoting.
enum class QuotingType { None, Single, Double };

/// This class should be specialized by type that requires custom conversion
/// to/from a yaml scalar.  For example:
///
///    template<>
///    struct ScalarTraits<MyType> {
///      static void output(const MyType &val, void*, polar::RawOutStream &out) {
///        // stream out custom formatting
///        out << polar::format("%x", val);
///      }
///      static StringRef input(StringRef scalar, void*, MyType &value) {
///        // parse scalar and set `value`
///        // return empty string on success, or error string
///        return StringRef();
///      }
///      static QuotingType mustQuote(StringRef) { return QuotingType::Single; }
///    };
template<typename T>
struct ScalarTraits
{
   // Must provide:
   //
   // Function to write the value as a string:
   //static void output(const T &value, void *ctxt, polar::RawOutStream &out);
   //
   // Function to convert a string to a value.  Returns the empty
   // StringRef on success or an error string if string is malformed:
   //static StringRef input(StringRef scalar, void *ctxt, T &value);
   //
   // Function to determine if the value should be quoted.
   //static QuotingType mustQuote(StringRef);
};

/// This class should be specialized by type that requires custom conversion
/// to/from a YAML literal block scalar. For example:
///
///    template <>
///    struct BlockScalarTraits<MyType> {
///      static void output(const MyType &valueue, void*, polar::RawOutStream &Out)
///      {
///        // stream out custom formatting
///        Out << Val;
///      }
///      static StringRef input(StringRef strcalar, void*, MyType &valueue) {
///        // parse scalar and set `value`
///        // return empty string on success, or error string
///        return StringRef();
///      }
///    };
template <typename T>
struct BlockScalarTraits
{
   // Must provide:
   //
   // Function to write the value as a string:
   // static void output(const T &valueue, void *ctx, polar::RawOutStream &Out);
   //
   // Function to convert a string to a value.  Returns the empty
   // StringRef on success or an error string if string is malformed:
   // static StringRef input(StringRef strcalar, void *ctxt, T &valueue);
};

/// This class should be specialized by any type that needs to be converted
/// to/from a YAML sequence.  For example:
///
///    template<>
///    struct SequenceTraits<MyContainer> {
///      static size_t size(IO &io, MyContainer &seq) {
///        return seq.size();
///      }
///      static MyType& element(IO &, MyContainer &seq, size_t index) {
///        if ( index >= seq.size() )
///          seq.resize(index+1);
///        return seq[index];
///      }
///    };
template<typename T, typename EnableIf = void>
struct SequenceTraits
{
   // Must provide:
   // static size_t size(IO &io, T &seq);
   // static T::value_type& element(IO &io, T &seq, size_t index);
   //
   // The following is option and will cause generated YAML to use
   // a flow sequence (e.g. [a,b,c]).
   // static const bool flow = true;
};

/// This class should be specialized by any type for which vectors of that
/// type need to be converted to/from a YAML sequence.
template<typename T, typename EnableIf = void>
struct SequenceElementTraits
{
   // Must provide:
   // static const bool flow;
};

/// This class should be specialized by any type that needs to be converted
/// to/from a list of YAML documents.
template<typename T>
struct DocumentListTraits
{
   // Must provide:
   // static size_t size(IO &io, T &seq);
   // static T::value_type& element(IO &io, T &seq, size_t index);
};

/// This class should be specialized by any type that needs to be converted
/// to/from a YAML mapping in the case where the names of the keys are not known
/// in advance, e.g. a string map.
template <typename T>
struct CustomMappingTraits
{
   // static void inputOne(IO &io, StringRef key, T &elem);
   // static void output(IO &io, T &elem);
};

// Only used for better diagnostics of missing traits
template <typename T>
struct MissingTrait;

// Test if ScalarEnumerationTraits<T> is defined on type T.
template <class T>
struct HasScalarEnumerationTraits
{
   using SignatureEnumeration = void (*)(class IO&, T&);

   template <typename U>
   static char test(SameType<SignatureEnumeration, &U::enumeration>*);

   template <typename U>
   static double test(...);

public:
   static bool const value =
         (sizeof(test<ScalarEnumerationTraits<T>>(nullptr)) == 1);
};

// Test if ScalarBitSetTraits<T> is defined on type T.
template <class T>
struct HasScalarBitSetTraits
{
   using SignatureBitset = void (*)(class IO&, T&);

   template <typename U>
   static char test(SameType<SignatureBitset, &U::bitset>*);

   template <typename U>
   static double test(...);

public:
   static bool const value = (sizeof(test<ScalarBitSetTraits<T>>(nullptr)) == 1);
};

// Test if ScalarTraits<T> is defined on type T.
template <class T>
struct HasScalarTraits
{
   using SignatureInput = StringRef (*)(StringRef, void*, T&);
   using SignatureOutput = void (*)(const T&, void*, RawOutStream&);
   using SignatureMustQuote = QuotingType (*)(StringRef);

   template <typename U>
   static char test(SameType<SignatureInput, &U::input> *,
                    SameType<SignatureOutput, &U::output> *,
                    SameType<SignatureMustQuote, &U::mustQuote> *);

   template <typename U>
   static double test(...);

public:
   static bool const value =
         (sizeof(test<ScalarTraits<T>>(nullptr, nullptr, nullptr)) == 1);
};

// Test if BlockScalarTraits<T> is defined on type T.
template <class T>
struct HasBlockScalarTraits
{
   using SignatureInput = StringRef (*)(StringRef, void *, T &);
   using SignatureOutput = void (*)(const T &, void *, RawOutStream &);

   template <typename U>
   static char test(SameType<SignatureInput, &U::input> *,
                    SameType<SignatureOutput, &U::output> *);

   template <typename U>
   static double test(...);

public:
   static bool const value =
         (sizeof(test<BlockScalarTraits<T>>(nullptr, nullptr)) == 1);
};

// Test if MappingContextTraits<T> is defined on type T.
template <class T, class Context>
struct HasMappingTraits
{
   using SignatureMapping = void (*)(class IO &, T &, Context &);

   template <typename U>
   static char test(SameType<SignatureMapping, &U::mapping>*);

   template <typename U>
   static double test(...);

public:
   static bool const value =
         (sizeof(test<MappingContextTraits<T, Context>>(nullptr)) == 1);
};

// Test if MappingTraits<T> is defined on type T.
template <class T>
struct HasMappingTraits<T, EmptyContext>
{
   using SignatureMapping = void (*)(class IO &, T &);

   template <typename U>
   static char test(SameType<SignatureMapping, &U::mapping> *);

   template <typename U> static double test(...);

public:
   static bool const value = (sizeof(test<MappingTraits<T>>(nullptr)) == 1);
};

// Test if MappingContextTraits<T>::validate() is defined on type T.
template <class T, class Context>
struct HasMappingValidateTraits
{
   using SignatureValidate = StringRef (*)(class IO &, T &, Context &);

   template <typename U>
   static char test(SameType<SignatureValidate, &U::validate>*);

   template <typename U>
   static double test(...);

public:
   static bool const value =
         (sizeof(test<MappingContextTraits<T, Context>>(nullptr)) == 1);
};

// Test if MappingTraits<T>::validate() is defined on type T.
template <class T>
struct HasMappingValidateTraits<T, EmptyContext> {
   using SignatureValidate = StringRef (*)(class IO &, T &);

   template <typename U>
   static char test(SameType<SignatureValidate, &U::validate> *);

   template <typename U> static double test(...);

public:
   static bool const value = (sizeof(test<MappingTraits<T>>(nullptr)) == 1);
};

// Test if SequenceTraits<T> is defined on type T.
template <class T>
struct HasSequenceMethodTraits
{
   using SignatureSize = size_t (*)(class IO&, T&);

   template <typename U>
   static char test(SameType<SignatureSize, &U::size>*);

   template <typename U>
   static double test(...);

public:
   static bool const value =  (sizeof(test<SequenceTraits<T>>(nullptr)) == 1);
};

// Test if CustomMappingTraits<T> is defined on type T.
template <class T>
struct HasCustomMappingTraits
{
   using SignatureInput = void (*)(IO &io, StringRef key, T &v);

   template <typename U>
   static char test(SameType<SignatureInput, &U::inputOne>*);

   template <typename U>
   static double test(...);

public:
   static bool const value =
         (sizeof(test<CustomMappingTraits<T>>(nullptr)) == 1);
};

// HasFlowTraits<int> will cause an error with some compilers because
// it subclasses int.  Using this wrapper only instantiates the
// real HasFlowTraits only if the template type is a class.
template <typename T, bool Enabled = std::is_class<T>::value>
class HasFlowTraits
{
public:
   static const bool value = false;
};

// Some older gcc compilers don't support straight forward tests
// for members, so test for ambiguity cause by the base and derived
// classes both defining the member.
template <class T>
struct HasFlowTraits<T, true>
{
   struct Fallback
   {
      bool m_flow;
   };

   struct Derived : T, Fallback
   {};

   template<typename C>
   static char (&f(SameType<bool Fallback::*, &C::m_flow>*))[1];

   template<typename C>
   static char (&f(...))[2];

public:
   static bool const value = sizeof(f<Derived>(nullptr)) == 2;
};

// Test if SequenceTraits<T> is defined on type T
template<typename T>
struct HasSequenceTraits : public std::integral_constant<bool,
      HasSequenceMethodTraits<T>::value >
{};

// Test if DocumentListTraits<T> is defined on type T
template <class T>
struct has_DocumentListTraits
{
   using SignatureSize = size_t (*)(class IO &, T &);

   template <typename U>
   static char test(SameType<SignatureSize, &U::size>*);

   template <typename U>
   static double test(...);

public:
   static bool const value = (sizeof(test<DocumentListTraits<T>>(nullptr))==1);
};

inline bool is_number(StringRef str)
{
   static const char octalChars[] = "01234567";
   if (str.startsWith("0") &&
       str.dropFront().findFirstNotOf(octalChars) == StringRef::npos) {
      return true;
   }
   if (str.startsWith("0o") &&
       str.dropFront(2).findFirstNotOf(octalChars) == StringRef::npos) {
      return true;
   }
   static const char hexChars[] = "0123456789abcdefABCDEF";
   if (str.startsWith("0x") &&
       str.dropFront(2).findFirstNotOf(hexChars) == StringRef::npos) {
      return true;
   }
   static const char DecChars[] = "0123456789";
   if (str.findFirstNotOf(DecChars) == StringRef::npos) {
      return true;
   }
   if (str.equals(".inf") || str.equals(".Inf") || str.equals(".INF")) {
      return true;
   }
   std::regex regex("^(\\.[0-9]+|[0-9]+(\\.[0-9]*)?)([eE][-+]?[0-9]+)?$");
   std::string text(str.getData(), str.getSize());
   std::smatch matches;
   if (std::regex_match(text, matches, regex)) {
      return true;
   }
   return false;
}

inline bool is_numeric(StringRef str)
{
   if ((str.front() == '-' || str.front() == '+') && is_number(str.dropFront())) {
      return true;
   }
   if (is_number(str)) {
      return true;
   }
   if (str.equals(".nan") || str.equals(".NaN") || str.equals(".NAN")) {
      return true;
   }
   return false;
}

inline bool is_null(StringRef str)
{
   return str.equals("null") || str.equals("Null") || str.equals("NULL") ||
         str.equals("~");
}

inline bool is_bool(StringRef str)
{
   return str.equals("true") || str.equals("True") || str.equals("TRUE") ||
         str.equals("false") || str.equals("False") || str.equals("FALSE");
}

// 5.1. Character Set
// The allowed character range explicitly excludes the C0 control block #x0-#x1F
// (except for TAB #x9, LF #xA, and CR #xD which are allowed), DEL #x7F, the C1
// control block #x80-#x9F (except for NEL #x85 which is allowed), the surrogate
// block #xD800-#xDFFF, #xFFFE, and #xFFFF.
inline QuotingType needs_quotes(StringRef str)
{
   if (str.empty()) {
      return QuotingType::Single;
   }
   if (isspace(str.front()) || isspace(str.back())) {
      return QuotingType::Single;
   }
   if (is_null(str)) {
      return QuotingType::Single;
   }
   if (is_bool(str)) {
      return QuotingType::Single;
   }
   if (is_numeric(str)) {
      return QuotingType::Single;
   }

   // 7.3.3 Plain Style
   // Plain scalars must not begin with most indicators, as this would cause
   // ambiguity with other YAML constructs.
   static constexpr char indicators[] = R"(-?:\,[]{}#&*!|>'"%@`)";
   if (str.findFirstOf(indicators) == 0) {
      return QuotingType::Single;
   }

   QuotingType maxQuotingNeeded = QuotingType::None;
   for (unsigned char c : str) {
      // Alphanum is safe.
      if (is_alnum(c)) {
         continue;
      }
      switch (c) {
      // Safe scalar characters.
      case '_':
      case '-':
      case '/':
      case '^':
      case '.':
      case ',':
      case ' ':
         // TAB (0x9) is allowed in unquoted strings.
      case 0x9:
         continue;
         // LF(0xA) and CR(0xD) may delimit values and so require at least single
         // quotes.
      case 0xA:
      case 0xD:
         maxQuotingNeeded = QuotingType::Single;
         continue;
         // DEL (0x7F) are excluded from the allowed character range.
      case 0x7F:
         return QuotingType::Double;
      default: {
         // C0 control block (0x0 - 0x1F) is excluded from the allowed character
         // range.
         if (c <= 0x1F) {
            return QuotingType::Double;
         }
         // Always double quote UTF-8.
         if ((c & 0x80) != 0) {
            return QuotingType::Double;
         }
         // The character is not safe, at least simple quoting needed.
         maxQuotingNeeded = QuotingType::Single;
      }
      }
   }
   return maxQuotingNeeded;
}

template <typename T, typename Context>
struct missingTraits
      : public std::integral_constant<bool,
      !HasScalarEnumerationTraits<T>::value &&
      !HasScalarBitSetTraits<T>::value &&
      !HasScalarTraits<T>::value &&
      !HasBlockScalarTraits<T>::value &&
      !HasMappingTraits<T, Context>::value &&
      !HasSequenceTraits<T>::value &&
      !HasCustomMappingTraits<T>::value &&
      !has_DocumentListTraits<T>::value>
{};

template <typename T, typename Context>
struct validatedMappingTraits
      : public std::integral_constant<
      bool, HasMappingTraits<T, Context>::value &&
      HasMappingValidateTraits<T, Context>::value>
{};

template <typename T, typename Context>
struct unvalidatedMappingTraits
      : public std::integral_constant<
      bool, HasMappingTraits<T, Context>::value &&
      !HasMappingValidateTraits<T, Context>::value>
{};

// Base class for Input and Output.
class IO
{
public:
   IO(void *m_context = nullptr);
   virtual ~IO();

   virtual bool outputting() = 0;

   virtual unsigned beginSequence() = 0;
   virtual bool preflightElement(unsigned, void *&) = 0;
   virtual void postflightElement(void*) = 0;
   virtual void endSequence() = 0;
   virtual bool canElideEmptySequence() = 0;

   virtual unsigned beginFlowSequence() = 0;
   virtual bool preflightFlowElement(unsigned, void *&) = 0;
   virtual void postflightFlowElement(void*) = 0;
   virtual void endFlowSequence() = 0;

   virtual bool mapTag(StringRef tag, bool defaultValue = false) = 0;
   virtual void beginMapping() = 0;
   virtual void endMapping() = 0;
   virtual bool preflightKey(const char*, bool, bool, bool &, void *&) = 0;
   virtual void postflightKey(void*) = 0;
   virtual std::vector<StringRef> keys() = 0;

   virtual void beginFlowMapping() = 0;
   virtual void endFlowMapping() = 0;

   virtual void beginEnumScalar() = 0;
   virtual bool matchEnumScalar(const char*, bool) = 0;
   virtual bool matchEnumFallback() = 0;
   virtual void endEnumScalar() = 0;

   virtual bool beginBitSetScalar(bool &) = 0;
   virtual bool bitSetMatch(const char*, bool) = 0;
   virtual void endBitSetScalar() = 0;

   virtual void scalarString(StringRef &, QuotingType) = 0;
   virtual void blockScalarString(StringRef &) = 0;

   virtual void setError(const Twine &) = 0;

   template <typename T>
   void enumCase(T &value, const char* str, const T constVal)
   {
      if (matchEnumScalar(str, outputting() && value == constVal)) {
         value = constVal;
      }
   }

   // allow anonymous enum values to be used with POLAR_YAML_STRONG_TYPEDEF
   template <typename T>
   void enumCase(T &value, const char *str, const uint32_t constVal)
   {
      if (matchEnumScalar(str, outputting() && value == static_cast<T>(constVal))) {
         value = constVal;
      }
   }

   template <typename FBT, typename T>
   void enumFallback(T &value)
   {
      if (matchEnumFallback()) {
         EmptyContext context;
         // FIXME: Force integral conversion to allow strong typedefs to convert.
         FBT res = static_cast<typename FBT::BaseType>(value);
         yamlize(*this, res, true, context);
         value = static_cast<T>(static_cast<typename FBT::BaseType>(res));
      }
   }

   template <typename T>
   void bitSetCase(T &value, const char *str, const T constVal)
   {
      if (bitSetMatch(str, outputting() && (value & constVal) == constVal)) {
         value = static_cast<T>(value | constVal);
      }
   }

   // allow anonymous enum values to be used with POLAR_YAML_STRONG_TYPEDEF
   template <typename T>
   void bitSetCase(T &value, const char *str, const uint32_t constVal)
   {
      if (bitSetMatch(str, outputting() && (value & constVal) == constVal)) {
         value = static_cast<T>(value | constVal);
      }
   }

   template <typename T>
   void maskedBitSetCase(T &value, const char *str, T constVal, T mask)
   {
      if (bitSetMatch(str, outputting() && (value & mask) == constVal)) {
         value = value | constVal;
      }
   }

   template <typename T>
   void maskedBitSetCase(T &value, const char *str, uint32_t constVal,
                         uint32_t mask)
   {
      if (bitSetMatch(str, outputting() && (value & mask) == constVal)) {
         value = value | constVal;
      }
   }

   void *getContext();
   void setContext(void *);

   template <typename T> void mapRequired(const char *key, T &value)
   {
      EmptyContext contex;
      this->processKey(key, value, true, contex);
   }

   template <typename T, typename Context>
   void mapRequired(const char *key, T &value, Context &context)
   {
      this->processKey(key, value, true, context);
   }

   template <typename T> void mapOptional(const char *key, T &value)
   {
      EmptyContext context;
      mapOptionalWithContext(key, value, context);
   }

   template <typename T>
   void mapOptional(const char *key, T &value, const T &defaultValue)
   {
      EmptyContext context;
      mapOptionalWithContext(key, value, defaultValue, context);
   }

   template <typename T, typename Context>
   typename std::enable_if<HasSequenceTraits<T>::value, void>::type
   mapOptionalWithContext(const char *key, T &value, Context &context)
   {
      // omit key/value instead of outputting empty sequence
      if (this->canElideEmptySequence() && !(value.begin() != value.end())) {
         return;
      }
      this->processKey(key, value, false, context);
   }

   template <typename T, typename Context>
   void mapOptionalWithContext(const char *key, std::optional<T> &value, Context &context)
   {
      this->processKeyWithDefault(key, value, std::optional<T>(), /*Required=*/false,
                                  context);
   }

   template <typename T, typename Context>
   typename std::enable_if<!HasSequenceTraits<T>::value, void>::type
   mapOptionalWithContext(const char *Key, T &value, Context &context)
   {
      this->processKey(Key, value, false, context);
   }

   template <typename T, typename Context>
   void mapOptionalWithContext(const char *key, T &value, const T &defaultValue,
                               Context &context)
   {
      this->processKeyWithDefault(key, value, defaultValue, false, context);
   }

private:
   template <typename T, typename Context>
   void processKeyWithDefault(const char *key, std::optional<T> &value,
                              const std::optional<T> &defaultValue, bool required,
                              Context &context)
   {
      assert(defaultValue.has_value() == false &&
             "Optional<T> shouldn't have a value!");
      void *saveInfo;
      bool useDefault = true;
      const bool sameAsDefault = outputting() && !value.has_value();
      if (!outputting() && !value.has_value()) {
         value = T();
      }

      if (value.has_value() &&
          this->preflightKey(key, required, sameAsDefault, useDefault,
                             saveInfo)) {
         yamlize(*this, value.value(), required, context);
         this->postflightKey(saveInfo);
      } else {
         if (useDefault) {
            value = defaultValue;
         }
      }
   }

   template <typename T, typename Context>
   void processKeyWithDefault(const char *key, T &value, const T &defaultValue,
                              bool required, Context &context)
   {
      void *saveInfo;
      bool useDefault;
      const bool sameAsDefault = outputting() && value == defaultValue;
      if ( this->preflightKey(key, required, sameAsDefault, useDefault,
                              saveInfo) ) {
         yamlize(*this, value, required, context);
         this->postflightKey(saveInfo);
      }
      else {
         if (useDefault) {
            value = defaultValue;
         }
      }
   }

   template <typename T, typename Context>
   void processKey(const char *key, T &value, bool required, Context &context)
   {
      void *saveInfo;
      bool useDefault;
      if ( this->preflightKey(key, required, false, useDefault, saveInfo)) {
         yamlize(*this, value, required, context);
         this->postflightKey(saveInfo);
      }
   }

private:
   void *m_context;
};

namespace internal {

template <typename T, typename Context>
void do_mapping(IO &io, T &value, Context &context)
{
   MappingContextTraits<T, Context>::mapping(io, value, context);
}

template <typename T> void do_mapping(IO &io, T &value, EmptyContext &context)
{
   MappingTraits<T>::mapping(io, value);
}

} // end namespace internal

template <typename T>
typename std::enable_if<HasScalarEnumerationTraits<T>::value, void>::type
yamlize(IO &io, T &value, bool, EmptyContext &context)
{
   io.beginEnumScalar();
   ScalarEnumerationTraits<T>::enumeration(io, value);
   io.endEnumScalar();
}

template <typename T>
typename std::enable_if<HasScalarBitSetTraits<T>::value, void>::type
yamlize(IO &io, T &value, bool, EmptyContext &context) {
   bool doClear;
   if (io.beginBitSetScalar(doClear)) {
      if (doClear) {
         value = static_cast<T>(0);
      }
      ScalarBitSetTraits<T>::bitset(io, value);
      io.endBitSetScalar();
   }
}

template <typename T>
typename std::enable_if<HasScalarTraits<T>::value, void>::type
yamlize(IO &io, T &value, bool, EmptyContext &context)
{
   if ( io.outputting() ) {
      std::string storage;
      RawStringOutStream buffer(storage);
      ScalarTraits<T>::output(value, io.getContext(), buffer);
      StringRef str = buffer.getStr();
      io.scalarString(str, ScalarTraits<T>::mustQuote(str));
   } else {
      StringRef str;
      io.scalarString(str, ScalarTraits<T>::mustQuote(str));
      StringRef result = ScalarTraits<T>::input(str, io.getContext(), value);
      if (!result.empty()) {
         io.setError(Twine(result));
      }
   }
}

template <typename T>
typename std::enable_if<HasBlockScalarTraits<T>::value, void>::type
yamlize(IO &yamlIo, T &value, bool, EmptyContext &context)
{
   if (yamlIo.outputting()) {
      std::string storage;
      RawStringOutStream buffer(storage);
      BlockScalarTraits<T>::output(value, yamlIo.getContext(), buffer);
      StringRef str = buffer.getStr();
      yamlIo.blockScalarString(str);
   } else {
      StringRef str;
      yamlIo.blockScalarString(str);
      StringRef result =
            BlockScalarTraits<T>::input(str, yamlIo.getContext(), value);
      if (!result.empty()) {
         yamlIo.setError(Twine(result));
      }
   }
}

template <typename T, typename Context>
typename std::enable_if<validatedMappingTraits<T, Context>::value, void>::type
yamlize(IO &io, T &value, bool, Context &context)
{
   if (HasFlowTraits<MappingTraits<T>>::value) {
      io.beginFlowMapping();
   } else {
      io.beginMapping();
   }
   if (io.outputting()) {
      StringRef error = MappingTraits<T>::validate(io, value);
      if (!error.empty()) {
         error_stream() << error << "\n";
         assert(error.empty() && "invalid struct trying to be written as yaml");
      }
   }
   internal::do_mapping(io, value, context);
   if (!io.outputting()) {
      StringRef error = MappingTraits<T>::validate(io, value);
      if (!error.empty()) {
         io.setError(error);
      }
   }
   if (HasFlowTraits<MappingTraits<T>>::value) {
      io.endFlowMapping();
   } else {
      io.endMapping();
   }
}

template <typename T, typename Context>
typename std::enable_if<unvalidatedMappingTraits<T, Context>::value, void>::type
yamlize(IO &io, T &value, bool, Context &context)
{
   if (HasFlowTraits<MappingTraits<T>>::value) {
      io.beginFlowMapping();
      internal::do_mapping(io, value, context);
      io.endFlowMapping();
   } else {
      io.beginMapping();
      internal::do_mapping(io, value, context);
      io.endMapping();
   }
}

template <typename T>
typename std::enable_if<HasCustomMappingTraits<T>::value, void>::type
yamlize(IO &io, T &value, bool, EmptyContext &context) {
   if ( io.outputting() ) {
      io.beginMapping();
      CustomMappingTraits<T>::output(io, value);
      io.endMapping();
   } else {
      io.beginMapping();
      for (StringRef key : io.keys()) {
         CustomMappingTraits<T>::inputOne(io, key, value);
      }
      io.endMapping();
   }
}

template <typename T>
typename std::enable_if<missingTraits<T, EmptyContext>::value, void>::type
yamlize(IO &io, T &value, bool, EmptyContext &context)
{
   char missing_yaml_trait_for_type[sizeof(MissingTrait<T>)];
}

template <typename T, typename Context>
typename std::enable_if<HasSequenceTraits<T>::value, void>::type
yamlize(IO &io, T &seq, bool, Context &context)
{
   if ( HasFlowTraits< SequenceTraits<T>>::value ) {
      unsigned incnt = io.beginFlowSequence();
      unsigned count = io.outputting() ? SequenceTraits<T>::size(io, seq) : incnt;
      for(unsigned i = 0; i < count; ++i) {
         void *saveInfo;
         if (io.preflightFlowElement(i, saveInfo)) {
            yamlize(io, SequenceTraits<T>::element(io, seq, i), true, context);
            io.postflightFlowElement(saveInfo);
         }
      }
      io.endFlowSequence();
   }
   else {
      unsigned incnt = io.beginSequence();
      unsigned count = io.outputting() ? SequenceTraits<T>::size(io, seq) : incnt;
      for(unsigned i=0; i < count; ++i) {
         void *saveInfo;
         if (io.preflightElement(i, saveInfo)) {
            yamlize(io, SequenceTraits<T>::element(io, seq, i), true, context);
            io.postflightElement(saveInfo);
         }
      }
      io.endSequence();
   }
}

template<>
struct ScalarTraits<bool>
{
   static void output(const bool &, void* , RawOutStream &);
   static StringRef input(StringRef, void *, bool &);
   static QuotingType mustQuote(StringRef)
   {
      return QuotingType::None;
   }
};

template<>
struct ScalarTraits<StringRef>
{
   static void output(const StringRef &, void *, RawOutStream &);
   static StringRef input(StringRef, void *, StringRef &);
   static QuotingType mustQuote(StringRef str)
   {
      return needs_quotes(str);
   }
};

template<>
struct ScalarTraits<std::string>
{
   static void output(const std::string &, void *, RawOutStream &);
   static StringRef input(StringRef, void *, std::string &);
   static QuotingType mustQuote(StringRef str)
   {
      return needs_quotes(str);
   }
};

template<>
struct ScalarTraits<uint8_t> {
   static void output(const uint8_t &, void *, RawOutStream &);
   static StringRef input(StringRef, void *, uint8_t &);
   static QuotingType mustQuote(StringRef)
   {
      return QuotingType::None;
   }
};

template<>
struct ScalarTraits<uint16_t> {
   static void output(const uint16_t &, void *, RawOutStream &);
   static StringRef input(StringRef, void *, uint16_t &);
   static QuotingType mustQuote(StringRef)
   {
      return QuotingType::None;
   }
};

template<>
struct ScalarTraits<uint32_t> {
   static void output(const uint32_t &, void *, RawOutStream &);
   static StringRef input(StringRef, void *, uint32_t &);
   static QuotingType mustQuote(StringRef)
   {
      return QuotingType::None;
   }
};

template<>
struct ScalarTraits<uint64_t> {
   static void output(const uint64_t &, void *, RawOutStream &);
   static StringRef input(StringRef, void *, uint64_t &);
   static QuotingType mustQuote(StringRef)
   {
      return QuotingType::None;
   }
};

template<>
struct ScalarTraits<int8_t> {
   static void output(const int8_t &, void *, RawOutStream &);
   static StringRef input(StringRef, void *, int8_t &);
   static QuotingType mustQuote(StringRef)
   {
      return QuotingType::None;
   }
};

template<>
struct ScalarTraits<int16_t> {
   static void output(const int16_t &, void *, RawOutStream &);
   static StringRef input(StringRef, void *, int16_t &);
   static QuotingType mustQuote(StringRef)
   {
      return QuotingType::None;
   }
};

template<>
struct ScalarTraits<int32_t>
{
   static void output(const int32_t &, void *, RawOutStream &);
   static StringRef input(StringRef, void *, int32_t &);
   static QuotingType mustQuote(StringRef)
   {
      return QuotingType::None;
   }
};

template<>
struct ScalarTraits<int64_t>
{
   static void output(const int64_t &, void *, RawOutStream &);
   static StringRef input(StringRef, void *, int64_t &);
   static QuotingType mustQuote(StringRef)
   {
      return QuotingType::None;
   }
};

template<>
struct ScalarTraits<float>
{
   static void output(const float &, void *, RawOutStream &);
   static StringRef input(StringRef, void *, float &);
   static QuotingType mustQuote(StringRef)
   {
      return QuotingType::None;
   }
};

template<>
struct ScalarTraits<double> {
   static void output(const double &, void *, RawOutStream &);
   static StringRef input(StringRef, void *, double &);
   static QuotingType mustQuote(StringRef)
   {
      return QuotingType::None;
   }
};

// For endian types, we just use the existing ScalarTraits for the underlying
// type.  This way endian aware types are supported whenever a ScalarTraits
// is defined for the underlying type.
template <typename value_type, polar::utils::Endianness defaultEndian, size_t alignment>
struct ScalarTraits<PackedEndianSpecificIntegral<
      value_type, defaultEndian, alignment>>
{
   using EndianType =
   PackedEndianSpecificIntegral<value_type, defaultEndian,
   alignment>;

   static void output(const EndianType &endian, void *context, RawOutStream &stream)
   {
      ScalarTraits<value_type>::output(static_cast<value_type>(endian), context, stream);
   }

   static StringRef input(StringRef str, void *context, EndianType &endian)
   {
      value_type value;
      auto ret = ScalarTraits<value_type>::input(str, context, value);
      endian = static_cast<EndianType>(value);
      return ret;
   }

   static QuotingType mustQuote(StringRef str)
   {
      return ScalarTraits<value_type>::mustQuote(str);
   }
};

// Utility for use within MappingTraits<>::mapping() method
// to [de]normalize an object for use with YAML conversion.
template <typename TNorm, typename TFinal>
struct MappingNormalization
{
   MappingNormalization(IO &io, TFinal &obj)
      : m_io(io), m_bufPtr(nullptr), m_result(obj)
   {
      if (m_io.outputting() ) {
         m_bufPtr = new (&m_buffer) TNorm(io, obj);
      }
      else {
         m_bufPtr = new (&m_buffer) TNorm(io);
      }
   }

   ~MappingNormalization()
   {
      if (!m_io.outputting()) {
         m_result = m_bufPtr->denormalize(m_io);
      }
      m_bufPtr->~TNorm();
   }

   TNorm* operator->()
   {
      return m_bufPtr;
   }

private:
   using Storage = AlignedCharArrayUnion<TNorm>;

   Storage      m_buffer;
   IO           &m_io;
   TNorm        *m_bufPtr;
   TFinal       &m_result;
};

// Utility for use within MappingTraits<>::mapping() method
// to [de]normalize an object for use with YAML conversion.
template <typename TNorm, typename TFinal>
struct MappingNormalizationHeap
{
   MappingNormalizationHeap(IO &io, TFinal &obj, BumpPtrAllocator *allocator)
      : m_io(io), m_result(obj)
   {
      if (m_io.outputting()) {
         m_bufPtr = new (&m_buffer) TNorm(m_io, obj);
      }
      else if (allocator) {
         m_bufPtr = allocator->allocate<TNorm>();
         new (m_bufPtr) TNorm(m_io);
      } else {
         m_bufPtr = new TNorm(m_io);
      }
   }

   ~MappingNormalizationHeap()
   {
      if (m_io.outputting()) {
         m_bufPtr->~TNorm();
      }
      else {
         m_result = m_bufPtr->denormalize(m_io);
      }
   }

   TNorm* operator->()
   {
      return m_bufPtr;
   }

private:
   using Storage = AlignedCharArrayUnion<TNorm>;

   Storage      m_buffer;
   IO           &m_io;
   TNorm        *m_bufPtr = nullptr;
   TFinal       &m_result;
};

///
/// The Input class is used to parse a yaml document into in-memory structs
/// and vectors.
///
/// It works by using YAMLParser to do a syntax parse of the entire yaml
/// document, then the Input class builds a graph of HNodes which wraps
/// each yaml Node.  The extra layer is buffering.  The low level yaml
/// parser only lets you look at each node once.  The buffering layer lets
/// you search and interate multiple times.  This is necessary because
/// the mapRequired() method calls may not be in the same order
/// as the keys in the document.
///
class Input : public IO
{
public:
   // Construct a yaml Input object from a StringRef and optional
   // user-data. The DiagHandler can be specified to provide
   // alternative error reporting.
   Input(StringRef inputContent,
         void *context = nullptr,
         SourceMgr::DiagHandlerTy diagHandler = nullptr,
         void *diagHandlerContext = nullptr);
   Input(MemoryBufferRef input,
         void *context = nullptr,
         SourceMgr::DiagHandlerTy diagHandler = nullptr,
         void *diagHandlerContext = nullptr);
   ~Input() override;

   // Check if there was an syntax or semantic error during parsing.
   std::error_code getError();

private:
   bool outputting() override;
   bool mapTag(StringRef, bool) override;
   void beginMapping() override;
   void endMapping() override;
   bool preflightKey(const char *, bool, bool, bool &, void *&) override;
   void postflightKey(void *) override;
   std::vector<StringRef> keys() override;
   void beginFlowMapping() override;
   void endFlowMapping() override;
   unsigned beginSequence() override;
   void endSequence() override;
   bool preflightElement(unsigned index, void *&) override;
   void postflightElement(void *) override;
   unsigned beginFlowSequence() override;
   bool preflightFlowElement(unsigned , void *&) override;
   void postflightFlowElement(void *) override;
   void endFlowSequence() override;
   void beginEnumScalar() override;
   bool matchEnumScalar(const char*, bool) override;
   bool matchEnumFallback() override;
   void endEnumScalar() override;
   bool beginBitSetScalar(bool &) override;
   bool bitSetMatch(const char *, bool ) override;
   void endBitSetScalar() override;
   void scalarString(StringRef &, QuotingType) override;
   void blockScalarString(StringRef &) override;
   void setError(const Twine &message) override;
   bool canElideEmptySequence() override;

   class HNode
   {
      virtual void anchor();

   public:
      HNode(Node *node) : m_node(node)
      {}
      virtual ~HNode() = default;

      static bool classof(const HNode *)
      {
         return true;
      }

      Node *m_node;
   };

   class EmptyHNode : public HNode
   {
      void anchor() override;

   public:
      EmptyHNode(Node *node) : HNode(node)
      {}

      static bool classof(const HNode *node)
      {
         return NullNode::classof(node->m_node);
      }

      static bool classof(const EmptyHNode *)
      {
         return true;
      }
   };

   class ScalarHNode : public HNode
   {
      void anchor() override;

   public:
      ScalarHNode(Node *node, StringRef str) : HNode(node), m_value(str)
      {}

      StringRef value() const { return m_value; }

      static bool classof(const HNode *node)
      {
         return ScalarNode::classof(node->m_node) ||
               BlockScalarNode::classof(node->m_node);
      }

      static bool classof(const ScalarHNode *)
      {
         return true;
      }

   protected:
      StringRef m_value;
   };

   class MapHNode : public HNode
   {
      void anchor() override;

   public:
      MapHNode(Node *node) : HNode(node)
      {}

      static bool classof(const HNode *node)
      {
         return MappingNode::classof(node->m_node);
      }

      static bool classof(const MapHNode *)
      {
         return true;
      }

      using NameToNode = StringMap<std::unique_ptr<HNode>>;

      NameToNode m_mapping;
      SmallVector<std::string, 6> m_validKeys;
   };

   class SequenceHNode : public HNode
   {
      void anchor() override;

   public:
      SequenceHNode(Node *node) : HNode(node)
      {}

      static bool classof(const HNode *node)
      {
         return SequenceNode::classof(node->m_node);
      }

      static bool classof(const SequenceHNode *)
      {
         return true;
      }

      std::vector<std::unique_ptr<HNode>> m_entries;
   };

   std::unique_ptr<Input::HNode> createHNodes(Node *node);
   void setError(HNode *hnode, const Twine &message);
   void setError(Node *node, const Twine &message);

public:
   // These are only used by operator>>. They could be private
   // if those templated things could be made friends.
   bool setCurrentDocument();
   bool nextDocument();

   /// Returns the current node that's being parsed by the YAML Parser.
   const Node *getCurrentNode() const;

private:
   SourceMgr                           m_srcMgr; // must be before m_strm
   std::unique_ptr<polar::yaml::Stream> m_strm;
   std::unique_ptr<HNode>              m_topNode;
   std::error_code                     m_errorCode;
   BumpPtrAllocator                    m_stringAllocator;
   DocumentIterator                   m_docIterator;
   std::vector<bool>                   m_bitValuesUsed;
   HNode *m_currentNode = nullptr;
   bool                                m_scalarMatchFound;
};

///
/// The Output class is used to generate a yaml document from in-memory structs
/// and vectors.
///
class Output : public IO
{
public:
   Output(RawOutStream &, void *context = nullptr, int wrapColumn = 70);
   ~Output() override;

   /// Set whether or not to output optional values which are equal
   /// to the default value.  By default, when outputting if you attempt
   /// to write a value that is equal to the default, the value gets ignored.
   /// Sometimes, it is useful to be able to see these in the resulting YAML
   /// anyway.
   void setWriteDefaultValues(bool write)
   {
      m_writeDefaultValues = write;
   }

   bool outputting() override;
   bool mapTag(StringRef, bool) override;
   void beginMapping() override;
   void endMapping() override;
   bool preflightKey(const char *key, bool, bool, bool &, void *&) override;
   void postflightKey(void *) override;
   std::vector<StringRef> keys() override;
   void beginFlowMapping() override;
   void endFlowMapping() override;
   unsigned beginSequence() override;
   void endSequence() override;
   bool preflightElement(unsigned, void *&) override;
   void postflightElement(void *) override;
   unsigned beginFlowSequence() override;
   bool preflightFlowElement(unsigned, void *&) override;
   void postflightFlowElement(void *) override;
   void endFlowSequence() override;
   void beginEnumScalar() override;
   bool matchEnumScalar(const char*, bool) override;
   bool matchEnumFallback() override;
   void endEnumScalar() override;
   bool beginBitSetScalar(bool &) override;
   bool bitSetMatch(const char *, bool ) override;
   void endBitSetScalar() override;
   void scalarString(StringRef &, QuotingType) override;
   void blockScalarString(StringRef &) override;
   void setError(const Twine &message) override;
   bool canElideEmptySequence() override;

   // These are only used by operator<<. They could be private
   // if that templated operator could be made a friend.
   void beginDocuments();
   bool preflightDocument(unsigned);
   void postflightDocument();
   void endDocuments();

private:
   void output(StringRef s);
   void outputUpToEndOfLine(StringRef s);
   void newLineCheck();
   void outputNewLine();
   void paddedKey(StringRef key);
   void flowKey(StringRef Key);

   enum InState {
      inSeq,
      inFlowSeq,
      inMapFirstKey,
      inMapOtherKey,
      inFlowMapFirstKey,
      inFlowMapOtherKey
   };

   RawOutStream &m_out;
   int m_wrapColumn;
   SmallVector<InState, 8> m_stateStack;
   int m_column = 0;
   int m_columnAtFlowStart = 0;
   int m_columnAtMapFlowStart = 0;
   bool m_needBitValueComma = false;
   bool m_needFlowSequenceComma = false;
   bool m_enumerationMatchFound = false;
   bool m_needsNewLine = false;
   bool m_writeDefaultValues = false;
};

/// YAML I/O does conversion based on types. But often native data types
/// are just a typedef of built in intergral types (e.g. int).  But the C++
/// type matching system sees through the typedef and all the typedefed types
/// look like a built in type. This will cause the generic YAML I/O conversion
/// to be used. To provide better control over the YAML conversion, you can
/// use this macro instead of typedef.  It will create a class with one field
/// and automatic conversion operators to and from the base type.
/// Based on BOOST_STRONG_TYPEDEF
#define POLAR_YAML_STRONG_TYPEDEF(_base, _type)                                 \
   struct _type {                                                             \
   _type() = default;                                                     \
   _type(const _base v) : value(v) {}                                     \
   _type(const _type &v) = default;                                       \
   _type &operator=(const _type &rhs) = default;                          \
   _type &operator=(const _base &rhs) { value = rhs; return *this; }      \
   operator const _base & () const { return value; }                      \
   bool operator==(const _type &rhs) const { return value == rhs.value; } \
   bool operator==(const _base &rhs) const { return value == rhs; }       \
   bool operator<(const _type &rhs) const { return value < rhs.value; }   \
   _base value;                                                           \
   using BaseType = _base;                                                \
};

///
/// Use these types instead of uintXX_t in any mapping to have
/// its yaml output formatted as hexadecimal.
///
POLAR_YAML_STRONG_TYPEDEF(uint8_t, Hex8)
POLAR_YAML_STRONG_TYPEDEF(uint16_t, Hex16)
POLAR_YAML_STRONG_TYPEDEF(uint32_t, Hex32)
POLAR_YAML_STRONG_TYPEDEF(uint64_t, Hex64)

template<>
struct ScalarTraits<Hex8>
{
   static void output(const Hex8 &, void *, RawOutStream &);
   static StringRef input(StringRef, void *, Hex8 &);
   static QuotingType mustQuote(StringRef) { return QuotingType::None; }
};

template<>
struct ScalarTraits<Hex16>
{
   static void output(const Hex16 &, void *, RawOutStream &);
   static StringRef input(StringRef, void *, Hex16 &);
   static QuotingType mustQuote(StringRef) { return QuotingType::None; }
};

template<>
struct ScalarTraits<Hex32>
{
   static void output(const Hex32 &, void *, RawOutStream &);
   static StringRef input(StringRef, void *, Hex32 &);
   static QuotingType mustQuote(StringRef) { return QuotingType::None; }
};

template<>
struct ScalarTraits<Hex64>
{
   static void output(const Hex64 &, void *, RawOutStream &);
   static StringRef input(StringRef, void *, Hex64 &);
   static QuotingType mustQuote(StringRef) { return QuotingType::None; }
};

// Define non-member operator>> so that Input can stream in a document list.
template <typename T>
inline
typename std::enable_if<has_DocumentListTraits<T>::value, Input &>::type
operator>>(Input &yin, T &docList) {
   int i = 0;
   EmptyContext context;
   while ( yin.setCurrentDocument() ) {
      yamlize(yin, DocumentListTraits<T>::element(yin, docList, i), true, context);
      if (yin.getError())
         return yin;
      yin.nextDocument();
      ++i;
   }
   return yin;
}

// Define non-member operator>> so that Input can stream in a map as a document.
template <typename T>
inline typename std::enable_if<HasMappingTraits<T, EmptyContext>::value,
Input &>::type
operator>>(Input &yin, T &docMap)
{
   EmptyContext context;
   yin.setCurrentDocument();
   yamlize(yin, docMap, true, context);
   return yin;
}

// Define non-member operator>> so that Input can stream in a sequence as
// a document.
template <typename T>
inline
typename std::enable_if<HasSequenceTraits<T>::value, Input &>::type
operator>>(Input &yin, T &docSeq)
{
   EmptyContext context;
   if (yin.setCurrentDocument()) {
      yamlize(yin, docSeq, true, context);
   }
   return yin;
}

// Define non-member operator>> so that Input can stream in a block scalar.
template <typename T>
inline
typename std::enable_if<HasBlockScalarTraits<T>::value, Input &>::type
operator>>(Input &input, T &value)
{
   EmptyContext context;
   if (input.setCurrentDocument()) {
      yamlize(input, value, true, context);
   }
   return input;
}

// Define non-member operator>> so that Input can stream in a string map.
template <typename T>
inline
typename std::enable_if<HasCustomMappingTraits<T>::value, Input &>::type
operator>>(Input &input, T &value)
{
   EmptyContext context;
   if (input.setCurrentDocument()) {
      yamlize(input, value, true, context);
   }
   return input;
}

// Provide better error message about types missing a trait specialization
template <typename T>
inline typename std::enable_if<missingTraits<T, EmptyContext>::value,
Input &>::type
operator>>(Input &yin, T &docSeq)
{
   char missing_yaml_trait_for_type[sizeof(MissingTrait<T>)];
   return yin;
}

// Define non-member operator<< so that Output can stream out document list.
template <typename T>
inline
typename std::enable_if<has_DocumentListTraits<T>::value, Output &>::type
operator<<(Output &yout, T &docList)
{
   EmptyContext context;
   yout.beginDocuments();
   const size_t count = DocumentListTraits<T>::size(yout, docList);
   for(size_t i=0; i < count; ++i) {
      if ( yout.preflightDocument(i) ) {
         yamlize(yout, DocumentListTraits<T>::element(yout, docList, i), true,
                 context);
         yout.postflightDocument();
      }
   }
   yout.endDocuments();
   return yout;
}

// Define non-member operator<< so that Output can stream out a map.
template <typename T>
inline typename std::enable_if<HasMappingTraits<T, EmptyContext>::value,
Output &>::type
operator<<(Output &yout, T &map)
{
   EmptyContext context;
   yout.beginDocuments();
   if ( yout.preflightDocument(0) ) {
      yamlize(yout, map, true, context);
      yout.postflightDocument();
   }
   yout.endDocuments();
   return yout;
}

// Define non-member operator<< so that Output can stream out a sequence.
template <typename T>
inline
typename std::enable_if<HasSequenceTraits<T>::value, Output &>::type
operator<<(Output &yout, T &seq)
{
   EmptyContext context;
   yout.beginDocuments();
   if ( yout.preflightDocument(0) ) {
      yamlize(yout, seq, true, context);
      yout.postflightDocument();
   }
   yout.endDocuments();
   return yout;
}

// Define non-member operator<< so that Output can stream out a block scalar.
template <typename T>
inline
typename std::enable_if<HasBlockScalarTraits<T>::value, Output &>::type
operator<<(Output &out, T &value)
{
   EmptyContext context;
   out.beginDocuments();
   if (out.preflightDocument(0)) {
      yamlize(out, value, true, context);
      out.postflightDocument();
   }
   out.endDocuments();
   return out;
}

// Define non-member operator<< so that Output can stream out a string map.
template <typename T>
inline
typename std::enable_if<HasCustomMappingTraits<T>::value, Output &>::type
operator<<(Output &out, T &value)
{
   EmptyContext context;
   out.beginDocuments();
   if (out.preflightDocument(0)) {
      yamlize(out, value, true, context);
      out.postflightDocument();
   }
   out.endDocuments();
   return out;
}

// Provide better error message about types missing a trait specialization
template <typename T>
inline typename std::enable_if<missingTraits<T, EmptyContext>::value,
Output &>::type
operator<<(Output &yout, T &seq)
{
   char missing_yaml_trait_for_type[sizeof(MissingTrait<T>)];
   return yout;
}

template <bool B> struct IsFlowSequenceBase
{};

template <>
struct IsFlowSequenceBase<true>
{
   static const bool flow = true;
};

template <typename T, bool Flow>
struct SequenceTraitsImpl : IsFlowSequenceBase<Flow>
{
private:
   using type = typename T::value_type;

public:
   static size_t size(IO &io, T &seq)
   {
      return seq.size();
   }

   static type &element(IO &io, T &seq, size_t index)
   {
      if (index >= seq.size()) {
         seq.resize(index + 1);
      }
      return seq[index];
   }
};

// Simple helper to check an expression can be used as a bool-valued template
// argument.
template <bool>
struct CheckIsBool
{
   static const bool value = true;
};

// If T has SequenceElementTraits, then vector<T> and SmallVector<T, N> have
// SequenceTraits that do the obvious thing.
template <typename T>
struct SequenceTraits<std::vector<T>,
      typename std::enable_if<CheckIsBool<
      SequenceElementTraits<T>::flow>::value>::type>
      : SequenceTraitsImpl<std::vector<T>, SequenceElementTraits<T>::flow>
{};

template <typename T, unsigned N>
struct SequenceTraits<SmallVector<T, N>,
      typename std::enable_if<CheckIsBool<
      SequenceElementTraits<T>::flow>::value>::type>
      : SequenceTraitsImpl<SmallVector<T, N>, SequenceElementTraits<T>::flow>
{};

// Sequences of fundamental types use flow formatting.
template <typename T>
struct SequenceElementTraits<
      T, typename std::enable_if<std::is_fundamental<T>::value>::type>
{
   static const bool flow = true;
};

// Sequences of strings use block formatting.
template<>
struct SequenceElementTraits<std::string>
{
   static const bool flow = false;
};

template<>
struct SequenceElementTraits<StringRef>
{
   static const bool flow = false;
};

template<>
struct SequenceElementTraits<std::pair<std::string, std::string>>
{
   static const bool flow = false;
};

/// Implementation of CustomMappingTraits for std::map<std::string, T>.
template <typename T>
struct StdMapStringCustomMappingTraitsImpl
{
   using map_type = std::map<std::string, T>;

   static void inputOne(IO &io, StringRef key, map_type &v) {
      io.mapRequired(key.getStr().c_str(), v[key]);
   }

   static void output(IO &io, map_type &v)
   {
      for (auto &p : v) {
         io.mapRequired(p.first.c_str(), p.second);
      }
   }
};

} // yaml
} // polar

#define POLAR_YAML_IS_SEQUENCE_VECTOR_IMPL(TYPE, FLOW)                          \
   namespace polar {                                                             \
   namespace yaml {                                                             \
   static_assert(                                                               \
   !std::is_fundamental<TYPE>::value &&                                     \
   !std::is_same<TYPE, std::string>::value &&                               \
   !std::is_same<TYPE, polar::StringRef>::value,                             \
   "only use POLAR_YAML_IS_SEQUENCE_VECTOR for types you control");          \
   template <> struct SequenceElementTraits<TYPE> {                             \
   static const bool flow = FLOW;                                             \
   };                                                                           \
   }                                                                            \
   }

/// Utility for declaring that a std::vector of a particular type
/// should be considered a YAML sequence.
#define POLAR_YAML_IS_SEQUENCE_VECTOR(type)                                     \
   POLAR_YAML_IS_SEQUENCE_VECTOR_IMPL(type, false)

/// Utility for declaring that a std::vector of a particular type
/// should be considered a YAML flow sequence.
#define POLAR_YAML_IS_FLOW_SEQUENCE_VECTOR(type)                                \
   POLAR_YAML_IS_SEQUENCE_VECTOR_IMPL(type, true)

#define POLAR_YAML_DECLARE_MAPPING_TRAITS(Type)                                 \
   namespace polar {                                                             \
   namespace yaml {                                                             \
   template <> struct MappingTraits<Type> {                                     \
   static void mapping(IO &IO, Type &obj);                                    \
   };                                                                           \
   }                                                                            \
   }

#define POLAR_YAML_DECLARE_ENUM_TRAITS(Type)                                    \
   namespace polar {                                                             \
   namespace yaml {                                                             \
   template <> struct ScalarEnumerationTraits<Type> {                           \
   static void enumeration(IO &io, Type &valueue);                              \
   };                                                                           \
   }                                                                            \
   }

#define POLAR_YAML_DECLARE_BITSET_TRAITS(Type)                                  \
   namespace polar {                                                             \
   namespace yaml {                                                             \
   template <> struct ScalarBitSetTraits<Type> {                                \
   static void bitset(IO &IO, Type &Options);                                 \
   };                                                                           \
   }                                                                            \
   }

#define POLAR_YAML_DECLARE_SCALAR_TRAITS(Type, MustQuote)                       \
   namespace polar {                                                             \
   namespace yaml {                                                             \
   template <> struct ScalarTraits<Type> {                                      \
   static void output(const Type &valueue, void *ctx, RawOutStream &Out);        \
   static StringRef input(StringRef strcalar, void *ctxt, Type &valueue);         \
   static QuotingType mustQuote(StringRef) { return MustQuote; }              \
   };                                                                           \
   }                                                                            \
   }

/// Utility for declaring that a std::vector of a particular type
/// should be considered a YAML document list.
#define POLAR_YAML_IS_DOCUMENT_LIST_VECTOR(_type)                               \
   namespace polar {                                                             \
   namespace yaml {                                                             \
   template <unsigned N>                                                        \
   struct DocumentListTraits<SmallVector<_type, N>>                             \
   : public SequenceTraitsImpl<SmallVector<_type, N>, false> {};            \
   template <>                                                                  \
   struct DocumentListTraits<std::vector<_type>>                                \
   : public SequenceTraitsImpl<std::vector<_type>, false> {};               \
   }                                                                            \
   }

/// Utility for declaring that std::map<std::string, _type> should be considered
/// a YAML map.
#define POLAR_YAML_IS_STRING_MAP(_type)                                         \
   namespace polar {                                                             \
   namespace yaml {                                                             \
   template <>                                                                  \
   struct CustomMappingTraits<std::map<std::string, _type>>                     \
   : public StdMapStringCustomMappingTraitsImpl<_type> {};                  \
   }                                                                            \
   }


#endif // POLAR_UTILS_YAML_TRAITS_H
