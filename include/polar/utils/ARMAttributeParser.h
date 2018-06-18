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
      void (ARMAttributeParser::*Routine)(armbuildattrs::AttrType,
                                          const uint8_t *, uint32_t &);
   };
   static const DisplayHandler sm_displayRoutines[];

   uint64_t ParseInteger(const uint8_t *data, uint32_t &offset);

   StringRef ParseString(const uint8_t *data, uint32_t &offset);

   void IntegerAttribute(armbuildattrs::AttrType tag, const uint8_t *data,
                         uint32_t &offset);
   void StringAttribute(armbuildattrs::AttrType tag, const uint8_t *data,
                        uint32_t &offset);

   void PrintAttribute(unsigned tag, unsigned Value, StringRef ValueDesc);

   void CPU_arch(armbuildattrs::AttrType tag, const uint8_t *data,
                 uint32_t &offset);

   void CPU_arch_profile(armbuildattrs::AttrType tag, const uint8_t *data,
                         uint32_t &offset);

   void ARM_ISA_use(armbuildattrs::AttrType tag, const uint8_t *data,
                    uint32_t &offset);

   void THUMB_ISA_use(armbuildattrs::AttrType tag, const uint8_t *data,
                      uint32_t &offset);

   void FP_arch(armbuildattrs::AttrType tag, const uint8_t *data,
                uint32_t &offset);

   void WMMX_arch(armbuildattrs::AttrType tag, const uint8_t *data,
                  uint32_t &offset);

   void Advanced_SIMD_arch(armbuildattrs::AttrType tag, const uint8_t *data,
                           uint32_t &offset);

   void PCS_config(armbuildattrs::AttrType tag, const uint8_t *data,
                   uint32_t &offset);

   void ABI_PCS_R9_use(armbuildattrs::AttrType tag, const uint8_t *data,
                       uint32_t &offset);

   void ABI_PCS_RW_data(armbuildattrs::AttrType tag, const uint8_t *data,
                        uint32_t &offset);

   void ABI_PCS_RO_data(armbuildattrs::AttrType tag, const uint8_t *data,
                        uint32_t &offset);

   void ABI_PCS_GOT_use(armbuildattrs::AttrType tag, const uint8_t *data,
                        uint32_t &offset);

   void ABI_PCS_wchar_t(armbuildattrs::AttrType tag, const uint8_t *data,
                        uint32_t &offset);

   void ABI_FP_rounding(armbuildattrs::AttrType tag, const uint8_t *data,
                        uint32_t &offset);

   void ABI_FP_denormal(armbuildattrs::AttrType tag, const uint8_t *data,
                        uint32_t &offset);

   void ABI_FP_exceptions(armbuildattrs::AttrType tag, const uint8_t *data,
                          uint32_t &offset);

   void ABI_FP_user_exceptions(armbuildattrs::AttrType tag, const uint8_t *data,
                               uint32_t &offset);

   void ABI_FP_number_model(armbuildattrs::AttrType tag, const uint8_t *data,
                            uint32_t &offset);

   void ABI_align_needed(armbuildattrs::AttrType tag, const uint8_t *data,
                         uint32_t &offset);

   void ABI_align_preserved(armbuildattrs::AttrType tag, const uint8_t *data,
                            uint32_t &offset);

   void ABI_enum_size(armbuildattrs::AttrType tag, const uint8_t *data,
                      uint32_t &offset);

   void ABI_HardFP_use(armbuildattrs::AttrType tag, const uint8_t *data,
                       uint32_t &offset);

   void ABI_VFP_args(armbuildattrs::AttrType tag, const uint8_t *data,
                     uint32_t &offset);

   void ABI_WMMX_args(armbuildattrs::AttrType tag, const uint8_t *data,
                      uint32_t &offset);

   void ABI_optimization_goals(armbuildattrs::AttrType tag, const uint8_t *data,
                               uint32_t &offset);

   void ABI_FP_optimization_goals(armbuildattrs::AttrType tag,
                                  const uint8_t *data, uint32_t &offset);

   void compatibility(armbuildattrs::AttrType tag, const uint8_t *data,
                      uint32_t &offset);

   void CPU_unaligned_access(armbuildattrs::AttrType tag, const uint8_t *data,
                             uint32_t &offset);

   void FP_HP_extension(armbuildattrs::AttrType tag, const uint8_t *data,
                        uint32_t &offset);

   void ABI_FP_16bit_format(armbuildattrs::AttrType tag, const uint8_t *data,
                            uint32_t &offset);

   void MPextension_use(armbuildattrs::AttrType tag, const uint8_t *data,
                        uint32_t &offset);

   void DIV_use(armbuildattrs::AttrType tag, const uint8_t *data,
                uint32_t &offset);

   void DSP_extension(armbuildattrs::AttrType tag, const uint8_t *data,
                      uint32_t &offset);

   void T2EE_use(armbuildattrs::AttrType tag, const uint8_t *data,
                 uint32_t &offset);

   void Virtualization_use(armbuildattrs::AttrType tag, const uint8_t *data,
                           uint32_t &offset);

   void nodefaults(armbuildattrs::AttrType tag, const uint8_t *data,
                   uint32_t &offset);

   void ParseAttributeList(const uint8_t *data, uint32_t &offset,
                           uint32_t length);

   void ParseIndexList(const uint8_t *data, uint32_t &offset,
                       SmallVectorImpl<uint8_t> &indexList);

   void ParseSubsection(const uint8_t *data, uint32_t length);
public:
   ARMAttributeParser(ScopedPrinter *printer)
      : m_printer(printer)
   {}

   ARMAttributeParser() : m_printer(nullptr)
   {}

   void Parse(ArrayRef<uint8_t> section, bool isLittle);

   bool hasAttribute(unsigned tag) const
   {
      return Attributes.count(tag);
   }

   unsigned getAttributeValue(unsigned tag) const
   {
      return Attributes.find(tag)->second;
   }
};

} // polar

#endif // POLAR_UTILS_ARMATTRIBUTEPARSER_H
