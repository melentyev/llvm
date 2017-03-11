#pragma once
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCInstrInfo.h"

using namespace llvm;

MCCodeEmitter *createMSP430MCCodeEmitter(const MCInstrInfo &mcii, const MCRegisterInfo &MRI, MCContext &ctx);