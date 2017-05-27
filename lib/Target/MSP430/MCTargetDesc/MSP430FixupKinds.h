//===-- SparcFixupKinds.h - Sparc Specific Fixup Entries --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MSP430_MCTARGETDESC_MSP430FIXUPKINDS_H
#define LLVM_LIB_TARGET_MSP430_MCTARGETDESC_MSP430FIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
    namespace MSP430 {
        enum Fixups {
                    fixup_msp430_jmp10 = FirstTargetFixupKind,
            // Marker
                    LastTargetFixupKind,
            NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
        };
    }
}

#endif
