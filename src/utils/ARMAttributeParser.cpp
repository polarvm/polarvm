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

#include "polar/utils/ARMAttributeParser.h"
#include "polar/basic/adt/StlExtras.h"
#include "polar/basic/adt/StringExtras.h"
#include "polar/utils/Leb128.h"
#include "polar/utils/ScopedPrinter.h"

namespace polar {

using polar::utils::EnumEntry;
using armbuildattrs::AttrType;
using polar::utils::decode_uleb128;
using polar::utils::DictScope;
using polar::basic::array_lengthof;
using polar::basic::SmallVector;
using polar::utils::error_stream;
using polar::basic::utostr;
using polar::basic::make_array_ref;
using polar::basic::Twine;

static const EnumEntry<unsigned> sg_tagNames[] = {
   { "tag_File", armbuildattrs::File },
   { "tag_section", armbuildattrs::Section },
   { "tag_Symbol", armbuildattrs::Symbol },
};

#define ATTRIBUTE_HANDLER(Attr_, Method_)                                                \
{ armbuildattrs::Attr_, &ARMAttributeParser::Method_ }

const ARMAttributeParser::DisplayHandler
ARMAttributeParser::sm_displayRoutines[] = {
   { armbuildattrs::CPU_raw_name, &ARMAttributeParser::stringAttribute, },
   { armbuildattrs::CPU_name, &ARMAttributeParser::stringAttribute },
   ATTRIBUTE_HANDLER(CPU_arch, cpuArch),
   ATTRIBUTE_HANDLER(CPU_arch_profile, cpuArchProfile),
   ATTRIBUTE_HANDLER(ARM_ISA_use, armIsaUse),
   ATTRIBUTE_HANDLER(THUMB_ISA_use, thumbIsaUse),
   ATTRIBUTE_HANDLER(FP_arch, fpArch),
   ATTRIBUTE_HANDLER(WMMX_arch, wmmxArch),
   ATTRIBUTE_HANDLER(Advanced_SIMD_arch, advancedSimdArch),
   ATTRIBUTE_HANDLER(PCS_config, pcsConfig),
   ATTRIBUTE_HANDLER(ABI_PCS_R9_use, abiPcsR9Use),
   ATTRIBUTE_HANDLER(ABI_PCS_RW_data, abiPcsRwData),
   ATTRIBUTE_HANDLER(ABI_PCS_RO_data, abiPcsRoData),
   ATTRIBUTE_HANDLER(ABI_PCS_GOT_use, abiPcsGotUse),
   ATTRIBUTE_HANDLER(ABI_PCS_wchar_t, abiPcsWcharType),
   ATTRIBUTE_HANDLER(ABI_FP_rounding, abiFpRounding),
   ATTRIBUTE_HANDLER(ABI_FP_denormal, abiFpDenormal),
   ATTRIBUTE_HANDLER(ABI_FP_exceptions, abiFpExceptions),
   ATTRIBUTE_HANDLER(ABI_FP_user_exceptions, abiFpUserExceptions),
   ATTRIBUTE_HANDLER(ABI_FP_number_model, abiFpNumberModel),
   ATTRIBUTE_HANDLER(ABI_align_needed, abiAlignNeeded),
   ATTRIBUTE_HANDLER(ABI_align_preserved, abiAlignPreserved),
   ATTRIBUTE_HANDLER(ABI_enum_size, abiEnumSize),
   ATTRIBUTE_HANDLER(ABI_HardFP_use, abiHardFpUse),
   ATTRIBUTE_HANDLER(ABI_VFP_args, abiVfpArgs),
   ATTRIBUTE_HANDLER(ABI_WMMX_args, abiWmmxArgs),
   ATTRIBUTE_HANDLER(ABI_optimization_goals, abiOptimizationGoals),
   ATTRIBUTE_HANDLER(ABI_FP_optimization_goals, abiFpOptimizationGoals),
   ATTRIBUTE_HANDLER(compatibility, compatibility),
   ATTRIBUTE_HANDLER(CPU_unaligned_access, cpuUnalignedAccess),
   ATTRIBUTE_HANDLER(FP_HP_extension, fpHpExtension),
   ATTRIBUTE_HANDLER(ABI_FP_16bit_format, abiFp16bitFormat),
   ATTRIBUTE_HANDLER(MPextension_use, mpExtensionUse),
   ATTRIBUTE_HANDLER(DIV_use, divUse),
   ATTRIBUTE_HANDLER(DSP_extension, dspExtension),
   ATTRIBUTE_HANDLER(T2EE_use, t2eeUse),
   ATTRIBUTE_HANDLER(Virtualization_use, virtualizationUse),
   ATTRIBUTE_HANDLER(nodefaults, nodefaults)
};

#undef ATTRIBUTE_HANDLER

uint64_t ARMAttributeParser::parseInteger(const uint8_t *data,
                                          uint32_t &offset)
{
   unsigned length;
   uint64_t value = decode_uleb128(data + offset, &length);
   offset = offset + length;
   return value;
}

StringRef ARMAttributeParser::parseString(const uint8_t *data,
                                          uint32_t &offset)
{
   const char *String = reinterpret_cast<const char*>(data + offset);
   size_t length = std::strlen(String);
   offset = offset + length + 1;
   return StringRef(String, length);
}

void ARMAttributeParser::integerAttribute(AttrType tag, const uint8_t *data,
                                          uint32_t &offset)
{

   uint64_t value = parseInteger(data, offset);
   m_attributes.insert(std::make_pair(tag, value));

   if (m_printer) {
      m_printer->printNumber(armbuildattrs::attr_type_as_string(tag), value);
   }
}

void ARMAttributeParser::stringAttribute(AttrType tag, const uint8_t *data,
                                         uint32_t &offset)
{
   StringRef tagName = armbuildattrs::attr_type_as_string(tag, /*TagPrefix*/false);
   StringRef valueDesc = parseString(data, offset);

   if (m_printer) {
      DictScope as(*m_printer, "Attribute");
      m_printer->printNumber("tag", tag);
      if (!tagName.empty()) {
         m_printer->printString("tagName", tagName);
      }
      m_printer->printString("value", valueDesc);
   }
}

void ARMAttributeParser::printAttribute(unsigned tag, unsigned value,
                                        StringRef valueDesc)
{
   m_attributes.insert(std::make_pair(tag, value));

   if (m_printer) {
      StringRef tagName = armbuildattrs::attr_type_as_string(tag,
                                                             /*TagPrefix*/false);
      DictScope as(*m_printer, "Attribute");
      m_printer->printNumber("tag", tag);
      m_printer->printNumber("value", value);
      if (!tagName.empty()) {
         m_printer->printString("tagName", tagName);
      }
      if (!valueDesc.empty()) {
         m_printer->printString("description", valueDesc);
      }
   }
}

void ARMAttributeParser::cpuArch(AttrType tag, const uint8_t *data,
                                 uint32_t &offset)
{
   static const char *const names[] = {
      "Pre-v4", "ARM v4", "ARM v4T", "ARM v5T", "ARM v5TE", "ARM v5TEJ", "ARM v6",
      "ARM v6KZ", "ARM v6T2", "ARM v6K", "ARM v7", "ARM v6-M", "ARM v6S-M",
      "ARM v7E-M", "ARM v8"
   };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(names)) ? names[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::cpuArchProfile(AttrType tag, const uint8_t *data,
                                        uint32_t &offset)
{
   uint64_t encoded = parseInteger(data, offset);

   StringRef profile;
   switch (encoded) {
   default:  profile = "Unknown"; break;
   case 'A': profile = "Application"; break;
   case 'R': profile = "Real-time"; break;
   case 'M': profile = "Microcontroller"; break;
   case 'S': profile = "Classic"; break;
   case 0: profile = "None"; break;
   }

   printAttribute(tag, encoded, profile);
}

void ARMAttributeParser::armIsaUse(AttrType tag, const uint8_t *data,
                                   uint32_t &offset)
{
   static const char *const strings[] = { "Not Permitted", "Permitted" };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::thumbIsaUse(AttrType tag, const uint8_t *data,
                                     uint32_t &offset)
{
   static const char *const strings[] = { "Not Permitted", "Thumb-1", "Thumb-2" };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::fpArch(AttrType tag, const uint8_t *data,
                                uint32_t &offset)
{
   static const char *const strings[] = {
      "Not Permitted", "VFPv1", "VFPv2", "VFPv3", "VFPv3-D16", "VFPv4",
      "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"
   };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::wmmxArch(AttrType tag, const uint8_t *data,
                                  uint32_t &offset)
{
   static const char *const strings[] = { "Not Permitted", "WMMXv1", "WMMXv2" };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::advancedSimdArch(AttrType tag, const uint8_t *data,
                                          uint32_t &offset)
{
   static const char *const strings[] = {
      "Not Permitted", "NEONv1", "NEONv2+FMA", "ARMv8-a NEON", "ARMv8.1-a NEON"
   };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::pcsConfig(AttrType tag, const uint8_t *data,
                                   uint32_t &offset)
{
   static const char *const strings[] = {
      "None", "Bare Platform", "Linux Application", "Linux DSO", "Palm OS 2004",
      "Reserved (Palm OS)", "Symbian OS 2004", "Reserved (Symbian OS)"
   };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::abiPcsR9Use(AttrType tag, const uint8_t *data,
                                     uint32_t &offset)
{
   static const char *const strings[] = { "v6", "Static Base", "TLS", "Unused" };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::abiPcsRwData(AttrType tag, const uint8_t *data,
                                      uint32_t &offset)
{
   static const char *const strings[] = {
      "Absolute", "PC-relative", "SB-relative", "Not Permitted"
   };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::abiPcsRoData(AttrType tag, const uint8_t *data,
                                      uint32_t &offset)
{
   static const char *const strings[] = {
      "Absolute", "PC-relative", "Not Permitted"
   };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::abiPcsGotUse(AttrType tag, const uint8_t *data,
                                      uint32_t &offset)
{
   static const char *const strings[] = {
      "Not Permitted", "Direct", "GOT-Indirect"
   };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::abiPcsWcharType(AttrType tag, const uint8_t *data,
                                         uint32_t &offset)
{
   static const char *const strings[] = {
      "Not Permitted", "Unknown", "2-byte", "Unknown", "4-byte"
   };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::abiFpRounding(AttrType tag, const uint8_t *data,
                                       uint32_t &offset)
{
   static const char *const strings[] = { "IEEE-754", "Runtime" };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::abiFpDenormal(AttrType tag, const uint8_t *data,
                                       uint32_t &offset)
{
   static const char *const strings[] = {
      "Unsupported", "IEEE-754", "Sign Only"
   };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::abiFpExceptions(AttrType tag, const uint8_t *data,
                                         uint32_t &offset)
{
   static const char *const strings[] = { "Not Permitted", "IEEE-754" };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::abiFpUserExceptions(AttrType tag,
                                             const uint8_t *data,
                                             uint32_t &offset)
{
   static const char *const strings[] = { "Not Permitted", "IEEE-754" };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::abiFpNumberModel(AttrType tag, const uint8_t *data,
                                          uint32_t &offset)
{
   static const char *const strings[] = {
      "Not Permitted", "Finite Only", "RTABI", "IEEE-754"
   };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::abiAlignNeeded(AttrType tag, const uint8_t *data,
                                        uint32_t &offset)
{
   static const char *const strings[] = {
      "Not Permitted", "8-byte alignment", "4-byte alignment", "Reserved"
   };

   uint64_t value = parseInteger(data, offset);

   std::string description;
   if (value < array_lengthof(strings)) {
      description = std::string(strings[value]);
   } else if (value <= 12) {
      description = std::string("8-byte alignment, ") + utostr(1ULL << value)
            + std::string("-byte extended alignment");
   } else {
      description = "Invalid";
   }
   printAttribute(tag, value, description);
}

void ARMAttributeParser::abiAlignPreserved(AttrType tag, const uint8_t *data,
                                           uint32_t &offset)
{
   static const char *const strings[] = {
      "Not Required", "8-byte data alignment", "8-byte data and code alignment",
      "Reserved"
   };

   uint64_t value = parseInteger(data, offset);

   std::string description;
   if (value < array_lengthof(strings)) {
      description = std::string(strings[value]);
   } else if (value <= 12) {
      description = std::string("8-byte stack alignment, ") +
            utostr(1ULL << value) + std::string("-byte data alignment");
   } else {
      description = "Invalid";
   }

   printAttribute(tag, value, description);
}

void ARMAttributeParser::abiEnumSize(AttrType tag, const uint8_t *data,
                                     uint32_t &offset)
{
   static const char *const strings[] = {
      "Not Permitted", "Packed", "Int32", "External Int32"
   };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::abiHardFpUse(AttrType tag, const uint8_t *data,
                                      uint32_t &offset)
{
   static const char *const strings[] = {
      "tag_FP_arch", "Single-Precision", "Reserved", "tag_FP_arch (deprecated)"
   };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::abiVfpArgs(AttrType tag, const uint8_t *data,
                                    uint32_t &offset)
{
   static const char *const strings[] = {
      "AAPCS", "AAPCS VFP", "Custom", "Not Permitted"
   };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::abiWmmxArgs(AttrType tag, const uint8_t *data,
                                     uint32_t &offset)
{
   static const char *const strings[] = { "AAPCS", "iWMMX", "Custom" };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::abiOptimizationGoals(AttrType tag,
                                              const uint8_t *data,
                                              uint32_t &offset)
{
   static const char *const strings[] = {
      "None", "Speed", "Aggressive Speed", "Size", "Aggressive Size", "Debugging",
      "Best Debugging"
   };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::abiFpOptimizationGoals(AttrType tag,
                                                const uint8_t *data,
                                                uint32_t &offset)
{
   static const char *const strings[] = {
      "None", "Speed", "Aggressive Speed", "Size", "Aggressive Size", "Accuracy",
      "Best Accuracy"
   };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::compatibility(AttrType tag, const uint8_t *data,
                                       uint32_t &offset) {
   uint64_t integer = parseInteger(data, offset);
   StringRef String = parseString(data, offset);

   if (m_printer) {
      DictScope as(*m_printer, "Attribute");
      m_printer->printNumber("tag", tag);
      m_printer->startLine() << "value: " << integer << ", " << String << '\n';
      m_printer->printString("tagName", armbuildattrs::attr_type_as_string(tag, /*TagPrefix*/false));
      switch (integer) {
      case 0:
         m_printer->printString("description", StringRef("No Specific Requirements"));
         break;
      case 1:
         m_printer->printString("description", StringRef("AEABI Conformant"));
         break;
      default:
         m_printer->printString("description", StringRef("AEABI Non-Conformant"));
         break;
      }
   }
}

void ARMAttributeParser::cpuUnalignedAccess(AttrType tag, const uint8_t *data,
                                            uint32_t &offset)
{
   static const char *const strings[] = { "Not Permitted", "v6-style" };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::fpHpExtension(AttrType tag, const uint8_t *data,
                                       uint32_t &offset)
{
   static const char *const strings[] = { "If Available", "Permitted" };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::abiFp16bitFormat(AttrType tag, const uint8_t *data,
                                          uint32_t &offset)
{
   static const char *const strings[] = { "Not Permitted", "IEEE-754", "VFPv3" };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::mpExtensionUse(AttrType tag, const uint8_t *data,
                                        uint32_t &offset)
{
   static const char *const strings[] = { "Not Permitted", "Permitted" };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::divUse(AttrType tag, const uint8_t *data,
                                uint32_t &offset)
{
   static const char *const strings[] = {
      "If Available", "Not Permitted", "Permitted"
   };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::dspExtension(AttrType tag, const uint8_t *data,
                                      uint32_t &offset)
{
   static const char *const strings[] = { "Not Permitted", "Permitted" };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::t2eeUse(AttrType tag, const uint8_t *data,
                                 uint32_t &offset)
{
   static const char *const strings[] = { "Not Permitted", "Permitted" };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::virtualizationUse(AttrType tag, const uint8_t *data,
                                           uint32_t &offset)
{
   static const char *const strings[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"
   };

   uint64_t value = parseInteger(data, offset);
   StringRef valueDesc =
         (value < array_lengthof(strings)) ? strings[value] : nullptr;
   printAttribute(tag, value, valueDesc);
}

void ARMAttributeParser::nodefaults(AttrType tag, const uint8_t *data,
                                    uint32_t &offset)
{
   uint64_t value = parseInteger(data, offset);
   printAttribute(tag, value, "Unspecified Tags UNDEFINED");
}

void ARMAttributeParser::parseIndexList(const uint8_t *data, uint32_t &offset,
                                        SmallVectorImpl<uint8_t> &IndexList)
{
   for (;;) {
      unsigned length;
      uint64_t value = decode_uleb128(data + offset, &length);
      offset = offset + length;
      if (value == 0) {
         break;
      }
      IndexList.push_back(value);
   }
}

void ARMAttributeParser::parseAttributeList(const uint8_t *data,
                                            uint32_t &offset, uint32_t length) {
   while (offset < length) {
      unsigned length;
      uint64_t tag = decode_uleb128(data + offset, &length);
      offset += length;

      bool handled = false;
      for (unsigned ahi = 0, ahe = array_lengthof(sm_displayRoutines);
           ahi != ahe && !handled; ++ahi) {
         if (uint64_t(sm_displayRoutines[ahi].m_attribute) == tag) {
            (this->*sm_displayRoutines[ahi].m_routine)(armbuildattrs::AttrType(tag),
                                                  data, offset);
            handled = true;
            break;
         }
      }
      if (!handled) {
         if (tag < 32) {
            error_stream() << "unhandled AEABI tag " << tag
                   << " (" << armbuildattrs::attr_type_as_string(tag) << ")\n";
            continue;
         }

         if (tag % 2 == 0)
            integerAttribute(armbuildattrs::AttrType(tag), data, offset);
         else
            stringAttribute(armbuildattrs::AttrType(tag), data, offset);
      }
   }
}

void ARMAttributeParser::parseSubsection(const uint8_t *data, uint32_t length)
{
   uint32_t offset = sizeof(uint32_t); /* SectionLength */

   const char *VendorName = reinterpret_cast<const char*>(data + offset);
   size_t vendorNameLength = std::strlen(VendorName);
   offset = offset + vendorNameLength + 1;

   if (m_printer) {
      m_printer->printNumber("SectionLength", length);
      m_printer->printString("Vendor", StringRef(VendorName, vendorNameLength));
   }

   if (StringRef(VendorName, vendorNameLength).toLower() != "aeabi") {
      return;
   }

   while (offset < length) {
      /// tag_File | tag_section | tag_Symbol   uleb128:byte-size
      uint8_t tag = data[offset];
      offset = offset + sizeof(tag);

      uint32_t Size =
            *reinterpret_cast<const utils::ulittle32_t*>(data + offset);
      offset = offset + sizeof(Size);

      if (m_printer) {
         m_printer->printEnum("tag", tag, make_array_ref(sg_tagNames));
         m_printer->printNumber("Size", Size);
      }

      if (Size > length) {
         error_stream() << "subsection length greater than section length\n";
         return;
      }

      StringRef scopeName, indexName;
      SmallVector<uint8_t, 8> indicies;
      switch (tag) {
      case armbuildattrs::File:
         scopeName = "Filem_attributes";
         break;
      case armbuildattrs::Section:
         scopeName = "Sectionm_attributes";
         indexName = "Sections";
         parseIndexList(data, offset, indicies);
         break;
      case armbuildattrs::Symbol:
         scopeName = "Symbolm_attributes";
         indexName = "Symbols";
         parseIndexList(data, offset, indicies);
         break;
      default:
         error_stream() << "unrecognised tag: 0x" << Twine::utohexstr(tag) << '\n';
         return;
      }

      if (m_printer) {
         DictScope ASS(*m_printer, scopeName);
         if (!indicies.empty())
            m_printer->printList(indexName, indicies);
         parseAttributeList(data, offset, length);
      } else {
         parseAttributeList(data, offset, length);
      }
   }
}

void ARMAttributeParser::parse(ArrayRef<uint8_t> section, bool isLittle) {
   size_t offset = 1;
   unsigned sectionNumber = 0;

   while (offset < section.getSize()) {
      uint32_t SectionLength = isLittle ?
               utils::endian::read32le(section.getData() + offset) :
               utils::endian::read32be(section.getData() + offset);

      if (m_printer) {
         m_printer->startLine() << "section " << ++sectionNumber << " {\n";
         m_printer->indent();
      }

      parseSubsection(section.getData() + offset, SectionLength);
      offset = offset + SectionLength;

      if (m_printer) {
         m_printer->unindent();
         m_printer->startLine() << "}\n";
      }
   }
}

} // polar
