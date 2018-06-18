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
// \brief AMDGPU metadata definitions and in-memory representations.
//===----------------------------------------------------------------------===//

#ifndef POLAR_UTILS_AMDGPUMETADATA_H
#define POLAR_UTILS_AMDGPUMETADATA_H

#include <cstdint>
#include <string>
#include <system_error>
#include <vector>

namespace polar {
namespace amdgpu {

//===----------------------------------------------------------------------===//
// HSA metadata.
//===----------------------------------------------------------------------===//
namespace hsamd {

/// \brief HSA metadata major version.
constexpr uint32_t VersionMajor = 1;
/// \brief HSA metadata minor version.
constexpr uint32_t VersionMinor = 0;

/// \brief HSA metadata beginning assembler directive.
constexpr char AssemblerDirectiveBegin[] = ".amd_amdgpu_hsa_metadata";
/// \brief HSA metadata ending assembler directive.
constexpr char AssemblerDirectiveEnd[] = ".end_amd_amdgpu_hsa_metadata";

/// \brief Access qualifiers.
enum class AccessQualifier : uint8_t {
   Default   = 0,
   ReadOnly  = 1,
   WriteOnly = 2,
   ReadWrite = 3,
   Unknown   = 0xff
};

/// \brief Address space qualifiers.
enum class AddressSpaceQualifier : uint8_t {
   Private  = 0,
   Global   = 1,
   Constant = 2,
   Local    = 3,
   Generic  = 4,
   Region   = 5,
   Unknown  = 0xff
};

/// \brief Value kinds.
enum class ValueKind : uint8_t
{
   ByValue                = 0,
   GlobalBuffer           = 1,
   DynamicSharedPointer   = 2,
   Sampler                = 3,
   Image                  = 4,
   Pipe                   = 5,
   Queue                  = 6,
   HiddenGlobalOffsetX    = 7,
   HiddenGlobalOffsetY    = 8,
   HiddenGlobalOffsetZ    = 9,
   HiddenNone             = 10,
   HiddenPrintfBuffer     = 11,
   HiddenDefaultQueue     = 12,
   HiddenCompletionAction = 13,
   Unknown                = 0xff
};

/// \brief Value types.
enum class ValueType : uint8_t
{
   Struct  = 0,
   I8      = 1,
   U8      = 2,
   I16     = 3,
   U16     = 4,
   F16     = 5,
   I32     = 6,
   U32     = 7,
   F32     = 8,
   I64     = 9,
   U64     = 10,
   F64     = 11,
   Unknown = 0xff
};

//===----------------------------------------------------------------------===//
// kernel Metadata.
//===----------------------------------------------------------------------===//
namespace kernel {

//===----------------------------------------------------------------------===//
// kernel Attributes Metadata.
//===----------------------------------------------------------------------===//
namespace attrs {

namespace key {
/// \brief key for kernel::Attr::Metadata::mReqdWorkGroupSize.
constexpr char ReqdWorkGroupSize[] = "ReqdWorkGroupSize";
/// \brief key for kernel::Attr::Metadata::mWorkGroupSizeHint.
constexpr char WorkGroupSizeHint[] = "WorkGroupSizeHint";
/// \brief key for kernel::Attr::Metadata::mVecTypeHint.
constexpr char VecTypeHint[] = "VecTypeHint";
/// \brief key for kernel::Attr::Metadata::mRuntimeHandle.
constexpr char RuntimeHandle[] = "RuntimeHandle";
} // end namespace key

/// \brief In-memory representation of kernel attributes metadata.
struct Metadata final
{
   /// \brief 'reqd_work_group_size' attribute. Optional.
   std::vector<uint32_t> mReqdWorkGroupSize = std::vector<uint32_t>();
   /// \brief 'work_group_size_hint' attribute. Optional.
   std::vector<uint32_t> mWorkGroupSizeHint = std::vector<uint32_t>();
   /// \brief 'vec_type_hint' attribute. Optional.
   std::string mVecTypeHint = std::string();
   /// \brief External symbol created by runtime to store the kernel address
   /// for enqueued blocks.
   std::string mRuntimeHandle = std::string();

   /// \brief Default constructor.
   Metadata() = default;

   /// \returns True if kernel attributes metadata is empty, false otherwise.
   bool empty() const
   {
      return !notEmpty();
   }

   /// \returns True if kernel attributes metadata is not empty, false otherwise.
   bool notEmpty() const
   {
      return !mReqdWorkGroupSize.empty() || !mWorkGroupSizeHint.empty() ||
            !mVecTypeHint.empty() || !mRuntimeHandle.empty();
   }
};

} // end namespace attrs

//===----------------------------------------------------------------------===//
// kernel Argument Metadata.
//===----------------------------------------------------------------------===//
namespace arg {

namespace key {
/// \brief key for kernel::Arg::Metadata::mName.
constexpr char Name[] = "Name";
/// \brief key for kernel::Arg::Metadata::mTypeName.
constexpr char TypeName[] = "TypeName";
/// \brief key for kernel::Arg::Metadata::mSize.
constexpr char Size[] = "Size";
/// \brief key for kernel::Arg::Metadata::mAlign.
constexpr char Align[] = "Align";
/// \brief key for kernel::Arg::Metadata::mValueKind.
constexpr char ValueKind[] = "ValueKind";
/// \brief key for kernel::Arg::Metadata::mValueType.
constexpr char ValueType[] = "ValueType";
/// \brief key for kernel::Arg::Metadata::mPointeeAlign.
constexpr char PointeeAlign[] = "PointeeAlign";
/// \brief key for kernel::Arg::Metadata::mAddrSpaceQual.
constexpr char AddrSpaceQual[] = "AddrSpaceQual";
/// \brief key for kernel::Arg::Metadata::mAccQual.
constexpr char AccQual[] = "AccQual";
/// \brief key for kernel::Arg::Metadata::mActualAccQual.
constexpr char ActualAccQual[] = "ActualAccQual";
/// \brief key for kernel::Arg::Metadata::mIsConst.
constexpr char IsConst[] = "IsConst";
/// \brief key for kernel::Arg::Metadata::mIsRestrict.
constexpr char IsRestrict[] = "IsRestrict";
/// \brief key for kernel::Arg::Metadata::mIsVolatile.
constexpr char IsVolatile[] = "IsVolatile";
/// \brief key for kernel::Arg::Metadata::mIsPipe.
constexpr char IsPipe[] = "IsPipe";
} // end namespace key

/// \brief In-memory representation of kernel argument metadata.
struct Metadata final
{
   /// \brief Name. Optional.
   std::string mName = std::string();
   /// \brief Type name. Optional.
   std::string mTypeName = std::string();
   /// \brief Size in bytes. Required.
   uint32_t mSize = 0;
   /// \brief Alignment in bytes. Required.
   uint32_t mAlign = 0;
   /// \brief Value kind. Required.
   ValueKind mValueKind = ValueKind::Unknown;
   /// \brief Value type. Required.
   ValueType mValueType = ValueType::Unknown;
   /// \brief Pointee alignment in bytes. Optional.
   uint32_t mPointeeAlign = 0;
   /// \brief Address space qualifier. Optional.
   AddressSpaceQualifier mAddrSpaceQual = AddressSpaceQualifier::Unknown;
   /// \brief Access qualifier. Optional.
   AccessQualifier mAccQual = AccessQualifier::Unknown;
   /// \brief Actual access qualifier. Optional.
   AccessQualifier mActualAccQual = AccessQualifier::Unknown;
   /// \brief True if 'const' qualifier is specified. Optional.
   bool mIsConst = false;
   /// \brief True if 'restrict' qualifier is specified. Optional.
   bool mIsRestrict = false;
   /// \brief True if 'volatile' qualifier is specified. Optional.
   bool mIsVolatile = false;
   /// \brief True if 'pipe' qualifier is specified. Optional.
   bool mIsPipe = false;

   /// \brief Default constructor.
   Metadata() = default;
};

} // end namespace Arg

//===----------------------------------------------------------------------===//
// kernel Code Properties Metadata.
//===----------------------------------------------------------------------===//
namespace codeprops {

namespace key {
/// \brief key for kernel::codeprops::Metadata::mKernargSegmentSize.
constexpr char KernargSegmentSize[] = "KernargSegmentSize";
/// \brief key for kernel::codeprops::Metadata::mGroupSegmentFixedSize.
constexpr char GroupSegmentFixedSize[] = "GroupSegmentFixedSize";
/// \brief key for kernel::codeprops::Metadata::mPrivateSegmentFixedSize.
constexpr char PrivateSegmentFixedSize[] = "PrivateSegmentFixedSize";
/// \brief key for kernel::codeprops::Metadata::mKernargSegmentAlign.
constexpr char KernargSegmentAlign[] = "KernargSegmentAlign";
/// \brief key for kernel::codeprops::Metadata::mWavefrontSize.
constexpr char WavefrontSize[] = "WavefrontSize";
/// \brief key for kernel::codeprops::Metadata::mNumSGPRs.
constexpr char NumSGPRs[] = "NumSGPRs";
/// \brief key for kernel::codeprops::Metadata::mNumVGPRs.
constexpr char NumVGPRs[] = "NumVGPRs";
/// \brief key for kernel::codeprops::Metadata::mMaxFlatWorkGroupSize.
constexpr char MaxFlatWorkGroupSize[] = "MaxFlatWorkGroupSize";
/// \brief key for kernel::codeprops::Metadata::mIsDynamicCallStack.
constexpr char IsDynamicCallStack[] = "IsDynamicCallStack";
/// \brief key for kernel::codeprops::Metadata::mIsXNACKEnabled.
constexpr char IsXNACKEnabled[] = "IsXNACKEnabled";
/// \brief key for kernel::codeprops::Metadata::mNumSpilledSGPRs.
constexpr char NumSpilledSGPRs[] = "NumSpilledSGPRs";
/// \brief key for kernel::codeprops::Metadata::mNumSpilledVGPRs.
constexpr char NumSpilledVGPRs[] = "NumSpilledVGPRs";
} // end namespace key

/// \brief In-memory representation of kernel code properties metadata.
struct Metadata final
{
   /// \brief Size in bytes of the kernarg segment memory. Kernarg segment memory
   /// holds the values of the arguments to the kernel. Required.
   uint64_t mKernargSegmentSize = 0;
   /// \brief Size in bytes of the group segment memory required by a workgroup.
   /// This value does not include any dynamically allocated group segment memory
   /// that may be added when the kernel is dispatched. Required.
   uint32_t mGroupSegmentFixedSize = 0;
   /// \brief Size in bytes of the private segment memory required by a workitem.
   /// Private segment memory includes arg, spill and private segments. Required.
   uint32_t mPrivateSegmentFixedSize = 0;
   /// \brief Maximum byte alignment of variables used by the kernel in the
   /// kernarg memory segment. Required.
   uint32_t mKernargSegmentAlign = 0;
   /// \brief Wavefront size. Required.
   uint32_t mWavefrontSize = 0;
   /// \brief Total number of SGPRs used by a wavefront. Optional.
   uint16_t mNumSGPRs = 0;
   /// \brief Total number of VGPRs used by a workitem. Optional.
   uint16_t mNumVGPRs = 0;
   /// \brief Maximum flat work-group size supported by the kernel. Optional.
   uint32_t mMaxFlatWorkGroupSize = 0;
   /// \brief True if the generated machine code is using a dynamically sized
   /// call stack. Optional.
   bool mIsDynamicCallStack = false;
   /// \brief True if the generated machine code is capable of supporting XNACK.
   /// Optional.
   bool mIsXNACKEnabled = false;
   /// \brief Number of SGPRs spilled by a wavefront. Optional.
   uint16_t mNumSpilledSGPRs = 0;
   /// \brief Number of VGPRs spilled by a workitem. Optional.
   uint16_t mNumSpilledVGPRs = 0;

   /// \brief Default constructor.
   Metadata() = default;

   /// \returns True if kernel code properties metadata is empty, false
   /// otherwise.
   bool empty() const {
      return !notEmpty();
   }

   /// \returns True if kernel code properties metadata is not empty, false
   /// otherwise.
   bool notEmpty() const {
      return true;
   }
};

} // end namespace codeprops

//===----------------------------------------------------------------------===//
// kernel Debug Properties Metadata.
//===----------------------------------------------------------------------===//
namespace debugprops
{

namespace key {
/// \brief key for kernel::debugprops::Metadata::mDebuggerABIVersion.
constexpr char DebuggerABIVersion[] = "DebuggerABIVersion";
/// \brief key for kernel::debugprops::Metadata::mReservedNumVGPRs.
constexpr char ReservedNumVGPRs[] = "ReservedNumVGPRs";
/// \brief key for kernel::debugprops::Metadata::mReservedFirstVGPR.
constexpr char ReservedFirstVGPR[] = "ReservedFirstVGPR";
/// \brief key for kernel::debugprops::Metadata::mPrivateSegmentBufferSGPR.
constexpr char PrivateSegmentBufferSGPR[] = "PrivateSegmentBufferSGPR";
/// \brief key for
///     kernel::debugprops::Metadata::mWavefrontPrivateSegmentOffsetSGPR.
constexpr char WavefrontPrivateSegmentOffsetSGPR[] =
      "WavefrontPrivateSegmentOffsetSGPR";
} // end namespace key

/// \brief In-memory representation of kernel debug properties metadata.
struct Metadata final
{
   /// \brief Debugger ABI version. Optional.
   std::vector<uint32_t> mDebuggerABIVersion = std::vector<uint32_t>();
   /// \brief Consecutive number of VGPRs reserved for debugger use. Must be 0 if
   /// mDebuggerABIVersion is not set. Optional.
   uint16_t mReservedNumVGPRs = 0;
   /// \brief First fixed VGPR reserved. Must be uint16_t(-1) if
   /// mDebuggerABIVersion is not set or mReservedFirstVGPR is 0. Optional.
   uint16_t mReservedFirstVGPR = uint16_t(-1);
   /// \brief Fixed SGPR of the first of 4 SGPRs used to hold the scratch V# used
   /// for the entire kernel execution. Must be uint16_t(-1) if
   /// mDebuggerABIVersion is not set or SGPR not used or not known. Optional.
   uint16_t mPrivateSegmentBufferSGPR = uint16_t(-1);
   /// \brief Fixed SGPR used to hold the wave scratch offset for the entire
   /// kernel execution. Must be uint16_t(-1) if mDebuggerABIVersion is not set
   /// or SGPR is not used or not known. Optional.
   uint16_t mWavefrontPrivateSegmentOffsetSGPR = uint16_t(-1);

   /// \brief Default constructor.
   Metadata() = default;

   /// \returns True if kernel debug properties metadata is empty, false
   /// otherwise.
   bool empty() const {
      return !notEmpty();
   }

   /// \returns True if kernel debug properties metadata is not empty, false
   /// otherwise.
   bool notEmpty() const {
      return !mDebuggerABIVersion.empty();
   }
};

} // end namespace debugprops

namespace key {
/// \brief key for kernel::Metadata::mName.
constexpr char Name[] = "Name";
/// \brief key for kernel::Metadata::mSymbolName.
constexpr char SymbolName[] = "SymbolName";
/// \brief key for kernel::Metadata::mLanguage.
constexpr char Language[] = "Language";
/// \brief key for kernel::Metadata::mLanguageVersion.
constexpr char LanguageVersion[] = "LanguageVersion";
/// \brief key for kernel::Metadata::mattrs.
constexpr char attrs[] = "attrs";
/// \brief key for kernel::Metadata::mArgs.
constexpr char Args[] = "Args";
/// \brief key for kernel::Metadata::mcodeprops.
constexpr char codeprops[] = "codeprops";
/// \brief key for kernel::Metadata::mdebugprops.
constexpr char debugprops[] = "debugprops";
} // end namespace key

/// \brief In-memory representation of kernel metadata.
struct Metadata final
{
   /// \brief kernel source name. Required.
   std::string mName = std::string();
   /// \brief kernel descriptor name. Required.
   std::string mSymbolName = std::string();
   /// \brief Language. Optional.
   std::string mLanguage = std::string();
   /// \brief Language version. Optional.
   std::vector<uint32_t> mLanguageVersion = std::vector<uint32_t>();
   /// \brief Attributes metadata. Optional.
   attrs::Metadata mattrs = attrs::Metadata();
   /// \brief Arguments metadata. Optional.
   std::vector<arg::Metadata> mArgs = std::vector<Arg::Metadata>();
   /// \brief Code properties metadata. Optional.
   codeprops::Metadata mcodeprops = codeprops::Metadata();
   /// \brief Debug properties metadata. Optional.
   debugprops::Metadata mdebugprops = debugprops::Metadata();

   /// \brief Default constructor.
   Metadata() = default;
};

} // end namespace kernel

namespace key {
/// \brief key for HSA::Metadata::mVersion.
constexpr char Version[] = "Version";
/// \brief key for HSA::Metadata::mPrintf.
constexpr char Printf[] = "Printf";
/// \brief key for HSA::Metadata::mkernels.
constexpr char kernels[] = "kernels";
} // end namespace key

/// \brief In-memory representation of HSA metadata.
struct Metadata final
{
   /// \brief HSA metadata version. Required.
   std::vector<uint32_t> mVersion = std::vector<uint32_t>();
   /// \brief Printf metadata. Optional.
   std::vector<std::string> mPrintf = std::vector<std::string>();
   /// \brief kernels metadata. Required.
   std::vector<kernel::Metadata> mkernels = std::vector<kernel::Metadata>();

   /// \brief Default constructor.
   Metadata() = default;
};

/// \brief Converts \p String to \p HSAMetadata.
std::error_code from_string(std::string string, Metadata &hsaMetadata);

/// \brief Converts \p HSAMetadata to \p String.
std::error_code to_string(Metadata hsaMetadata, std::string &string);

} // end namespace hsamd

//===----------------------------------------------------------------------===//
// PAL metadata.
//===----------------------------------------------------------------------===//
namespace palmd {

/// \brief PAL metadata assembler directive.
constexpr char AssemblerDirective[] = ".amd_amdgpu_pal_metadata";

/// \brief PAL metadata keys.
enum key : uint32_t
{
   LS_NUM_USED_VGPRS = 0x10000021,
   HS_NUM_USED_VGPRS = 0x10000022,
   ES_NUM_USED_VGPRS = 0x10000023,
   GS_NUM_USED_VGPRS = 0x10000024,
   VS_NUM_USED_VGPRS = 0x10000025,
   PS_NUM_USED_VGPRS = 0x10000026,
   CS_NUM_USED_VGPRS = 0x10000027,

   LS_NUM_USED_SGPRS = 0x10000028,
   HS_NUM_USED_SGPRS = 0x10000029,
   ES_NUM_USED_SGPRS = 0x1000002a,
   GS_NUM_USED_SGPRS = 0x1000002b,
   VS_NUM_USED_SGPRS = 0x1000002c,
   PS_NUM_USED_SGPRS = 0x1000002d,
   CS_NUM_USED_SGPRS = 0x1000002e,

   LS_SCRATCH_SIZE = 0x10000044,
   HS_SCRATCH_SIZE = 0x10000045,
   ES_SCRATCH_SIZE = 0x10000046,
   GS_SCRATCH_SIZE = 0x10000047,
   VS_SCRATCH_SIZE = 0x10000048,
   PS_SCRATCH_SIZE = 0x10000049,
   CS_SCRATCH_SIZE = 0x1000004a
};

/// \brief PAL metadata represented as a vector.
typedef std::vector<uint32_t> Metadata;

/// \brief Converts \p PALMetadata to \p String.
std::error_code to_string(const Metadata &palMetadata, std::string &string);

} // end namespace palmd
} // end namespace amdgpu
} // polar

#endif // POLAR_UTILS_AMDGPUMETADATA_H
