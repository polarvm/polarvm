// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/14.

#include "polar/basic/adt/StringExtras.h"
#include "polar/utils/RawOutStream.h"
#include "polar/basic/adt/SmallVector.h"

namespace polar {
namespace basic {

/// str_in_str_no_case - Portable version of strcasestr.  Locates the first
/// occurrence of string 's1' in string 's2', ignoring case.  Returns
/// the offset of s2 in s1 or npos if s2 cannot be found.
StringRef::size_type str_in_str_no_case(StringRef lhs, StringRef rhs)
{
   size_t lhsSize = lhs.getSize();
   size_t rhsSize = rhs.getSize();
   if (rhsSize > lhsSize) {
      return StringRef::npos;
   }
   for (size_t i = 0, e = lhsSize - rhsSize + 1; i != e; ++i) {
      if (lhs.substr(i, rhsSize).equalsLower(rhs)) {
         return i;
      }
   }
   return StringRef::npos;
}

/// getToken - This function extracts one token from source, ignoring any
/// leading characters that appear in the Delimiters string, and ending the
/// token at any of the characters that appear in the Delimiters string.  If
/// there are no tokens in the source string, an empty string is returned.
/// The function returns a pair containing the extracted token and the
/// remaining tail string.
std::pair<StringRef, StringRef> get_token(StringRef source,
                                          StringRef delimiters)
{
   // Figure out where the token starts.
   StringRef::size_type start = source.findFirstNotOf(delimiters);
   // Find the next occurrence of the delimiter.
   StringRef::size_type end = source.findFirstOf(delimiters, start);
   return std::make_pair(source.slice(start, end), source.substr(end));
}

/// split_string - Split up the specified string according to the specified
/// delimiters, appending the result fragments to the output list.
void split_string(StringRef source,
                  SmallVectorImpl<StringRef> &outFragments,
                  StringRef delimiters)
{
   std::pair<StringRef, StringRef> pair = get_token(source, delimiters);
   while (!pair.first.empty()) {
      outFragments.push_back(pair.first);
      pair = get_token(pair.second, delimiters);
   }
}

void print_lower_case(StringRef string, RawOutStream &out)
{
   for (const char c : string) {
      out << to_lower(c);
   }
}

} // basic
} // polar
