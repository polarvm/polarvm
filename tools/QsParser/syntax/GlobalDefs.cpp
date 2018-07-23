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

#include "GlobalDefs.h"
#include "polar/basic/adt/StringRef.h"
#include <cstring>

namespace qsparser {
namespace syntax {

using polar::basic::StringRef;

std::string kind_to_type(Kind kind)
{
   switch (kind) {
   case Kind::Syntax:
      return "Syntax";
   case Kind::SyntaxCollection:
      return "SyntaxCollection";
   case Kind::BackslashToken:
   case Kind::ColonToken:
   case Kind::CommaToken:
   case Kind::EqualToken:
   case Kind::IdentifierToken:
   case Kind::InfixQuestionMarkToken:
   case Kind::InToken:
   case Kind::LeftBraceToken:
   case Kind::LeftParenToken:
   case Kind::LeftSquareBracketToken:
   case Kind::PeriodToken:
   case Kind::PostfixOperatorToken:
   case Kind::PostfixQuestionMarkToken:
   case Kind::RightBraceToken:
   case Kind::RightParenToken:
   case Kind::RightSquareBracketToken:
   case Kind::SelfToken:
   case Kind::StringInterpolationAnchorToken:
   case Kind::StringSegmentToken:
   case Kind::ThrowsToken:
   case Kind::Token:
      return "TokenSyntax";
   case Kind::ClosureCaptureItemList:
      return "ClosureCaptureItemList";
   case Kind::ClosureExpr:
      return "ClosureExpr";
   case Kind::ClosureParamList:
      return "ClosureParamList";
   case Kind::ClosureSignature:
      return "ClosureSignature";
   case Kind::CodeBlockItemList:
      return "CodeBlockItemList";
   case Kind::Decl:
      return "Decl";
   case Kind::DeclNameArguments:
      return "DeclNameArguments";
   case Kind::Expr:
      return "Expr";
   case Kind::FunctionCallArgumentList:
      return "FunctionCallArgumentList";
   case Kind::GenericArgumentClause:
      return "GenericArgumentClause";
   case Kind::None:
      return "None";
   case Kind::ParameterClause:
      return "ParameterClause";
   case Kind::Pattern:
      return "Pattern";
   case Kind::ReturnClause:
      return "ReturnClause";
   case Kind::Stmt:
      return "Stmt";
   case Kind::StringInterpolationSegments:
      return "StringInterpolationSegments";
   case Kind::TokenList:
      return "TokenList";
   case Kind::Type:
      return "Type";
   }
}

std::string lowercase_first_word(std::string name)
{
   size_t wordIndex = 0;
   size_t thresholdIndex = 1;
   for (int i = 0; i < name.size(); ++i) {
      char c = name.at(i);
      if (std::islower(c)) {
         if (wordIndex > thresholdIndex) {
            wordIndex -= 1;
         }
         break;
      }
      wordIndex += 1;
   }
   if (wordIndex == 0) {
      return std::move(name);
   }
   return StringRef(name.substr(0, wordIndex).c_str()).toLower() + name.substr(wordIndex);
}

} // syntax
} // qsparser
