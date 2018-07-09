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

//===----------------------------------------------------------------------===//
//
// This file implements a target parser to recognise hardware features such as
// FPU/CPU/ARCH names as well as specific support such as HDIV, etc.
//
//===----------------------------------------------------------------------===//

#ifndef POLAR_UTILS_TARGET_PARSER_H
#define POLAR_UTILS_TARGET_PARSER_H

// FIXME: vector is used because that's what clang uses for subtarget feature
// lists, but SmallVector would probably be better
#include "polar/basic/adt/Triple.h"
#include <vector>

namespace polar {

// forward declare class with namespace
namespace basic {
class StringRef;
class Triple;
} // basic

using polar::basic::StringRef;
using polar::basic::Triple;

// Target specific information into their own namespaces. These should be
// generated from TableGen because the information is already there, and there
// is where new information about targets will be added.
// FIXME: To TableGen this we need to make some table generated files available
// even if the back-end is not compiled with LLVM, plus we need to create a new
// back-end to TableGen to create these clean tables.
namespace arm {

// FPU Version
enum class FPUVersion
{
   NONE,
   VFPV2,
   VFPV3,
   VFPV3_FP16,
   VFPV4,
   VFPV5
};

// An FPU name restricts the FPU in one of three ways:
enum class FPURestriction
{
   None = 0, ///< No restriction
   D16,      ///< Only 16 D registers
   SP_D16    ///< Only single-precision instructions, with 16 D registers
};

// An FPU name implies one of three levels of Neon support:
enum class NeonSupportLevel
{
   None = 0, ///< No Neon
   Neon,     ///< Neon
   Crypto    ///< Neon with Crypto
};

// FPU names.
enum FPUKind
{
#define ARM_FPU(NAME, KIND, VERSION, NEON_SUPPORT, RESTRICTION) KIND,
#include "polar/utils/ARMTargetParser.def"
   FK_LAST
};

// Arch names.
enum class ArchKind
{
#define ARM_ARCH(NAME, ID, CPU_ATTR, SUB_ARCH, ARCH_ATTR, ARCH_FPU, ARCH_BASE_EXT) ID,
#include "polar/utils/ARMTargetParser.def"
};

// Arch extension modifiers for CPUs.
enum ArchExtKind : unsigned
{
   AEK_INVALID =     0,
   AEK_NONE =        1,
   AEK_CRC =         1 << 1,
   AEK_CRYPTO =      1 << 2,
   AEK_FP =          1 << 3,
   AEK_HWDIVTHUMB =  1 << 4,
   AEK_HWDIVARM =    1 << 5,
   AEK_MP =          1 << 6,
   AEK_SIMD =        1 << 7,
   AEK_SEC =         1 << 8,
   AEK_VIRT =        1 << 9,
   AEK_DSP =         1 << 10,
   AEK_FP16 =        1 << 11,
   AEK_RAS =         1 << 12,
   AEK_SVE =         1 << 13,
   AEK_DOTPROD =     1 << 14,
   // Unsupported extensions.
   AEK_OS = 0x8000000,
   AEK_IWMMXT = 0x10000000,
   AEK_IWMMXT2 = 0x20000000,
   AEK_MAVERICK = 0x40000000,
   AEK_XSCALE = 0x80000000,
};

// ISA kinds.
enum class ISAKind
{
   INVALID = 0,
   ARM,
   THUMB,
   AARCH64
};

// Endianness
// FIXME: BE8 vs. BE32?
enum class EndianKind
{
   INVALID = 0,
   LITTLE,
   BIG
};

// v6/v7/v8 profile
enum class profileKind
{
   INVALID = 0, A, R, M
};

StringRef get_canonical_arch_name(StringRef arch);

// Information by ID
StringRef get_fpu_name(unsigned fpuKind);
FPUVersion get_fpu_version(unsigned fpuKind);
NeonSupportLevel get_fpu_neon_support_level(unsigned fpuKind);
FPURestriction get_fpu_restriction(unsigned fpuKind);

// FIXME: These should be moved to TargetTuple once it exists
bool get_fpu_features(unsigned fpuKind, std::vector<StringRef> &features);
bool get_hw_div_features(unsigned hwDivKind, std::vector<StringRef> &features);
bool get_extension_features(unsigned extensions,
                          std::vector<StringRef> &Features);

StringRef get_arch_name(ArchKind archKind);
unsigned get_arch_attr(ArchKind archKind);
StringRef get_cpu_attr(ArchKind archKind);
StringRef get_sub_arch(ArchKind archKind);
StringRef get_arch_ext_name(unsigned archExtKind);
StringRef get_arch_ext_feature(StringRef archExt);
StringRef get_hw_div_name(unsigned hwDivKind);

// Information by Name
unsigned  get_default_fpu(StringRef cpu, ArchKind archKind);
unsigned  get_default_extensions(StringRef cpu, ArchKind archKind);
StringRef get_default_cpu(StringRef arch);

// Parser
unsigned parse_hw_div(StringRef hwDiv);
unsigned parse_fpu(StringRef FPU);
ArchKind parse_arch(StringRef Arch);
unsigned parse_arch_ext(StringRef ArchExt);
ArchKind parse_cpu_arch(StringRef CPU);
ISAKind parse_arch_isa(StringRef Arch);
EndianKind parse_arch_endian(StringRef Arch);
profileKind parse_arch_profile(StringRef Arch);
unsigned parse_arch_version(StringRef Arch);

StringRef compute_default_target_abi(const Triple &TT, StringRef CPU);

} // namespace ARM

// FIXME:This should be made into class design,to avoid dupplication.
namespace aarch64 {

// Arch names.
enum class ArchKind
{
#define AARCH64_ARCH(NAME, ID, CPU_ATTR, SUB_ARCH, ARCH_ATTR, ARCH_FPU, ARCH_BASE_EXT) ID,
#include "AArch64TargetParser.def"
};

// Arch extension modifiers for CPUs.
enum ArchExtKind : unsigned
{
   AEK_INVALID =     0,
   AEK_NONE =        1,
   AEK_CRC =         1 << 1,
   AEK_CRYPTO =      1 << 2,
   AEK_FP =          1 << 3,
   AEK_SIMD =        1 << 4,
   AEK_FP16 =        1 << 5,
   AEK_PROFILE =     1 << 6,
   AEK_RAS =         1 << 7,
   AEK_LSE =         1 << 8,
   AEK_SVE =         1 << 9,
   AEK_DOTPROD =     1 << 10,
   AEK_RCPC =        1 << 11,
   AEK_RDM =         1 << 12
};

StringRef get_canonical_arch_name(StringRef Arch);

// Information by ID
StringRef get_fpu_name(unsigned FPUKind);
arm::FPUVersion get_fpu_version(unsigned FPUKind);
arm::NeonSupportLevel get_fpu_neon_support_level(unsigned FPUKind);
arm::FPURestriction get_fpu_restriction(unsigned FPUKind);

// FIXME: These should be moved to TargetTuple once it exists
bool get_fpu_features(unsigned FPUKind, std::vector<StringRef> &Features);
bool get_extension_features(unsigned Extensions,
                          std::vector<StringRef> &Features);
bool getArchFeatures(ArchKind AK, std::vector<StringRef> &Features);

StringRef get_arch_name(ArchKind AK);
unsigned get_arch_attr(ArchKind AK);
StringRef get_cpu_attr(ArchKind AK);
StringRef get_sub_arch(ArchKind AK);
StringRef get_arch_ext_name(unsigned ArchExtKind);
StringRef get_arch_ext_feature(StringRef ArchExt);
unsigned checkArchVersion(StringRef Arch);

// Information by Name
unsigned  get_default_fpu(StringRef CPU, ArchKind AK);
unsigned  get_default_extensions(StringRef CPU, ArchKind AK);
StringRef get_default_cpu(StringRef Arch);

// Parser
unsigned parse_fpu(StringRef FPU);
aarch64::ArchKind parse_arch(StringRef Arch);
unsigned parse_arch_ext(StringRef ArchExt);
ArchKind parse_cpu_arch(StringRef CPU);
arm::ISAKind parse_arch_isa(StringRef Arch);
arm::EndianKind parse_arch_endian(StringRef Arch);
arm::profileKind parse_arch_profile(StringRef Arch);
unsigned parse_arch_version(StringRef Arch);

} // namespace AArch64

namespace x86 {

// This should be kept in sync with libcc/compiler-rt as its included by clang
// as a proxy for what's in libgcc/compiler-rt.
enum ProcessorVendors : unsigned
{
   VENDOR_DUMMY,
#define X86_VENDOR(ENUM, STRING) \
   ENUM,
#include "polar/utils/X86TargetParser.def"
   VENDOR_OTHER
};

// This should be kept in sync with libcc/compiler-rt as its included by clang
// as a proxy for what's in libgcc/compiler-rt.
enum ProcessorTypes : unsigned
{
   CPU_TYPE_DUMMY,
#define X86_CPU_TYPE(ARCHNAME, ENUM) \
   ENUM,
#include "polar/utils/X86TargetParser.def"
   CPU_TYPE_MAX
};

// This should be kept in sync with libcc/compiler-rt as its included by clang
// as a proxy for what's in libgcc/compiler-rt.
enum ProcessorSubtypes : unsigned
{
   CPU_SUBTYPE_DUMMY,
#define X86_CPU_SUBTYPE(ARCHNAME, ENUM) \
   ENUM,
#include "polar/utils/X86TargetParser.def"
   CPU_SUBTYPE_MAX
};

// This should be kept in sync with libcc/compiler-rt as it should be used
// by clang as a proxy for what's in libgcc/compiler-rt.
enum ProcessorFeatures
{
#define X86_FEATURE(VAL, ENUM) \
   ENUM = VAL,
#include "polar/utils/X86TargetParser.def"

};

} // namespace X86
} // polar

#endif // POLAR_UTILS_TARGET_PARSER_H
