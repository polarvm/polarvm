// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/23.

#ifndef QS_PARSER_SYNTAX_NODE_H
#define QS_PARSER_SYNTAX_NODE_H

#include <string>
#include <vector>
#include "EnumDefs.h"

namespace qsparser {
namespace syntax {

class Trait;
class Child;

/// A Syntax node, possibly with children.
/// If the kind is "SyntaxCollection", then this node is considered a Syntax
/// Collection that will expose itself as a typedef rather than a concrete
/// subclass.

class Node
{
public:
   Node(const std::string &name, const std::string &description = "",
        Kind kind= Kind::None, ElementType element = ElementType::Node,
        const std::string &elementName = "", TokenChoice choice = TokenChoice::None,
        bool omitWhenEmpty = false)
      : m_syntaxKind(name),
        m_polarSyntaxKind()
   {

   }
protected:
   std::string m_syntaxKind;
   std::string m_polarSyntaxKind;
   std::string m_name;
   std::string m_description;
   std::string m_kind;
   std::string m_baseKind;
   std::vector<Trait> m_traits;
   std::vector<Child> m_children;
   bool m_omitWhenEmpty;
   std::string m_collectionElement;
   /// If there's a preferred name for the collection element that differs
   /// from its supertype, use that.
   std::string m_collectionElementName;
   std::string m_collectionElementType;
   std::vector<std::string> m_collectionElementChoices;
};

} // syntax
} // qsparser

#endif // QS_PARSER_SYNTAX_NODE_H
