#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCObjectWriter.h"

#include "MSP430MCAsmBackend.h"
#include "MCTargetDesc/MSP430MCTargetDesc.h"
#include "MCTargetDesc/MSP430FixupKinds.h"

using namespace llvm;

// TODO remove
#include <iostream>

namespace {
class MSP430MCAsmBackend : public MCAsmBackend {
	uint8_t OSABI;
public:
	MSP430MCAsmBackend(uint8_t osABI): OSABI(osABI) {}

	unsigned getNumFixupKinds() const override {
        return MSP430::NumTargetFixupKinds;
	}
	const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override
	{
        const static MCFixupKindInfo Infos[MSP430::NumTargetFixupKinds] = {
                // name                    offset bits  flags
                {"fixup_msp430_jmp10", 6, 10, MCFixupKindInfo::FKF_IsPCRel}
        };

        if (Kind < FirstTargetFixupKind) {
            return MCAsmBackend::getFixupKindInfo(Kind);
        }

        assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
                       "Invalid kind!");
        return Infos[Kind - FirstTargetFixupKind];
	}
	void applyFixup(const MCFixup &Fixup, char *Data, unsigned DataSize,
		uint64_t Value, bool IsPCRel) const override {

        if (Fixup.getKind() == MSP430::Fixups::fixup_msp430_jmp10) {
            // TODO Finish;
			unsigned Offset = Fixup.getOffset();
			unsigned InstOffset = (Value >> 1) - 1;
            Data[Offset] = uint8_t(InstOffset & 0xFF);
			Data[Offset + 1] |= uint8_t((InstOffset >> 8) & 0x3);
            std::cout << "applyFixup fixup_msp430_jmp10 DataSize " << DataSize << " Data " << Data << std::endl;
        }

		//llvm_unreachable("llvm_unreachable applyFixup");
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