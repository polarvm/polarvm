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

#ifndef POLAR_BASIC_ADT_INTRUSIVE_LIST_NodeOptions_H
#define POLAR_BASIC_ADT_INTRUSIVE_LIST_NodeOptions_H

#include "polar/global/AbiBreaking.h"
#include "polar/global/PolarConfig.h"

#include <type_traits>

namespace polar {
namespace basic {

template <bool EnableSentinelTracking>
class IntrusiveListNodeBase;
template <bool EnableSentinelTracking>
class IntrusiveListBase;

/// Option to choose whether to track sentinels.
///
/// This option affects the ABI for the nodes.  When not specified explicitly,
/// the ABI depends on LLVM_ENABLE_ABI_BREAKING_CHECKS.  Specify explicitly to
/// enable \a ilist_node::isSentinel().
template <bool EnableSentinelTracking>
struct IntrusiveListSentinelTracking
{};

/// Option to specify a tag for the node type.
///
/// This option allows a single value type to be inserted in multiple lists
/// simultaneously.  See \a ilist_node for usage examples.
template <class Tag>
struct IntrusiveListTag
{};

namespace ilist_internal {

/// Helper trait for recording whether an option is specified explicitly.
template <bool IsExplicit>
struct Explicitness
{
   static const bool IsExplicit = IsExplicit;
};

using IsExplicit = Explicitness<true>;
using IsImplicit = Explicitness<false>;

/// Check whether an option is valid.
///
/// The steps for adding and enabling a new ilist option include:
/// \li define the option, ilist_foo<Bar>, above;
/// \li add new parameters for Bar to \a ilist_detail::NodeOptions;
/// \li add an extraction meta-function, ilist_detail::extract_foo;
/// \li call extract_foo from \a ilist_detail::ComputeNodeOptions and pass it
/// into \a ilist_detail::NodeOptions; and
/// \li specialize \c IsValidOption<ilist_foo<Bar>> to inherit from \c
/// std::true_type to get static assertions passing in \a simple_ilist and \a
/// ilist_node.
template <class Option>
struct IsValidOption : std::false_type
{};

/// Extract sentinel tracking option.
///
/// Look through \p Options for the \a IntrusiveListSentinelTracking option, with the
/// default depending on LLVM_ENABLE_ABI_BREAKING_CHECKS.
template <class... Options>
struct ExtractSentinelTracking;

template <bool EnableSentinelTracking, class... Options>
struct ExtractSentinelTracking<
      IntrusiveListSentinelTracking<EnableSentinelTracking>, Options...>
      : std::integral_constant<bool, EnableSentinelTracking>, IsExplicit {};
template <class Option1, class... Options>
struct ExtractSentinelTracking<Option1, Options...>
      : ExtractSentinelTracking<Options...> {};
#if POLAR_ENABLE_ABI_BREAKING_CHECKS
template <>
struct ExtractSentinelTracking<> : std::true_type, IsImplicit
{};
#else
template <>
struct ExtractSentinelTracking<> : std::false_type, IsImplicit
{};
#endif
template <bool EnableSentinelTracking>
struct IsValidOption<IntrusiveListSentinelTracking<EnableSentinelTracking>>
      : std::true_type
{};

/// Extract custom tag option.
///
/// Look through \p Options for the \a IntrusiveListTag option, pulling out the
/// custom tag type, using void as a default.
template <class... Options> struct ExtractTag;
template <class Tag, class... Options>
struct ExtractTag<IntrusiveListTag<Tag>, Options...>
{
   typedef Tag type;
};

template <class Option1, class... Options>
struct ExtractTag<Option1, Options...> : ExtractTag<Options...>
{};

template <> struct ExtractTag<> { typedef void type; };
template <class Tag> struct IsValidOption<IntrusiveListTag<Tag>> : std::true_type
{};

/// Check whether options are valid.
///
/// The conjunction of \a IsValidOption on each individual option.
template <class... Options>
struct CheckOptions;

template <>
struct CheckOptions<> : std::true_type
{};

template <class Option1, class... Options>
struct CheckOptions<Option1, Options...>
      : std::integral_constant<bool, IsValidOption<Option1>::value &&
      CheckOptions<Options...>::value>
{};

/// Traits for options for \a ilist_node.
///
/// This is usually computed via \a ComputeNodeOptions.
template <class T, bool EnableSentinelTracking, bool IsSentinelTrackingExplicit,
          class TagType>
struct NodeOptions
{
   using value_type = T;
   using pointer = T *;
   using reference = T &;
   using const_pointer = const T *;
   using const_reference = const T &;
   using tag = TagType;
   
   static const bool sm_enableSentinelSracking = EnableSentinelTracking;
   static const bool sm_isSentinelTrackingExplicit = IsSentinelTrackingExplicit;
   
   using NodeBaseType = IntrusiveListNodeBase<sm_enableSentinelSracking>;
   using ListBaseType = IntrusiveListBase<sm_enableSentinelSracking>;
};

template <class T, class... Options> struct ComputeNodeOptions
{
   using type = NodeOptions<T, ExtractSentinelTracking<Options...>::value,
   ExtractSentinelTracking<Options...>::IsExplicit,
   typename ExtractTag<Options...>::type>;
};

} // basic
} // polar

#endif // POLAR_BASIC_ADT_INTRUSIVE_LIST_NodeOptions_H
