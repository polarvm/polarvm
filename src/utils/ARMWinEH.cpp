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

#include "polar/utils/ARMWinEH.h"
#include "polar/utils/RawOutStream.h"

namespace polar {
namespace arm {
namespace wineu {

std::pair<uint16_t, uint32_t> saved_register_mask(const RuntimeFunction &runtimeFunc)
{
   uint8_t numRegisters = runtimeFunc.reg();
   uint8_t registersVFP = runtimeFunc.r();
   uint8_t linkRegister = runtimeFunc.l();
   uint8_t ChainedFrame = runtimeFunc.c();

   uint16_t gprMask = (ChainedFrame << 11) | (linkRegister << 14);
   uint32_t vfpMask = 0;

   if (registersVFP) {
      vfpMask |= (((1 << ((numRegisters + 1) % 8)) - 1) << 8);
   } else {
      gprMask |= (((1 << (numRegisters + 1)) - 1) << 4);
   }

   if (prologue_folding(runtimeFunc)) {
      gprMask |= (((1 << (numRegisters + 1)) - 1) << (~runtimeFunc.getStackAdjust() & 0x3));
   }
   return std::make_pair(gprMask, vfpMask);
}

} // wineu
} // arm
} // polar
