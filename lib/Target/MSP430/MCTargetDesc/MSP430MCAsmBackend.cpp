#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCObjectWriter.h"

#include "MSP430MCAsmBackend.h"
#include "MCTargetDesc/MSP430MCTargetDesc.h"

using namespace llvm;

namespace {
class MSP430MCAsmBackend : public MCAsmBackend {
	uint8_t OSABI;
public:
	MSP430MCAsmBackend(uint8_t osABI): OSABI(osABI) {}

	unsigned getNumFixupKinds() const override {
		return 0;
	}
	const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override
	{
		if (Kind >= FirstTargetFixupKind)
		{
			llvm_unreachable("llvm_unreachable getFixupKindInfo");
		}
		return MCAsmBackend::getFixupKindInfo(Kind);
	}
	void applyFixup(const MCFixup &Fixup, char *Data, unsigned DataSize,
		uint64_t Value, bool IsPCRel) const override {
		llvm_unreachable("llvm_unreachable applyFixup");
	}
	bool mayNeedRelaxation(const MCInst &Inst) const override {
		return false;
	}
	bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
		const MCRelaxableFragment *Fragment,
		const MCAsmLayout &Layout) const override {
		return false;
	}
	void relaxInstruction(const MCInst &Inst, MCInst &Res) const override {
		llvm_unreachable("llvm_unreachable relaxInstruction");
	}
	bool writeNopData(uint64_t Count, MCObjectWriter *OW) const override 
	{
		for (uint64_t I = 0; I != Count; ++I) {
			OW->write8(0);
		}
		return true;
	}
	MCObjectWriter *createObjectWriter(raw_pwrite_stream &OS) const override
	{
		return createMSP430ObjectWriter(OS, OSABI);
	}
};
}
MCAsmBackend *createMSP430MCAsmBackend(
	const Target &T,
	const MCRegisterInfo &MRI,
	const Triple &TT,
	StringRef CPU)
{
	uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(TT.getOS());
	return new MSP430MCAsmBackend(OSABI);
}