// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/01.

#ifndef POLAR_UTILS_TRIGRAM_INDEX_H
#define POLAR_UTILS_TRIGRAM_INDEX_H

#include "polar/basic/adt/SmallVector.h"
#include "polar/basic/adt/StringMap.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace polar {

// forawrd declare class with namespace
namespace basic {
class StringRef;
} // basic

namespace utils {

using polar::basic::StringRef;

class TrigramIndex
{
public:
   /// Inserts a new Regex into the index.
   void insert(std::string regex);

   /// Returns true, if special case list definitely does not have a line
   /// that matches the query. Returns false, if it's not sure.
   bool isDefinitelyOut(StringRef query) const;

   /// Returned true, iff the heuristic is defeated and not useful.
   /// In this case isDefinitelyOut always returns false.
   bool isDefeated()
   {
      return m_defeated;
   }
private:
   // If true, the rules are too complicated for the check to work, and full
   // regex matching is needed for every rule.
   bool m_defeated = false;
   // The minimum number of trigrams which should match for a rule to have a
   // chance to match the query. The number of elements equals the number of
   // regex rules in the SpecialCaseList.
   std::vector<unsigned> m_counts;
   // m_index holds a list of rules indices for each trigram. The same indices
   // are used in m_counts to store per-rule limits.
   // If a trigram is too common (>4 rules with it), we stop tracking it,
   // which increases the probability for a need to match using regex, but
   // decreases the costs in the regular case.
   std::unordered_map<unsigned, SmallVector<size_t, 4>> m_index{256};
};

} // utils
} // polar

#endif // POLAR_UTILS_TRIGRAM_INDEX_H
