// This source file is part of the polarphp.org open source project
//
// Copyright (c) 2017 - 2018 polarPHP software foundation
// Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://polarphp.org/LICENSE.txt for license information
// See http://polarphp.org/CONTRIBUTORS.txt for the list of polarPHP project authors
//
// Created by softboy on 2018/07/12.

#include "polar/utils/ARMAttributeParser.h"
#include "polar/utils/ARMBuildAttributes.h"
#include "gtest/gtest.h"
#include <string>

using namespace polar::basic;
using namespace polar::utils;
using namespace polar;

namespace {

struct AttributeSection
{
   unsigned Tag;
   unsigned Value;

   AttributeSection(unsigned tag, unsigned value) : Tag(tag), Value(value) { }

   void write(RawOutStream &outstream) {
      outstream.flush();
      // length = length + "aeabi\0" + TagFile + ByteSize + Tag + Value;
      // length = 17 bytes

      outstream << 'A' << (uint8_t)17 << (uint8_t)0 << (uint8_t)0 << (uint8_t)0;
      outstream << "aeabi" << '\0';
      outstream << (uint8_t)1 << (uint8_t)7 << (uint8_t)0 << (uint8_t)0 << (uint8_t)0;
      outstream << (uint8_t)Tag << (uint8_t)Value;

   }
};

bool test_build_attr(unsigned Tag, unsigned Value,
                   unsigned ExpectedTag, unsigned ExpectedValue)
{
   std::string buffer;
   RawStringOutStream outstream(buffer);
   AttributeSection Section(Tag, Value);
   Section.write(outstream);
   ArrayRef<uint8_t> Bytes(
            reinterpret_cast<const uint8_t*>(outstream.getStr().c_str()), outstream.getStr().size());

   ARMAttributeParser parser;
   parser.parse(Bytes, true);

   return (parser.hasAttribute(ExpectedTag) &&
           parser.getAttributeValue(ExpectedTag) == ExpectedValue);
}

bool testTagString(unsigned Tag, const char *name)
{
   return armbuildattrs::attr_type_as_string(Tag).getStr() == name;
}

TEST(ARMAttributeParserTest, testCPUArchBuildAttr)
{
   EXPECT_TRUE(testTagString(6, "Tag_CPU_arch"));

   EXPECT_TRUE(test_build_attr(6, 0, armbuildattrs::CPU_arch,
                             armbuildattrs::Pre_v4));
   EXPECT_TRUE(test_build_attr(6, 1, armbuildattrs::CPU_arch,
                             armbuildattrs::v4));
   EXPECT_TRUE(test_build_attr(6, 2, armbuildattrs::CPU_arch,
                             armbuildattrs::v4T));
   EXPECT_TRUE(test_build_attr(6, 3, armbuildattrs::CPU_arch,
                             armbuildattrs::v5T));
   EXPECT_TRUE(test_build_attr(6, 4, armbuildattrs::CPU_arch,
                             armbuildattrs::v5TE));
   EXPECT_TRUE(test_build_attr(6, 5, armbuildattrs::CPU_arch,
                             armbuildattrs::v5TEJ));
   EXPECT_TRUE(test_build_attr(6, 6, armbuildattrs::CPU_arch,
                             armbuildattrs::v6));
   EXPECT_TRUE(test_build_attr(6, 7, armbuildattrs::CPU_arch,
                             armbuildattrs::v6KZ));
   EXPECT_TRUE(test_build_attr(6, 8, armbuildattrs::CPU_arch,
                             armbuildattrs::v6T2));
   EXPECT_TRUE(test_build_attr(6, 9, armbuildattrs::CPU_arch,
                             armbuildattrs::v6K));
   EXPECT_TRUE(test_build_attr(6, 10, armbuildattrs::CPU_arch,
                             armbuildattrs::v7));
   EXPECT_TRUE(test_build_attr(6, 11, armbuildattrs::CPU_arch,
                             armbuildattrs::v6_M));
   EXPECT_TRUE(test_build_attr(6, 12, armbuildattrs::CPU_arch,
                             armbuildattrs::v6S_M));
   EXPECT_TRUE(test_build_attr(6, 13, armbuildattrs::CPU_arch,
                             armbuildattrs::v7E_M));
}

TEST(ARMAttributeParserTest, testCPUArchProfileBuildAttr)
{
   EXPECT_TRUE(testTagString(7, "Tag_CPU_arch_profile"));
   EXPECT_TRUE(test_build_attr(7, 'A', armbuildattrs::CPU_arch_profile,
                             armbuildattrs::ApplicationProfile));
   EXPECT_TRUE(test_build_attr(7, 'R', armbuildattrs::CPU_arch_profile,
                             armbuildattrs::RealTimeProfile));
   EXPECT_TRUE(test_build_attr(7, 'M', armbuildattrs::CPU_arch_profile,
                             armbuildattrs::MicroControllerProfile));
   EXPECT_TRUE(test_build_attr(7, 'S', armbuildattrs::CPU_arch_profile,
                             armbuildattrs::SystemProfile));
}

TEST(ARMAttributeParserTest, testARMISABuildAttr)
{
   EXPECT_TRUE(testTagString(8, "Tag_ARM_ISA_use"));
   EXPECT_TRUE(test_build_attr(8, 0, armbuildattrs::ARM_ISA_use,
                             armbuildattrs::Not_Allowed));
   EXPECT_TRUE(test_build_attr(8, 1, armbuildattrs::ARM_ISA_use,
                             armbuildattrs::Allowed));
}

TEST(ARMAttributeParserTest, testThumbISABuildAttr)
{
   EXPECT_TRUE(testTagString(9, "Tag_THUMB_ISA_use"));
   EXPECT_TRUE(test_build_attr(9, 0, armbuildattrs::THUMB_ISA_use,
                             armbuildattrs::Not_Allowed));
   EXPECT_TRUE(test_build_attr(9, 1, armbuildattrs::THUMB_ISA_use,
                             armbuildattrs::Allowed));
}

TEST(ARMAttributeParserTest, testFPArchBuildAttr)
{
   EXPECT_TRUE(testTagString(10, "Tag_FP_arch"));
   EXPECT_TRUE(test_build_attr(10, 0, armbuildattrs::FP_arch,
                             armbuildattrs::Not_Allowed));
   EXPECT_TRUE(test_build_attr(10, 1, armbuildattrs::FP_arch,
                             armbuildattrs::Allowed));
   EXPECT_TRUE(test_build_attr(10, 2, armbuildattrs::FP_arch,
                             armbuildattrs::AllowFPv2));
   EXPECT_TRUE(test_build_attr(10, 3, armbuildattrs::FP_arch,
                             armbuildattrs::AllowFPv3A));
   EXPECT_TRUE(test_build_attr(10, 4, armbuildattrs::FP_arch,
                             armbuildattrs::AllowFPv3B));
   EXPECT_TRUE(test_build_attr(10, 5, armbuildattrs::FP_arch,
                             armbuildattrs::AllowFPv4A));
   EXPECT_TRUE(test_build_attr(10, 6, armbuildattrs::FP_arch,
                             armbuildattrs::AllowFPv4B));
   EXPECT_TRUE(test_build_attr(10, 7, armbuildattrs::FP_arch,
                             armbuildattrs::AllowFPARMv8A));
   EXPECT_TRUE(test_build_attr(10, 8, armbuildattrs::FP_arch,
                             armbuildattrs::AllowFPARMv8B));
}

TEST(ARMAttributeParserTest, testWMMXBuildAttr)
{
   EXPECT_TRUE(testTagString(11, "Tag_WMMX_arch"));
   EXPECT_TRUE(test_build_attr(11, 0, armbuildattrs::WMMX_arch,
                             armbuildattrs::Not_Allowed));
   EXPECT_TRUE(test_build_attr(11, 1, armbuildattrs::WMMX_arch,
                             armbuildattrs::AllowWMMXv1));
   EXPECT_TRUE(test_build_attr(11, 2, armbuildattrs::WMMX_arch,
                             armbuildattrs::AllowWMMXv2));
}

TEST(ARMAttributeParserTest, testSIMDBuildAttr)
{
   EXPECT_TRUE(testTagString(12, "Tag_Advanced_SIMD_arch"));
   EXPECT_TRUE(test_build_attr(12, 0, armbuildattrs::Advanced_SIMD_arch,
                             armbuildattrs::Not_Allowed));
   EXPECT_TRUE(test_build_attr(12, 1, armbuildattrs::Advanced_SIMD_arch,
                             armbuildattrs::AllowNeon));
   EXPECT_TRUE(test_build_attr(12, 2, armbuildattrs::Advanced_SIMD_arch,
                             armbuildattrs::AllowNeon2));
   EXPECT_TRUE(test_build_attr(12, 3, armbuildattrs::Advanced_SIMD_arch,
                             armbuildattrs::AllowNeonARMv8));
   EXPECT_TRUE(test_build_attr(12, 4, armbuildattrs::Advanced_SIMD_arch,
                             armbuildattrs::AllowNeonARMv8_1a));
}

TEST(ARMAttributeParserTest, testFPHPBuildAttr)
{
   EXPECT_TRUE(testTagString(36, "Tag_FP_HP_extension"));
   EXPECT_TRUE(test_build_attr(36, 0, armbuildattrs::FP_HP_extension,
                             armbuildattrs::Not_Allowed));
   EXPECT_TRUE(test_build_attr(36, 1, armbuildattrs::FP_HP_extension,
                             armbuildattrs::AllowHPFP));
}

TEST(ARMAttributeParserTest, testCPUAlignBuildAttr)
{
   EXPECT_TRUE(testTagString(34, "Tag_CPU_unaligned_access"));
   EXPECT_TRUE(test_build_attr(34, 0, armbuildattrs::CPU_unaligned_access,
                             armbuildattrs::Not_Allowed));
   EXPECT_TRUE(test_build_attr(34, 1, armbuildattrs::CPU_unaligned_access,
                             armbuildattrs::Allowed));
}

TEST(ARMAttributeParserTest, testT2EEBuildAttr)
{
   EXPECT_TRUE(testTagString(66, "Tag_T2EE_use"));
   EXPECT_TRUE(test_build_attr(66, 0, armbuildattrs::T2EE_use,
                             armbuildattrs::Not_Allowed));
   EXPECT_TRUE(test_build_attr(66, 1, armbuildattrs::T2EE_use,
                             armbuildattrs::Allowed));
}

TEST(ARMAttributeParserTest, testVirtualizationBuildAttr)
{
   EXPECT_TRUE(testTagString(68, "Tag_Virtualization_use"));
   EXPECT_TRUE(test_build_attr(68, 0, armbuildattrs::Virtualization_use,
                             armbuildattrs::Not_Allowed));
   EXPECT_TRUE(test_build_attr(68, 1, armbuildattrs::Virtualization_use,
                             armbuildattrs::AllowTZ));
   EXPECT_TRUE(test_build_attr(68, 2, armbuildattrs::Virtualization_use,
                             armbuildattrs::AllowVirtualization));
   EXPECT_TRUE(test_build_attr(68, 3, armbuildattrs::Virtualization_use,
                             armbuildattrs::AllowTZVirtualization));
}

TEST(ARMAttributeParserTest, testMPBuildAttr)
{
   EXPECT_TRUE(testTagString(42, "Tag_MPextension_use"));
   EXPECT_TRUE(test_build_attr(42, 0, armbuildattrs::MPextension_use,
                             armbuildattrs::Not_Allowed));
   EXPECT_TRUE(test_build_attr(42, 1, armbuildattrs::MPextension_use,
                             armbuildattrs::AllowMP));
}

TEST(ARMAttributeParserTest, testDivBuildAttr)
{
   EXPECT_TRUE(testTagString(44, "Tag_DIV_use"));
   EXPECT_TRUE(test_build_attr(44, 0, armbuildattrs::DIV_use,
                             armbuildattrs::AllowDIVIfExists));
   EXPECT_TRUE(test_build_attr(44, 1, armbuildattrs::DIV_use,
                             armbuildattrs::DisallowDIV));
   EXPECT_TRUE(test_build_attr(44, 2, armbuildattrs::DIV_use,
                             armbuildattrs::AllowDIVExt));
}

TEST(ARMAttributeParserTest, testPCSConfigBuildAttr)
{
   EXPECT_TRUE(testTagString(13, "Tag_PCS_config"));
   EXPECT_TRUE(test_build_attr(13, 0, armbuildattrs::PCS_config, 0));
   EXPECT_TRUE(test_build_attr(13, 1, armbuildattrs::PCS_config, 1));
   EXPECT_TRUE(test_build_attr(13, 2, armbuildattrs::PCS_config, 2));
   EXPECT_TRUE(test_build_attr(13, 3, armbuildattrs::PCS_config, 3));
   EXPECT_TRUE(test_build_attr(13, 4, armbuildattrs::PCS_config, 4));
   EXPECT_TRUE(test_build_attr(13, 5, armbuildattrs::PCS_config, 5));
   EXPECT_TRUE(test_build_attr(13, 6, armbuildattrs::PCS_config, 6));
   EXPECT_TRUE(test_build_attr(13, 7, armbuildattrs::PCS_config, 7));
}

TEST(ARMAttributeParserTest, testPCSR9BuildAttr)
{
   EXPECT_TRUE(testTagString(14, "Tag_ABI_PCS_R9_use"));
   EXPECT_TRUE(test_build_attr(14, 0, armbuildattrs::ABI_PCS_R9_use,
                             armbuildattrs::R9IsGPR));
   EXPECT_TRUE(test_build_attr(14, 1, armbuildattrs::ABI_PCS_R9_use,
                             armbuildattrs::R9IsSB));
   EXPECT_TRUE(test_build_attr(14, 2, armbuildattrs::ABI_PCS_R9_use,
                             armbuildattrs::R9IsTLSPointer));
   EXPECT_TRUE(test_build_attr(14, 3, armbuildattrs::ABI_PCS_R9_use,
                             armbuildattrs::R9Reserved));
}

TEST(ARMAttributeParserTest, testPCSRWBuildAttr)
{
   EXPECT_TRUE(testTagString(15, "Tag_ABI_PCS_RW_data"));
   EXPECT_TRUE(test_build_attr(15, 0, armbuildattrs::ABI_PCS_RW_data,
                             armbuildattrs::Not_Allowed));
   EXPECT_TRUE(test_build_attr(15, 1, armbuildattrs::ABI_PCS_RW_data,
                             armbuildattrs::AddressRWPCRel));
   EXPECT_TRUE(test_build_attr(15, 2, armbuildattrs::ABI_PCS_RW_data,
                             armbuildattrs::AddressRWSBRel));
   EXPECT_TRUE(test_build_attr(15, 3, armbuildattrs::ABI_PCS_RW_data,
                             armbuildattrs::AddressRWNone));
}

TEST(ARMAttributeParserTest, testPCSROBuildAttr)
{
   EXPECT_TRUE(testTagString(16, "Tag_ABI_PCS_RO_data"));
   EXPECT_TRUE(test_build_attr(16, 0, armbuildattrs::ABI_PCS_RO_data,
                             armbuildattrs::Not_Allowed));
   EXPECT_TRUE(test_build_attr(16, 1, armbuildattrs::ABI_PCS_RO_data,
                             armbuildattrs::AddressROPCRel));
   EXPECT_TRUE(test_build_attr(16, 2, armbuildattrs::ABI_PCS_RO_data,
                             armbuildattrs::AddressRONone));
}

TEST(ARMAttributeParserTest, testPCSGOTBuildAttr)
{
   EXPECT_TRUE(testTagString(17, "Tag_ABI_PCS_GOT_use"));
   EXPECT_TRUE(test_build_attr(17, 0, armbuildattrs::ABI_PCS_GOT_use,
                             armbuildattrs::Not_Allowed));
   EXPECT_TRUE(test_build_attr(17, 1, armbuildattrs::ABI_PCS_GOT_use,
                             armbuildattrs::AddressDirect));
   EXPECT_TRUE(test_build_attr(17, 2, armbuildattrs::ABI_PCS_GOT_use,
                             armbuildattrs::AddressGOT));
}

TEST(ARMAttributeParserTest, testPCSWCharBuildAttr)
{
   EXPECT_TRUE(testTagString(18, "Tag_ABI_PCS_wchar_t"));
   EXPECT_TRUE(test_build_attr(18, 0, armbuildattrs::ABI_PCS_wchar_t,
                             armbuildattrs::WCharProhibited));
   EXPECT_TRUE(test_build_attr(18, 2, armbuildattrs::ABI_PCS_wchar_t,
                             armbuildattrs::WCharWidth2Bytes));
   EXPECT_TRUE(test_build_attr(18, 4, armbuildattrs::ABI_PCS_wchar_t,
                             armbuildattrs::WCharWidth4Bytes));
}

TEST(ARMAttributeParserTest, testEnumSizeBuildAttr)
{
   EXPECT_TRUE(testTagString(26, "Tag_ABI_enum_size"));
   EXPECT_TRUE(test_build_attr(26, 0, armbuildattrs::ABI_enum_size,
                             armbuildattrs::EnumProhibited));
   EXPECT_TRUE(test_build_attr(26, 1, armbuildattrs::ABI_enum_size,
                             armbuildattrs::EnumSmallest));
   EXPECT_TRUE(test_build_attr(26, 2, armbuildattrs::ABI_enum_size,
                             armbuildattrs::Enum32Bit));
   EXPECT_TRUE(test_build_attr(26, 3, armbuildattrs::ABI_enum_size,
                             armbuildattrs::Enum32BitABI));
}

TEST(ARMAttributeParserTest, testAlignNeededBuildAttr)
{
   EXPECT_TRUE(testTagString(24, "Tag_ABI_align_needed"));
   EXPECT_TRUE(test_build_attr(24, 0, armbuildattrs::ABI_align_needed,
                             armbuildattrs::Not_Allowed));
   EXPECT_TRUE(test_build_attr(24, 1, armbuildattrs::ABI_align_needed,
                             armbuildattrs::Align8Byte));
   EXPECT_TRUE(test_build_attr(24, 2, armbuildattrs::ABI_align_needed,
                             armbuildattrs::Align4Byte));
   EXPECT_TRUE(test_build_attr(24, 3, armbuildattrs::ABI_align_needed,
                             armbuildattrs::AlignReserved));
}

TEST(ARMAttributeParserTest, testAlignPreservedBuildAttr)
{
   EXPECT_TRUE(testTagString(25, "Tag_ABI_align_preserved"));
   EXPECT_TRUE(test_build_attr(25, 0, armbuildattrs::ABI_align_preserved,
                             armbuildattrs::AlignNotPreserved));
   EXPECT_TRUE(test_build_attr(25, 1, armbuildattrs::ABI_align_preserved,
                             armbuildattrs::AlignPreserve8Byte));
   EXPECT_TRUE(test_build_attr(25, 2, armbuildattrs::ABI_align_preserved,
                             armbuildattrs::AlignPreserveAll));
   EXPECT_TRUE(test_build_attr(25, 3, armbuildattrs::ABI_align_preserved,
                             armbuildattrs::AlignReserved));
}

TEST(ARMAttributeParserTest, testFPRoundingBuildAttr)
{
   EXPECT_TRUE(testTagString(19, "Tag_ABI_FP_rounding"));
   EXPECT_TRUE(test_build_attr(19, 0, armbuildattrs::ABI_FP_rounding, 0));
   EXPECT_TRUE(test_build_attr(19, 1, armbuildattrs::ABI_FP_rounding, 1));
}

TEST(ARMAttributeParserTest, testFPDenormalBuildAttr)
{
   EXPECT_TRUE(testTagString(20, "Tag_ABI_FP_denormal"));
   EXPECT_TRUE(test_build_attr(20, 0, armbuildattrs::ABI_FP_denormal,
                             armbuildattrs::PositiveZero));
   EXPECT_TRUE(test_build_attr(20, 1, armbuildattrs::ABI_FP_denormal,
                             armbuildattrs::IEEEDenormals));
   EXPECT_TRUE(test_build_attr(20, 2, armbuildattrs::ABI_FP_denormal,
                             armbuildattrs::PreserveFPSign));
}

TEST(ARMAttributeParserTest, testFPExceptionsBuildAttr)
{
   EXPECT_TRUE(testTagString(21, "Tag_ABI_FP_exceptions"));
   EXPECT_TRUE(test_build_attr(21, 0, armbuildattrs::ABI_FP_exceptions, 0));
   EXPECT_TRUE(test_build_attr(21, 1, armbuildattrs::ABI_FP_exceptions, 1));
}

TEST(ARMAttributeParserTest, testFPUserExceptionsBuildAttr)
{
   EXPECT_TRUE(testTagString(22, "Tag_ABI_FP_user_exceptions"));
   EXPECT_TRUE(test_build_attr(22, 0, armbuildattrs::ABI_FP_user_exceptions, 0));
   EXPECT_TRUE(test_build_attr(22, 1, armbuildattrs::ABI_FP_user_exceptions, 1));
}

TEST(ARMAttributeParserTest, testFPNumberModelBuildAttr)
{
   EXPECT_TRUE(testTagString(23, "Tag_ABI_FP_number_model"));
   EXPECT_TRUE(test_build_attr(23, 0, armbuildattrs::ABI_FP_number_model,
                             armbuildattrs::Not_Allowed));
   EXPECT_TRUE(test_build_attr(23, 1, armbuildattrs::ABI_FP_number_model,
                             armbuildattrs::AllowIEEENormal));
   EXPECT_TRUE(test_build_attr(23, 2, armbuildattrs::ABI_FP_number_model,
                             armbuildattrs::AllowRTABI));
   EXPECT_TRUE(test_build_attr(23, 3, armbuildattrs::ABI_FP_number_model,
                             armbuildattrs::AllowIEEE754));
}

TEST(ARMAttributeParserTest, testFP16BuildAttr)
{
   EXPECT_TRUE(testTagString(38, "Tag_ABI_FP_16bit_format"));
   EXPECT_TRUE(test_build_attr(38, 0, armbuildattrs::ABI_FP_16bit_format,
                             armbuildattrs::Not_Allowed));
   EXPECT_TRUE(test_build_attr(38, 1, armbuildattrs::ABI_FP_16bit_format,
                             armbuildattrs::FP16FormatIEEE));
   EXPECT_TRUE(test_build_attr(38, 2, armbuildattrs::ABI_FP_16bit_format,
                             armbuildattrs::FP16VFP3));
}

TEST(ARMAttributeParserTest, testHardFPBuildAttr)
{
   EXPECT_TRUE(testTagString(27, "Tag_ABI_HardFP_use"));
   EXPECT_TRUE(test_build_attr(27, 0, armbuildattrs::ABI_HardFP_use,
                             armbuildattrs::HardFPImplied));
   EXPECT_TRUE(test_build_attr(27, 1, armbuildattrs::ABI_HardFP_use,
                             armbuildattrs::HardFPSinglePrecision));
   EXPECT_TRUE(test_build_attr(27, 2, armbuildattrs::ABI_HardFP_use, 2));
}

TEST(ARMAttributeParserTest, testVFPArgsBuildAttr)
{
   EXPECT_TRUE(testTagString(28, "Tag_ABI_VFP_args"));
   EXPECT_TRUE(test_build_attr(28, 0, armbuildattrs::ABI_VFP_args,
                             armbuildattrs::BaseAAPCS));
   EXPECT_TRUE(test_build_attr(28, 1, armbuildattrs::ABI_VFP_args,
                             armbuildattrs::HardFPAAPCS));
   EXPECT_TRUE(test_build_attr(28, 2, armbuildattrs::ABI_VFP_args, 2));
   EXPECT_TRUE(test_build_attr(28, 3, armbuildattrs::ABI_VFP_args, 3));
}

TEST(ARMAttributeParserTest, testWMMXArgsBuildAttr)
{
   EXPECT_TRUE(testTagString(29, "Tag_ABI_WMMX_args"));
   EXPECT_TRUE(test_build_attr(29, 0, armbuildattrs::ABI_WMMX_args, 0));
   EXPECT_TRUE(test_build_attr(29, 1, armbuildattrs::ABI_WMMX_args, 1));
   EXPECT_TRUE(test_build_attr(29, 2, armbuildattrs::ABI_WMMX_args, 2));
}

} // anonymous namespace
