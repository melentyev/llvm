#include "MSP430MCTargetDesc.h"
#include "MSP430MCAsmInfo.h"
#include "MSP430FixupKinds.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"

#include <llvm/Support/Debug.h> 

#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>
#include <string>

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"
#define GMOV(idx) getMachineOpValue(MI, MI.getOperand(idx), Fixups, STI)

extern std::map<std::string, int> _intrinsics;

class MSP430MCCodeEmitter : public MCCodeEmitter {
	const MCInstrInfo &MCII;
	MCContext &Ctx;
	mutable FILE *txtOut;
public:
	MSP430MCCodeEmitter(const MCInstrInfo &mcii, MCContext &Ctx_)
		: MCII(mcii), Ctx(Ctx_)
	{
	}

	~MSP430MCCodeEmitter() override {}

	void encodeInstruction(const MCInst &MI, raw_ostream &OS,
		SmallVectorImpl<MCFixup> &Fixups,
		const MCSubtargetInfo &STI) const override
	{

		//MI.dump();

		uint64_t Bits = getBinaryCodeForInstrFixer(MI, Fixups, STI);
		unsigned Size = MCII.get(MI.getOpcode()).getSize();

		for (unsigned i = 0; i != Size; ++i) {
			uint8_t byte = (Bits >> (i * 8)) & 0xFF;
			OS << byte;
		}
	}
    // TODO dirtiest hack!!
    uint64_t getBinaryCodeForInstrFixer(const MCInst &MI,
                                        SmallVectorImpl<MCFixup> &Fixups,
                                        const MCSubtargetInfo &STI) const
    {
        uint64_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
        switch (MI.getOpcode()) {
            /*case MSP430::ADC16mi:
            case MSP430::ADD16mi:
            case MSP430::AND16mi:
            case MSP430::BIC16rm:
            case MSP430::BIT16rm:
            case MSP430::CMP16rm:
            case MSP430::MOV16mi:
            case MSP430::OR16mi:
            case MSP430::SBC16mi:
            case MSP430::SUB16mi:
            case MSP430::XOR16mi:
			case MSP430::CMP16mi:
                Bits = (Bits & 0xFFFF) | ((GMOV(2) & UINT64_C(0xFFFF)) << 16) | ((GMOV(1) & UINT64_C(0xFFFF)) << 32);
                break;*/
			/*case MSP430::MOV16ri: {
				uint64_t op = getMachineOpValue(MI, MI.getOperand(1), Fixups, STI);
				Bits = (Bits & 0xFFFF) | ((op & UINT64_C(65535)) << 16);
				// op: dst
				op = getMachineOpValue(MI, MI.getOperand(0), Fixups, STI);
				Bits |= op & UINT64_C(15);
				break;
			}*/


            case MSP430::MOV16rm: case MSP430::MOV8rm:
            case MSP430::CMP16rm: case MSP430::CMP8rm:
                Bits = (Bits & 0xFFFF) |
                       ((GMOV(0) & UINT64_C(15) )) |         // dst reg
                       ((GMOV(1) & UINT64_C(15)) << 8) |     // src reg
                       ((GMOV(2) & UINT64_C(0xFFFF)) << 16); // src imm
                break;
            case MSP430::ADC16rm: case MSP430::ADC8rm:
            case MSP430::ADD16rm: case MSP430::ADD8rm:
            case MSP430::AND16rm: case MSP430::AND8rm:
            case MSP430::BIC16rm: case MSP430::BIC8rm:
            case MSP430::BIT16rm: case MSP430::BIT8rm:
            case MSP430::OR16rm:  case MSP430::OR8rm:
            case MSP430::SBC16rm: case MSP430::SBC8rm:
            case MSP430::SUB16rm: case MSP430::SUB8rm:
            case MSP430::XOR16rm: case MSP430::XOR8rm:
                Bits = (Bits & 0xFFFF) |
                       ((GMOV(0) & UINT64_C(15) )) |         // dst reg
                       ((GMOV(2) & UINT64_C(15)) << 8) |     // src reg
                       ((GMOV(3) & UINT64_C(0xFFFF)) << 16); // src imm
                break;
        }
        switch (MI.getOpcode()) {
            case MSP430::CALLi:
            case MSP430::CALLr:
            case MSP430::CALLm:
                auto op0 = MI.getOperand(0);
                if (op0.isExpr()) {
                    const MCExpr *expr = op0.getExpr();
					if (isa<MCSymbolRefExpr>(expr)) {
						const MCSymbolRefExpr *symref = cast<MCSymbolRefExpr>(expr);
						if (symref->getSymbol().getName() == "__mulhi3hw_noint") {
							// TODO patching instruction
							std::cout << "MULT OR DIV NOT IMPLEMENTED!" << std::endl;
						}
					}
                }
                break;
        }
        return Bits;
    }

	uint64_t getBinaryCodeForInstr(const MCInst &MI,
		SmallVectorImpl<MCFixup> &Fixups,
		const MCSubtargetInfo &STI) const;

	uint64_t getMachineOpValue(const MCInst &MI, const MCOperand &MO,
		SmallVectorImpl<MCFixup> &Fixups,
		const MCSubtargetInfo &STI) const;
};

uint64_t MSP430MCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
	SmallVectorImpl<MCFixup> &Fixups,
	const MCSubtargetInfo &STI) const
{
	if (MO.isReg()) {
        if (MO.getReg() == MSP430::NoRegister) {
            return 2;
        }
		return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());
    }
	if (MO.isImm())
		return static_cast<uint64_t>(MO.getImm());
	if (MO.isExpr()) {
		const MCExpr *expr = MO.getExpr();
		if (isa<MCSymbolRefExpr>(expr)) {
			const MCSymbolRefExpr *symref = cast<MCSymbolRefExpr>(expr);
			StringRef name = symref->getSymbol().getName();
			if (StringSwitch<bool>(name)
					.Case("__mulhi3hw_noint", false)
					.Case("__mulsi3", false)
					.Case("__divsi3", false)
					.Default(true))
			{
            	Fixups.push_back(MCFixup::create(0, symref, (MCFixupKind)MSP430::Fixups::fixup_msp430_jmp10));
			}
			return 0;
		}
	}
	llvm_unreachable("Unexpected operand type!");
}


llvm::MCCodeEmitter *createMSP430MCCodeEmitter(const MCInstrInfo &mcii, const MCRegisterInfo &MRI, MCContext &ctx) {
	return new MSP430MCCodeEmitter(mcii, ctx);
}

#include "MSP430GenMCCodeEmitter.inc"
