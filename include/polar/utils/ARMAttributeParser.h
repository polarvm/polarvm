// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/06/18.

#ifndef POLAR_UTILS_ARMATTRIBUTEPARSER_H
#define POLAR_UTILS_ARMATTRIBUTEPARSER_H

#include "polar/utils/ARMBuildAttributes.h"
#include "polar/utils/ScopedPrinter.h"
#include <map>

namespace polar {

// forward declare class with namespace
namespace basic {
class StringRef;
template <typename T>
class SmallVectorImpl;
} // basic

using basic::ArrayRef;
using basic::StringRef;
using utils::ScopedPrinter;
using basic::SmallVectorImpl;

class ARMAttributeParser
{
   ScopedPrinter *m_printer;
   std::map<unsigned, unsigned> m_attributes;
   struct DisplayHandler
   {
      armbuildattrs::AttrType m_attribute;
      void (ARMAttributeParser::*m_routine)(armbuildattrs::AttrType,
                                          const uint8_t *, uint32_t &);
   };
   static const DisplayHandler sm_displayRoutines[];

   uint64_t parseInteger(const uint8_t *data, uint32_t &offset);

   StringRef parseString(const uint8_t *data, uint32_t &offset);

   void integerAttribute(armbuildattrs::AttrType tag, const uint8_t *data,
                         uint32_t &offset);
   void stringAttribute(armbuildattrs::AttrType tag, const uint8_t *data,
                        uint32_t &offset);

   void printAttribute(unsigned tag, unsigned Value, StringRef ValueDesc);

   void cpuArch(armbuildattrs::AttrType tag, const uint8_t *data,
                uint32_t &offset);

   void cpuArchprofile(armbuildattrs::AttrType tag, const uint8_t *data,
                       uint32_t &offset);

   void armIsaUse(armbuildattrs::AttrType tag, const uint8_t *data,
                  uint32_t &offset);

   void thumbIsaUse(armbuildattrs::AttrType tag, const uint8_t *data,
                    uint32_t &offset);

   void fpArch(armbuildattrs::AttrType tag, const uint8_t *data,
               uint32_t &offset);

   void wmmxArch(armbuildattrs::AttrType tag, const uint8_t *data,
                 uint32_t &offset);

   void advancedSimdArch(armbuildattrs::AttrType tag, const uint8_t *data,
                         uint32_t &offset);

   void pcsConfig(armbuildattrs::AttrType tag, const uint8_t *data,
                  uint32_t &offset);

   void abiPcsR9Use(armbuildattrs::AttrType tag, const uint8_t *data,
                    uint32_t &offset);

   void abiPcsRwData(armbuildattrs::AttrType tag, const uint8_t *data,
                        uint32_t &offset);

   void abiPcsRoData(armbuildattrs::AttrType tag, const uint8_t *data,
                        uint32_t &offset);

   void abiPcsGotUse(armbuildattrs::AttrType tag, const uint8_t *data,
                        uint32_t &offset);

   void abiPcsWcharType(armbuildattrs::AttrType tag, const uint8_t *data,
                        uint32_t &offset);

   void abiFpRounding(armbuildattrs::AttrType tag, const uint8_t *data,
                        uint32_t &offset);

   void abiFpDenormal(armbuildattrs::AttrType tag, const uint8_t *data,
                        uint32_t &offset);

   void abiFpExceptions(armbuildattrs::AttrType tag, const uint8_t *data,
                          uint32_t &offset);

   void abiFpUserExceptions(armbuildattrs::AttrType tag, const uint8_t *data,
                               uint32_t &offset);

   void abiFpNumberModel(armbuildattrs::AttrType tag, const uint8_t *data,
                            uint32_t &offset);

   void abiAlignNeeded(armbuildattrs::AttrType tag, const uint8_t *data,
                         uint32_t &offset);

   void abiAlignPreserved(armbuildattrs::AttrType tag, const uint8_t *data,
                            uint32_t &offset);

   void abiEnumSize(armbuildattrs::AttrType tag, const uint8_t *data,
                      uint32_t &offset);

   void abiHardFpUse(armbuildattrs::AttrType tag, const uint8_t *data,
                       uint32_t &offset);

   void abiVfpArgs(armbuildattrs::AttrType tag, const uint8_t *data,
                     uint32_t &offset);

   void abiWmmxArgs(armbuildattrs::AttrType tag, const uint8_t *data,
                      uint32_t &offset);

   void abiOptimizationGoals(armbuildattrs::AttrType tag, const uint8_t *data,
                               uint32_t &offset);

   void abiFpOptimizationGoals(armbuildattrs::AttrType tag,
                               const uint8_t *data, uint32_t &offset);

   void compatibility(armbuildattrs::AttrType tag, const uint8_t *data,
                      uint32_t &offset);

   void cpuUnalignedAccess(armbuildattrs::AttrType tag, const uint8_t *data,
                           uint32_t &offset);

   void fpHpExtension(armbuildattrs::AttrType tag, const uint8_t *data,
                      uint32_t &offset);

   void abiFp16bitFormat(armbuildattrs::AttrType tag, const uint8_t *data,
                         uint32_t &offset);

   void mpExtensionUse(armbuildattrs::AttrType tag, const uint8_t *data,
                       uint32_t &offset);

   void divUse(armbuildattrs::AttrType tag, const uint8_t *data,
               uint32_t &offset);

   void dspExtension(armbuildattrs::AttrType tag, const uint8_t *data,
                     uint32_t &offset);

   void t2eeUse(armbuildattrs::AttrType tag, const uint8_t *data,
                uint32_t &offset);

   void virtualizationUse(armbuildattrs::AttrType tag, const uint8_t *data,
                          uint32_t &offset);

   void nodefaults(armbuildattrs::AttrType tag, const uint8_t *data,
                   uint32_t &offset);

   void parseAttributeList(const uint8_t *data, uint32_t &offset,
                           uint32_t length);

   void parseIndexList(const uint8_t *data, uint32_t &offset,
                       SmallVectorImpl<uint8_t> &indexList);

   void parseSubsection(const uint8_t *data, uint32_t length);
public:
   ARMAttributeParser(ScopedPrinter *printer)
      : m_printer(printer)
   {}

   ARMAttributeParser() : m_printer(nullptr)
   {}

   void parse(ArrayRef<uint8_t> section, bool isLittle);

   bool hasAttribute(unsigned tag) const
   {
      return m_attributes.count(tag);
   }

   unsigned getAttributeValue(unsigned tag) const
   {
      return m_attributes.find(tag)->second;
   }
};

} // polar

#endif // POLAR_UTILS_ARMATTRIBUTEPARSER_H
