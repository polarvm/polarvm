// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/02.

#include "polar/basic/adt/StringRef.h"
#include "polar/utils/ARMBuildAttributes.h"

namespace polar {

namespace {
const struct {
   armbuildattrs::AttrType m_attr;
   StringRef m_tagName;
} g_armAttributeTags[] = {
{ armbuildattrs::File, "tag_File" },
{ armbuildattrs::Section, "tag_Section" },
{ armbuildattrs::Symbol, "tag_Symbol" },
{ armbuildattrs::CPU_raw_name, "tag_CPU_raw_name" },
{ armbuildattrs::CPU_name, "tag_CPU_name" },
{ armbuildattrs::CPU_arch, "tag_CPU_arch" },
{ armbuildattrs::CPU_arch_profile, "tag_CPU_arch_profile" },
{ armbuildattrs::ARM_ISA_use, "tag_ARM_ISA_use" },
{ armbuildattrs::THUMB_ISA_use, "tag_THUMB_ISA_use" },
{ armbuildattrs::FP_arch, "tag_FP_arch" },
{ armbuildattrs::WMMX_arch, "tag_WMMX_arch" },
{ armbuildattrs::Advanced_SIMD_arch, "tag_Advanced_SIMD_arch" },
{ armbuildattrs::PCS_config, "tag_PCS_config" },
{ armbuildattrs::ABI_PCS_R9_use, "tag_ABI_PCS_R9_use" },
{ armbuildattrs::ABI_PCS_RW_data, "tag_ABI_PCS_RW_data" },
{ armbuildattrs::ABI_PCS_RO_data, "tag_ABI_PCS_RO_data" },
{ armbuildattrs::ABI_PCS_GOT_use, "tag_ABI_PCS_GOT_use" },
{ armbuildattrs::ABI_PCS_wchar_t, "tag_ABI_PCS_wchar_t" },
{ armbuildattrs::ABI_FP_rounding, "tag_ABI_FP_rounding" },
{ armbuildattrs::ABI_FP_denormal, "tag_ABI_FP_denormal" },
{ armbuildattrs::ABI_FP_exceptions, "tag_ABI_FP_exceptions" },
{ armbuildattrs::ABI_FP_user_exceptions, "tag_ABI_FP_user_exceptions" },
{ armbuildattrs::ABI_FP_number_model, "tag_ABI_FP_number_model" },
{ armbuildattrs::ABI_align_needed, "tag_ABI_align_needed" },
{ armbuildattrs::ABI_align_preserved, "tag_ABI_align_preserved" },
{ armbuildattrs::ABI_enum_size, "tag_ABI_enum_size" },
{ armbuildattrs::ABI_HardFP_use, "tag_ABI_HardFP_use" },
{ armbuildattrs::ABI_VFP_args, "tag_ABI_VFP_args" },
{ armbuildattrs::ABI_WMMX_args, "tag_ABI_WMMX_args" },
{ armbuildattrs::ABI_optimization_goals, "tag_ABI_optimization_goals" },
{ armbuildattrs::ABI_FP_optimization_goals, "tag_ABI_FP_optimization_goals" },
{ armbuildattrs::compatibility, "tag_compatibility" },
{ armbuildattrs::CPU_unaligned_access, "tag_CPU_unaligned_access" },
{ armbuildattrs::FP_HP_extension, "tag_FP_HP_extension" },
{ armbuildattrs::ABI_FP_16bit_format, "tag_ABI_FP_16bit_format" },
{ armbuildattrs::MPextension_use, "tag_MPextension_use" },
{ armbuildattrs::DIV_use, "tag_DIV_use" },
{ armbuildattrs::DSP_extension, "tag_DSP_extension" },
{ armbuildattrs::nodefaults, "tag_nodefaults" },
{ armbuildattrs::also_compatible_with, "tag_also_compatible_with" },
{ armbuildattrs::T2EE_use, "tag_T2EE_use" },
{ armbuildattrs::conformance, "tag_conformance" },
{ armbuildattrs::Virtualization_use, "tag_Virtualization_use" },

// Legacy Names
{ armbuildattrs::FP_arch, "tag_VFP_arch" },
{ armbuildattrs::FP_HP_extension, "tag_VFP_HP_extension" },
{ armbuildattrs::ABI_align_needed, "tag_ABI_align8_needed" },
{ armbuildattrs::ABI_align_preserved, "tag_ABI_align8_preserved" },
};
}

namespace armbuildattrs {

StringRef attr_type_as_string(unsigned attr, bool hasTagPrefix)
{
   return attr_type_as_string(static_cast<AttrType>(attr), hasTagPrefix);
}

StringRef attr_type_as_string(AttrType attr, bool hasTagPrefix)
{
   for (unsigned titer = 0, tend = sizeof(g_armAttributeTags) / sizeof(*g_armAttributeTags);
        titer != tend; ++titer)
      if (g_armAttributeTags[titer].m_attr == attr) {
         auto tagName = g_armAttributeTags[titer].m_tagName;
         return hasTagPrefix ? tagName : tagName.dropFront(4);
      }
   return "";
}

int attr_type_from_string(StringRef tag)
{
   bool hasTagPrefix = tag.startsWith("tag_");
   for (unsigned titer = 0,
        tend = sizeof(g_armAttributeTags) / sizeof(*g_armAttributeTags);
        titer != tend; ++titer) {
      auto tagName = g_armAttributeTags[titer].m_tagName;
      if (tagName.dropFront(hasTagPrefix ? 0 : 4) == tag) {
         return g_armAttributeTags[titer].m_attr;
      }
   }
   return -1;
}

} // armbuildattrs

} // polar

