// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/03.

#include "polar/global/ManagedStatic.h"
#include "polar/utils/DebugCounter.h"
#include "polar/utils/CommandLine.h"
#include "polar/utils/Format.h"
#include "polar/utils/Options.h"

namespace polar {
namespace utils {

namespace {
// This class overrides the default list implementation of printing so we
// can pretty print the list of debug counter options.  This type of
// dynamic option is pretty rare (basically this and pass lists).
class DebugCounterList : public cmd::List<std::string, DebugCounter>
{
private:
   using Base = cmd::List<std::string, DebugCounter>;

public:
   template <class... Mods>
   explicit DebugCounterList(Mods &&... mods) : Base(std::forward<Mods>(mods)...)
   {}

private:
   void printOptionInfo(size_t globalWidth) const override
   {
      // This is a variant of from generic_parser_base::printOptionInfo.  Sadly,
      // it's not easy to make it more usable.  We could get it to print these as
      // options if we were a cl::opt and registered them, but lists don't have
      // options, nor does the parser for std::string.  The other mechanisms for
      // options are global and would pollute the global namespace with our
      // counters.  Rather than go that route, we have just overridden the
      // printing, which only a few things call anyway.
      out_stream() << "  -" << m_argStr;
      // All of the other options in CommandLine.cpp use m_argStr.size() + 6 for
      // width, so we do the same.
      Option::printHelpStr(m_helpStr, globalWidth, m_argStr.getSize() + 6);
      const auto &counterInstance = DebugCounter::getInstance();
      for (auto name : counterInstance) {
         const auto info =
               counterInstance.getCounterInfo(counterInstance.getCounterId(name));
         size_t NumSpaces = globalWidth - info.first.size() - 8;
         out_stream() << "    =" << info.first;
         out_stream().indent(NumSpaces) << " -   " << info.second << '\n';
      }
   }
};
} // namespace

// Create our command line option.
static DebugCounterList sg_debugCounterOption(
      "debug-counter",
      cmd::Desc("Comma separated list of debug counter skip and count"),
      cmd::CommaSeparated, cmd::ZeroOrMore, cmd::location(DebugCounter::getInstance()));

static ManagedStatic<DebugCounter> sg_debugCounter;

DebugCounter &DebugCounter::getInstance()
{
   return *sg_debugCounter;
}

// This is called by the command line parser when it sees a value for the
// debug-counter option defined above.
void DebugCounter::pushBack(const std::string &value)
{
   if (value.empty()) {
      return;
   }
   // The strings should come in as counter=value
   auto counterPair = StringRef(value).split('=');
   if (counterPair.second.empty()) {
      error_stream() << "DebugCounter Error: " << value << " does not have an = in it\n";
      return;
   }
   // Now we have counter=value.
   // First, process value.
   long counterVal;
   if (counterPair.second.getAsInteger(0, counterVal)) {
      error_stream() << "DebugCounter Error: " << counterPair.second
                     << " is not a number\n";
      return;
   }
   // Now we need to see if this is the skip or the count, remove the suffix, and
   // add it to the counter values.
   if (counterPair.first.endsWith("-skip")) {
      auto counterName = counterPair.first.dropBack(5);
      unsigned counterID = m_registeredCounters.idFor(counterName);
      if (!counterID) {
         error_stream() << "DebugCounter Error: " << counterName
                        << " is not a registered counter\n";
         return;
      }

      auto result = m_counters.insert({counterID, {0, -1}});
      result.first->second.first = counterVal;
   } else if (counterPair.first.endsWith("-count")) {
      auto counterName = counterPair.first.dropBack(6);
      unsigned counterID = m_registeredCounters.idFor(counterName);
      if (!counterID) {
         error_stream() << "DebugCounter Error: " << counterName
                        << " is not a registered counter\n";
         return;
      }

      auto result = m_counters.insert({counterID, {0, -1}});
      result.first->second.second = counterVal;
   } else {
      error_stream() << "DebugCounter Error: " << counterPair.first
                     << " does not end with -skip or -count\n";
   }
}

void DebugCounter::print(RawOutStream &outstream) const
{
   outstream << "Counters and values:\n";
   for (const auto &kv : m_counters) {
      outstream << left_justify(m_registeredCounters[kv.first], 32) << ": {"
                                                                    << kv.second.first << "," << kv.second.second << "}\n";
   }

}

POLAR_DUMP_METHOD void DebugCounter::dump() const
{
   print(debug_stream());
}

} // utils
} // polar
