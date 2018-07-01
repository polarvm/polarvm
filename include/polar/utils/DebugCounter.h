// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/29.

#ifndef POLAR_UTILS_DEBUG_COUNTER_H
#define POLAR_UTILS_DEBUG_COUNTER_H

#include "polar/basic/adt/DenseMap.h"
#include "polar/basic/adt/UniqueVector.h"
#include "polar/utils/CommandLine.h"
#include "polar/utils/Debug.h"
#include "polar/utils/RawOutStream.h"
#include <string>

namespace polar {
namespace utils {

using polar::basic::DenseMap;
using polar::basic::UniqueVector;

class DebugCounter
{
public:
   /// \brief Returns a reference to the singleton getInstance.
   static DebugCounter &getInstance();

   // Used by the command line option parser to push a new value it parsed.
   void pushBack(const std::string &);

   // Register a counter with the specified name.
   //
   // FIXME: Currently, counter registration is required to happen before command
   // line option parsing. The main reason to register counters is to produce a
   // nice list of them on the command line, but i'm not sure this is worth it.
   static unsigned registerCounter(StringRef name, StringRef desc)
   {
      return getInstance().addCounter(name, desc);
   }

   inline static bool shouldExecute(unsigned counterName)
   {
      // Compile to nothing when debugging is off
#ifdef NDEBUG
      return true;
#else
      auto &us = getInstance();
      auto result = us.m_counters.find(counterName);
      if (result != us.m_counters.end()) {
         auto &counterPair = result->second;
         // We only execute while the skip (first) is zero and the count (second)
         // is non-zero.
         // Negative counters always execute.
         if (counterPair.first < 0) {
            return true;
         }
         if (counterPair.first != 0) {
            --counterPair.first;
            return false;
         }
         if (counterPair.second < 0) {
            return true;
         }
         if (counterPair.second != 0) {
            --counterPair.second;
            return true;
         }
         return false;
      }
      // Didn't find the counter, should we warn?
      return true;
#endif // NDEBUG
   }

   // Return true if a given counter had values set (either programatically or on
   // the command line).  This will return true even if those values are
   // currently in a state where the counter will always execute.
   static bool isCounterSet(unsigned id)
   {
      return getInstance().m_counters.count(id);
   }

   // Return the skip and count for a counter. This only works for set counters.
   static std::pair<int, int> getCounterValue(unsigned id)
   {
      auto &us = getInstance();
      auto result = us.m_counters.find(id);
      assert(result != us.m_counters.end() && "Asking about a non-set counter");
      return result->second;
   }

   // Set a registered counter to a given value.
   static void setCounterValue(unsigned id, const std::pair<int, int> &value)
   {
      auto &us = getInstance();
      us.m_counters[id] = value;
   }

   // Dump or print the current counter set into llvm::dbgs().
   POLAR_DUMP_METHOD void dump() const;

   void print(RawOutStream &outstream) const;

   // Get the counter id for a given named counter, or return 0 if none is found.
   unsigned getCounterId(const std::string &name) const
   {
      return m_registeredCounters.idFor(name);
   }

   // Return the number of registered counters.
   unsigned int getNumCounters() const
   {
      return m_registeredCounters.getSize();
   }

   // Return the name and description of the counter with the given id.
   std::pair<std::string, std::string> getCounterInfo(unsigned id) const
   {
      return std::make_pair(m_registeredCounters[id], m_counterDesc.lookup(id));
   }

   // Iterate through the registered counters
   typedef UniqueVector<std::string> CounterVector;
   CounterVector::const_iterator begin() const
   {
      return m_registeredCounters.begin();
   }

   CounterVector::const_iterator end() const
   {
      return m_registeredCounters.end();
   }

private:
   unsigned addCounter(const std::string &name, const std::string &desc)
   {
      unsigned result = m_registeredCounters.insert(name);
      m_counterDesc[result] = desc;
      return result;
   }
   DenseMap<unsigned, std::pair<long, long>> m_counters;
   DenseMap<unsigned, std::string> m_counterDesc;
   CounterVector m_registeredCounters;
};

#define DEBUG_COUNTER(VARNAME, COUNTERNAME, DESC)                              \
   static const unsigned VARNAME =                                              \
   DebugCounter::registerCounter(COUNTERNAME, DESC)

} // utils
} // polar

#endif // POLAR_UTILS_DEBUG_COUNTER_H
