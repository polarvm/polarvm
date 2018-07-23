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

#ifndef QS_PARSER_SYNTAX_ENUM_DEFS_H
#define QS_PARSER_SYNTAX_ENUM_DEFS_H

#include <string>

namespace qsparser {
namespace syntax {

enum class Kind
{
   None,
   Decl, Expr, Pattern,
   Stmt, Syntax, SyntaxCollection,
   Type, Token, InfixQuestionMarkToken,
   ColonToken, PeriodToken, DeclNameArguments,
   SelfToken, TokenList, IdentifierToken,
   EqualToken, CommaToken, LeftSquareBracketToken,
   ClosureCaptureItemList, RightSquareBracketToken,
   ClosureParamList, ParameterClause, ThrowsToken,
   ReturnClause, InToken, LeftBraceToken, RightBraceToken,
   ClosureSignature, CodeBlockItemList, LeftParenToken,
   RightParenToken, ClosureExpr, FunctionCallArgumentList,
   PostfixQuestionMarkToken, ExclamationMarkToken,
   PostfixOperatorToken, GenericArgumentClause, StringSegmentToken,
   BackslashToken, StringInterpolationAnchorToken, StringInterpolationSegments,
};

enum class TokenChoice
{
   None,
   PostfixQuestionMarkToken,
   ExclamationMarkToken,
   IdentifierToken,
   SelfToken,
   CapitalSelfToken,
   DollarIdentifierToken,
   SpacedBinaryOperatorToken,
   TrueToken,
   FalseToken,
   PeriodToken,
   PrefixPeriodToken,
   WildcardToken,
   StringQuoteToken,
   MultilineStringQuoteToken,
   PoundColorLiteralToken,
   PoundFileLiteralToken,
   PoundImageLiteralToken,
   LetToken,
   VarToken
};

enum class ElementType
{
   None,
   CodeBlockItem,
   Token,
   Attribute,
   TuplePatternElement,
   FunctionCallArgument,
   TupleElement,
   ArrayElement,
   DictionaryElement,
   Syntax,
   DeclNameArgument,
   Expr,
   ClosureCaptureItem,
   ClosureParam,
   CatchClause,
   CaseItem,
   ConditionElement,
   FunctionParameter,
   IfConfigClause,
   InheritedType,
   MemberDeclListItem,
   DeclModifier,
   AccessPathComponent,
   AccessorDecl,
   PatternBinding,
   EnumCaseElement,
   PrecedenceGroupNameElement,
   GenericParameter,
   CompositionTypeElement,
   TupleTypeElement,
   GenericArgument,
   AvailabilityArgument
};

/// Converts a SyntaxKind to a type name, checking to see if the kind is
/// Syntax or SyntaxCollection first.
/// A type name is the same as the SyntaxKind name with the suffix "Syntax"
std::string kind_to_type(Kind kind);

/// Lowercases the first word in the provided camelCase or PascalCase string.
/// EOF -> eof
/// IfKeyword -> ifKeyword
// EOFToken -> eofToken
std::string lowercase_first_word(const std::string &name);

} // syntx
} // qsparser

#endif // QS_PARSER_SYNTAX_ENUM_DEFS_H
