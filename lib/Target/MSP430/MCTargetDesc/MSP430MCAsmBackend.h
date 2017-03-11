#pragma once

#include "llvm/Support/TargetRegistry.h"
#include "llvm/MC/MCAsmBackend.h"

using namespace llvm;

MCAsmBackend *createMSP430MCAsmBackend(
	const Target &T,
	const MCRegisterInfo &MRI,
	const Triple &TT,
	StringRef CPU);

