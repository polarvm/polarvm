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
{ armbuildattrs::File, "Tag_File" },
{ armbuildattrs::Section, "Tag_Section" },
{ armbuildattrs::Symbol, "Tag_Symbol" },
{ armbuildattrs::CPU_raw_name, "Tag_CPU_raw_name" },
{ armbuildattrs::CPU_name, "Tag_CPU_name" },
{ armbuildattrs::CPU_arch, "Tag_CPU_arch" },
{ armbuildattrs::CPU_arch_profile, "Tag_CPU_arch_profile" },
{ armbuildattrs::ARM_ISA_use, "Tag_ARM_ISA_use" },
{ armbuildattrs::THUMB_ISA_use, "Tag_THUMB_ISA_use" },
{ armbuildattrs::FP_arch, "Tag_FP_arch" },
{ armbuildattrs::WMMX_arch, "Tag_WMMX_arch" },
{ armbuildattrs::Advanced_SIMD_arch, "Tag_Advanced_SIMD_arch" },
{ armbuildattrs::PCS_config, "Tag_PCS_config" },
{ armbuildattrs::ABI_PCS_R9_use, "Tag_ABI_PCS_R9_use" },
{ armbuildattrs::ABI_PCS_RW_data, "Tag_ABI_PCS_RW_data" },
{ armbuildattrs::ABI_PCS_RO_data, "Tag_ABI_PCS_RO_data" },
{ armbuildattrs::ABI_PCS_GOT_use, "Tag_ABI_PCS_GOT_use" },
{ armbuildattrs::ABI_PCS_wchar_t, "Tag_ABI_PCS_wchar_t" },
{ armbuildattrs::ABI_FP_rounding, "Tag_ABI_FP_rounding" },
{ armbuildattrs::ABI_FP_denormal, "Tag_ABI_FP_denormal" },
{ armbuildattrs::ABI_FP_exceptions, "Tag_ABI_FP_exceptions" },
{ armbuildattrs::ABI_FP_user_exceptions, "Tag_ABI_FP_user_exceptions" },
{ armbuildattrs::ABI_FP_number_model, "Tag_ABI_FP_number_model" },
{ armbuildattrs::ABI_align_needed, "Tag_ABI_align_needed" },
{ armbuildattrs::ABI_align_preserved, "Tag_ABI_align_preserved" },
{ armbuildattrs::ABI_enum_size, "Tag_ABI_enum_size" },
{ armbuildattrs::ABI_HardFP_use, "Tag_ABI_HardFP_use" },
{ armbuildattrs::ABI_VFP_args, "Tag_ABI_VFP_args" },
{ armbuildattrs::ABI_WMMX_args, "Tag_ABI_WMMX_args" },
{ armbuildattrs::ABI_optimization_goals, "Tag_ABI_optimization_goals" },
{ armbuildattrs::ABI_FP_optimization_goals, "Tag_ABI_FP_optimization_goals" },
{ armbuildattrs::compatibility, "Tag_compatibility" },
{ armbuildattrs::CPU_unaligned_access, "Tag_CPU_unaligned_access" },
{ armbuildattrs::FP_HP_extension, "Tag_FP_HP_extension" },
{ armbuildattrs::ABI_FP_16bit_format, "Tag_ABI_FP_16bit_format" },
{ armbuildattrs::MPextension_use, "Tag_MPextension_use" },
{ armbuildattrs::DIV_use, "Tag_DIV_use" },
{ armbuildattrs::DSP_extension, "Tag_DSP_extension" },
{ armbuildattrs::nodefaults, "Tag_nodefaults" },
{ armbuildattrs::also_compatible_with, "Tag_also_compatible_with" },
{ armbuildattrs::T2EE_use, "Tag_T2EE_use" },
{ armbuildattrs::conformance, "Tag_conformance" },
{ armbuildattrs::Virtualization_use, "Tag_Virtualization_use" },

// Legacy Names
{ armbuildattrs::FP_arch, "Tag_VFP_arch" },
{ armbuildattrs::FP_HP_extension, "Tag_VFP_HP_extension" },
{ armbuildattrs::ABI_align_needed, "Tag_ABI_align8_needed" },
{ armbuildattrs::ABI_align_preserved, "Tag_ABI_align8_preserved" },
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
   bool hasTagPrefix = tag.startsWith("Tag_");
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

