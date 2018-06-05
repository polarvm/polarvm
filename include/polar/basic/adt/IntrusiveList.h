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

//===----------------------------------------------------------------------===//
//
// This file defines classes to implement an intrusive doubly linked list class
// (i.e. each node of the list must contain a next and previous field for the
// list.
//
// The ilist class itself should be a plug in replacement for list.  This list
// replacement does not provide a constant time size() method, so be careful to
// use empty() when you really want to know if it's empty.
//
// The ilist class is implemented as a circular list.  The list itself contains
// a sentinel node, whose Next points at begin() and whose Prev points at
// rbegin().  The sentinel node itself serves as end() and rend().
//
//===----------------------------------------------------------------------===//

#ifndef POLAR_BASIC_ADT_INTRUSIVE_LIST_H
#define POLAR_BASIC_ADT_INTRUSIVE_LIST_H

#include "polar/basic/adt/SimpleIntrusiveList.h"
#include <cassert>
#include <cstddef>
#include <iterator>

namespace polar {
namespace basic {

/// Use delete by default for iplist and ilist.
///
/// Specialize this to get different behaviour for ownership-related API.  (If
/// you really want ownership semantics, consider using std::list or building
/// something like \a BumpPtrList.)
///
/// \see IntrusiveListNoAllocTraits
template <typename NodeType>
struct IntrusiveListAllocTraits
{
   static void deleteNode(NodeType *node)
   {
      delete node;
   }
};

/// Custom traits to do nothing on deletion.
///
/// Specialize IntrusiveListAllocTraits to inherit from this to disable the
/// non-intrusive deletion in iplist (which implies ownership).
///
/// If you want purely intrusive semantics with no callbacks, consider using \a
/// simple_ilist instead.
///
/// \code
/// template <>
/// struct IntrusiveListAllocTraits<MyType> : IntrusiveListNoAllocTraits<MyType> {};
/// \endcode
template <typename NodeType>
struct IntrusiveListNoAllocTraits
{
   static void deleteNode(NodeType *node)
   {}
};

/// Callbacks do nothing by default in iplist and ilist.
///
/// Specialize this for to use callbacks for when nodes change their list
/// membership.
template <typename NodeType>
struct IntrusiveListCallbackTraits {
   void addNodeToList(NodeType *) {}
   void removeNodeFromList(NodeType *) {}
   
   /// Callback before transferring nodes to this list.
   ///
   /// \pre \c this!=&oldList
   template <class Iterator>
   void transferNodesFromList(IntrusiveListCallbackTraits &oldList, Iterator /*first*/,
                              Iterator /*last*/)
   {
      (void)oldList;
   }
};

/// A fragment for template traits for intrusive list that provides default
/// node related operations.
///
/// TODO: Remove this layer of indirection.  It's not necessary.
template <typename NodeType>
struct IntrusiveListNodeTraits : IntrusiveListAllocTraits<NodeType>,
      IntrusiveListCallbackTraits<NodeType>
{};

/// Default template traits for intrusive list.
///
/// By inheriting from this, you can easily use default implementations for all
/// common operations.
///
/// TODO: Remove this customization point.  Specializing IntrusiveListTraits is
/// already fully general.
template <typename NodeType>
struct IntrusiveListDefaultTraits : public IntrusiveListNodeTraits<NodeType>
{};

/// Template traits for intrusive list.
///
/// Customize callbacks and allocation semantics.
template <typename NodeType>
struct IntrusiveListTraits : public IntrusiveListDefaultTraits<NodeType>
{};

/// Const traits should never be instantiated.
template <typename Type>
struct IntrusiveListTraits<const Type>
{};

namespace ilist_internal {

template <class T> T &make();

/// Type trait to check for a traits class that has a getNext member (as a
/// canary for any of the ilist_nextprev_traits API).
template <class TraitsT, class NodeT> struct HasGetNext
{
   typedef char Yes[1];
   typedef char No[2];
   template <size_t N> struct SFINAE {};
   
   template <class U>
   static Yes &test(U *I, decltype(I->getNext(&make<NodeT>())) * = 0);
   template <class> static No &test(...);
   
public:
   static const bool value = sizeof(test<TraitsT>(nullptr)) == sizeof(Yes);
};

/// Type trait to check for a traits class that has a createSentinel member (as
/// a canary for any of the ilist_sentinel_traits API).
template <class TraitsT> struct HasCreateSentinel {
   typedef char Yes[1];
   typedef char No[2];
   
   template <class U>
   static Yes &test(U *I, decltype(I->createSentinel()) * = 0);
   template <class> static No &test(...);
   
public:
   static const bool value = sizeof(test<TraitsT>(nullptr)) == sizeof(Yes);
};

/// Type trait to check for a traits class that has a createNode member.
/// Allocation should be managed in a wrapper class, instead of in
/// IntrusiveListTraits.
template <class TraitsT, class NodeT> struct HasCreateNode {
   typedef char Yes[1];
   typedef char No[2];
   template <size_t N> struct SFINAE {};
   
   template <class U>
   static Yes &test(U *I, decltype(I->createNode(make<NodeT>())) * = 0);
   template <class> static No &test(...);
   
public:
   static const bool value = sizeof(test<TraitsT>(nullptr)) == sizeof(Yes);
};

template <class TraitsT, class NodeT> struct HasObsoleteCustomization {
   static const bool value = HasGetNext<TraitsT, NodeT>::value ||
   HasCreateSentinel<TraitsT>::value ||
   HasCreateNode<TraitsT, NodeT>::value;
};

} // end namespace ilist_internal

//===----------------------------------------------------------------------===//
//
/// A wrapper around an intrusive list with callbacks and non-intrusive
/// ownership.
///
/// This wraps a purely intrusive list (like simple_ilist) with a configurable
/// traits class.  The traits can implement callbacks and customize the
/// ownership semantics.
///
/// This is a subset of ilist functionality that can safely be used on nodes of
/// polymorphic types, i.e. a heterogeneous list with a common base class that
/// holds the next/prev pointers.  The only state of the list itself is an
/// ilist_sentinel, which holds pointers to the first and last nodes in the
/// list.
template <class IntrusiveListT, class TraitsT>
class iplist_impl : public TraitsT, IntrusiveListT {
   typedef IntrusiveListT base_list_type;
   
protected:
   typedef iplist_impl iplist_impl_type;
   
public:
   typedef typename base_list_type::pointer pointer;
   typedef typename base_list_type::const_pointer const_pointer;
   typedef typename base_list_type::reference reference;
   typedef typename base_list_type::const_reference const_reference;
   typedef typename base_list_type::value_type value_type;
   typedef typename base_list_type::size_type size_type;
   typedef typename base_list_type::difference_type difference_type;
   typedef typename base_list_type::iterator iterator;
   typedef typename base_list_type::const_iterator const_iterator;
   typedef typename base_list_type::reverse_iterator reverse_iterator;
   typedef
   typename base_list_type::const_reverse_iterator const_reverse_iterator;
   
private:
   // TODO: Drop this assertion and the transitive type traits anytime after
   // v4.0 is branched (i.e,. keep them for one release to help out-of-tree code
   // update).
   static_assert(
         !ilist_internal::HasObsoleteCustomization<TraitsT, value_type>::value,
         "ilist customization points have changed!");
   
   static bool op_less(const_reference L, const_reference R) { return L < R; }
   static bool op_equal(const_reference L, const_reference R) { return L == R; }
   
public:
   iplist_impl() = default;
   
   iplist_impl(const iplist_impl &) = delete;
   iplist_impl &operator=(const iplist_impl &) = delete;
   
   iplist_impl(iplist_impl &&X)
      : TraitsT(std::move(X)), IntrusiveListT(std::move(X)) {}
   iplist_impl &operator=(iplist_impl &&X) {
      *static_cast<TraitsT *>(this) = std::move(X);
      *static_cast<IntrusiveListT *>(this) = std::move(X);
      return *this;
   }
   
   ~iplist_impl() { clear(); }
   
   // Miscellaneous inspection routines.
   size_type max_size() const { return size_type(-1); }
   
   using base_list_type::begin;
   using base_list_type::end;
   using base_list_type::rbegin;
   using base_list_type::rend;
   using base_list_type::empty;
   using base_list_type::front;
   using base_list_type::back;
   
   void swap(iplist_impl &RHS) {
      assert(0 && "Swap does not use list traits callback correctly yet!");
      base_list_type::swap(RHS);
   }
   
   iterator insert(iterator where, pointer New) {
      this->addNodeToList(New); // Notify traits that we added a node...
      return base_list_type::insert(where, *New);
   }
   
   iterator insert(iterator where, const_reference New) {
      return this->insert(where, new value_type(New));
   }
   
   iterator insertAfter(iterator where, pointer New) {
      if (empty())
         return insert(begin(), New);
      else
         return insert(++where, New);
   }
   
   /// Clone another list.
   template <class Cloner> void cloneFrom(const iplist_impl &L2, Cloner clone) {
      clear();
      for (const_reference V : L2)
         push_back(clone(V));
   }
   
   pointer remove(iterator &IT) {
      pointer Node = &*IT++;
      this->removeNodeFromList(Node); // Notify traits that we removed a node...
      base_list_type::remove(*Node);
      return Node;
   }
   
   pointer remove(const iterator &IT) {
      iterator MutIt = IT;
      return remove(MutIt);
   }
   
   pointer remove(pointer IT) { return remove(iterator(IT)); }
   pointer remove(reference IT) { return remove(iterator(IT)); }
   
   // erase - remove a node from the controlled sequence... and delete it.
   iterator erase(iterator where) {
      this->deleteNode(remove(where));
      return where;
   }
   
   iterator erase(pointer IT) { return erase(iterator(IT)); }
   iterator erase(reference IT) { return erase(iterator(IT)); }
   
   /// Remove all nodes from the list like clear(), but do not call
   /// removeNodeFromList() or deleteNode().
   ///
   /// This should only be used immediately before freeing nodes in bulk to
   /// avoid traversing the list and bringing all the nodes into cache.
   void clearAndLeakNodesUnsafely() { base_list_type::clear(); }
   
private:
   // transfer - The heart of the splice function.  Move linked list nodes from
   // [first, last) into position.
   //
   void transfer(iterator position, iplist_impl &L2, iterator first, iterator last) {
      if (position == last)
         return;
      
      if (this != &L2) // Notify traits we moved the nodes...
         this->transferNodesFromList(L2, first, last);
      
      base_list_type::splice(position, L2, first, last);
   }
   
public:
   //===----------------------------------------------------------------------===
   // Functionality derived from other functions defined above...
   //
   
   using base_list_type::size;
   
   iterator erase(iterator first, iterator last) {
      while (first != last)
         first = erase(first);
      return last;
   }
   
   void clear() { erase(begin(), end()); }
   
   // Front and back inserters...
   void push_front(pointer val) { insert(begin(), val); }
   void push_back(pointer val) { insert(end(), val); }
   void pop_front() {
      assert(!empty() && "pop_front() on empty list!");
      erase(begin());
   }
   void pop_back() {
      assert(!empty() && "pop_back() on empty list!");
      iterator t = end(); erase(--t);
   }
   
   // Special forms of insert...
   template<class InIt> void insert(iterator where, InIt first, InIt last) {
      for (; first != last; ++first) insert(where, *first);
   }
   
   // Splice members - defined in terms of transfer...
   void splice(iterator where, iplist_impl &L2) {
      if (!L2.empty())
         transfer(where, L2, L2.begin(), L2.end());
   }
   void splice(iterator where, iplist_impl &L2, iterator first) {
      iterator last = first; ++last;
      if (where == first || where == last) return; // No change
      transfer(where, L2, first, last);
   }
   void splice(iterator where, iplist_impl &L2, iterator first, iterator last) {
      if (first != last) transfer(where, L2, first, last);
   }
   void splice(iterator where, iplist_impl &L2, reference N) {
      splice(where, L2, iterator(N));
   }
   void splice(iterator where, iplist_impl &L2, pointer N) {
      splice(where, L2, iterator(N));
   }
   
   template <class Compare>
   void merge(iplist_impl &Right, Compare comp) {
      if (this == &Right)
         return;
      this->transferNodesFromList(Right, Right.begin(), Right.end());
      base_list_type::merge(Right, comp);
   }
   void merge(iplist_impl &Right) { return merge(Right, op_less); }
   
   using base_list_type::sort;
   
   /// \brief Get the previous node, or \c nullptr for the list head.
   pointer getPrevNode(reference N) const {
      auto I = N.getIterator();
      if (I == begin())
         return nullptr;
      return &*std::prev(I);
   }
   /// \brief Get the previous node, or \c nullptr for the list head.
   const_pointer getPrevNode(const_reference N) const {
      return getPrevNode(const_cast<reference >(N));
   }
   
   /// \brief Get the next node, or \c nullptr for the list tail.
   pointer getNextNode(reference N) const {
      auto Next = std::next(N.getIterator());
      if (Next == end())
         return nullptr;
      return &*Next;
   }
   /// \brief Get the next node, or \c nullptr for the list tail.
   const_pointer getNextNode(const_reference N) const {
      return getNextNode(const_cast<reference >(N));
   }
};

/// An intrusive list with ownership and callbacks specified/controlled by
/// IntrusiveListTraits, only with API safe for polymorphic types.
///
/// The \p Options parameters are the same as those for \a simple_ilist.  See
/// there for a description of what's available.
template <class T, class... Options>
class iplist
      : public iplist_impl<simple_ilist<T, Options...>, IntrusiveListTraits<T>> {
   typedef typename iplist::iplist_impl_type iplist_impl_type;
   
public:
   iplist() = default;
   
   iplist(const iplist &X) = delete;
   iplist &operator=(const iplist &X) = delete;
   
   iplist(iplist &&X) : iplist_impl_type(std::move(X)) {}
   iplist &operator=(iplist &&X) {
      *static_cast<iplist_impl_type *>(this) = std::move(X);
      return *this;
   }
};

template <class T, class... Options> using ilist = iplist<T, Options...>;

} // basic
} // polar

namespace std {

// Ensure that swap uses the fast list swap...
template<class Ty>
void swap(llvm::iplist<Ty> &Left, llvm::iplist<Ty> &Right) {
   Left.swap(Right);
}

} // end namespace std

#endif // POLAR_BASIC_ADT_INTRUSIVE_LIST_H
