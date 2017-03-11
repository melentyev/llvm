#include "MSP430MCTargetDesc.h"
#include "MSP430MCAsmInfo.h"
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
#include <cstdio>

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

class MSP430MCCodeEmitter : public MCCodeEmitter {
	const MCInstrInfo &MCII;
	MCContext &Ctx;
	mutable FILE *txtOut;
public:
	MSP430MCCodeEmitter(const MCInstrInfo &mcii, MCContext &Ctx_)
		: MCII(mcii), Ctx(Ctx_)
	{
		txtOut = fopen("C:\\Users\\user\\Documents\\clangtest\\test2.txt", "w");
		fclose(txtOut);
	}

	~MSP430MCCodeEmitter() override {}

	void encodeInstruction(const MCInst &MI, raw_ostream &OS,
		SmallVectorImpl<MCFixup> &Fixups,
		const MCSubtargetInfo &STI) const override
	{
		MI.print(dbgs());
		MI.dump();
		
		uint64_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
		unsigned Size = MCII.get(MI.getOpcode()).getSize();

		for (unsigned i = 0; i != Size; ++i) {
			uint8_t byte = (Bits >> (i * 8)) & 0xFF;
			OS << byte;
		}

		txtOut = fopen("C:\\Users\\user\\Documents\\clangtest\\test2.txt", "a");
		for (int j = 0; j < Size; j += 2) {
			for (int i = j; i < j + 2; i++) {
				uint8_t byte = (Bits >> (i * 8)) & 0xFF;
				fprintf(txtOut, "%02X", (uint32_t)byte);
			}
			fprintf(txtOut, " ");
		}
		fprintf(txtOut, "\n");
		fclose(txtOut);

		std::cerr << "here\n";
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
	if (MO.isReg())
		return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());
	if (MO.isImm())
		return static_cast<uint64_t>(MO.getImm());
	if (MO.isExpr()) {
		const MCExpr *expr = MO.getExpr();
		if (isa<MCSymbolRefExpr>(expr)) {
			const MCSymbolRefExpr *refsym = cast<MCSymbolRefExpr>(expr);
			return 0;
		}
	}
	llvm_unreachable("Unexpected operand type!");
}


llvm::MCCodeEmitter *createMSP430MCCodeEmitter(const MCInstrInfo &mcii, const MCRegisterInfo &MRI, MCContext &ctx) {
	return new MSP430MCCodeEmitter(mcii, ctx);
}

#include "MSP430GenMCCodeEmitter.inc"
