//===-- SystemZMCObjectWriter.cpp - SystemZ ELF writer --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/MSP430MCTargetDesc.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCValue.h"

using namespace llvm;

namespace {
	class MSP430MCObjectWriter : public MCELFObjectTargetWriter {
	public:
		MSP430MCObjectWriter(uint8_t OSABI)
			: MCELFObjectTargetWriter(/*Is64Bit=*/true, OSABI, ELF::EM_S390,
				/*HasRelocationAddend=*/ true) {}

		~MSP430MCObjectWriter() override {}

	protected:
		unsigned GetRelocType(const MCValue &Target, const MCFixup &Fixup, bool IsPCRel) const override 
		{
			llvm_unreachable("MSP430MCObjectWriter::getRelocType");
			return 0;
		}
	};
}

MCObjectWriter *llvm::createMSP430ObjectWriter(raw_pwrite_stream &OS, uint8_t OSABI) {
	MCELFObjectTargetWriter *MOTW = new MSP430MCObjectWriter(OSABI);
	return createELFObjectWriter(MOTW, OS, /*IsLittleEndian=*/false);
}
