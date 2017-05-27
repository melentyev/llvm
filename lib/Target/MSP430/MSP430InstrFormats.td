//===-- MSP430InstrFormats.td - MSP430 Instruction Formats -*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source 
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Describe MSP430 instructions format here
//

// Format specifies the encoding used by the instruction.  This is part of the
// ad-hoc solution used to emit machine instruction encodings by our machine
// code emitter.
class Format<bits<2> val> {
  bits<2> Value = val;
}

def PseudoFrm   : Format<0>;
def SingleOpFrm : Format<1>;
def DoubleOpFrm : Format<2>;
def CondJumpFrm : Format<3>;

class SourceMode<bits<2> val> {
  bits<2> Value = val;
}

def SrcReg      : SourceMode<0>;
def SrcMem      : SourceMode<1>;
def SrcIndReg   : SourceMode<2>;
def SrcPostInc  : SourceMode<3>;
def SrcImm      : SourceMode<3>;

class DestMode<bit val> {
  bit Value = val;
}

def DstReg      : DestMode<0>;
def DstMem      : DestMode<1>;

class SizeVal<bits<3> val> {
  bits<3> Value = val;
}

def SizeUnknown : SizeVal<0>; // Unknown / unset size
def SizeSpecial : SizeVal<1>; // Special instruction, e.g. pseudo
def Size2Bytes  : SizeVal<2>;
def Size4Bytes  : SizeVal<3>;
def Size6Bytes  : SizeVal<4>;

class IOPCODE<bits<4> val> { bits<4> Value = val; }
class IIOPCODE<bits<9> val> { bits<9> Value = val; }
class CJOPCODE<bits<3> val> { bits<3> Value = val; }

// Generic MSP430 Format
class MSP430Inst<dag outs, dag ins, SizeVal sz, Format f,
                 string asmstr> : Instruction {
  field bits<48> Inst;

  let Namespace = "MSP430";

  dag OutOperandList = outs;
  dag InOperandList  = ins;

  Format Form = f;
  SizeVal Sz = sz;

  // Define how we want to layout our TargetSpecific information field... This
  // should be kept up-to-date with the fields in the MSP430InstrInfo.h file.
  let TSFlags{1-0} = Form.Value;
  let TSFlags{4-2} = Sz.Value;

  let AsmString   = asmstr;

  let Size = !if(!eq(sz.Value, 2), 2, !if(!eq(sz.Value, 3), 4, 6));
}

// FIXME: Create different classes for different addressing modes.

// MSP430 Double Operand (Format I) Instructions
class IForm<bits<4> opcode, DestMode dest, bit bw, SourceMode src, SizeVal sz,
            dag outs, dag ins, string asmstr, list<dag> pattern>
  : MSP430Inst<outs, ins, sz, DoubleOpFrm, asmstr> {
  let Pattern = pattern;

  DestMode ad = dest;
  SourceMode as = src;
  
  let Inst{15-12} = opcode;
  let Inst{7}     = ad.Value;
  let Inst{6}     = bw;
  let Inst{5-4}   = as.Value;
}

// 8 bit IForm instructions
class IForm8<IOPCODE opcode, DestMode dest, SourceMode src, SizeVal sz,
             dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm<opcode.Value, dest, 1, src, sz, outs, ins, asmstr, pattern>;

class I8rr<IOPCODE opcode,
           dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm8<opcode, DstReg, SrcReg, Size2Bytes, outs, ins, asmstr, pattern>;

class I8ri<IOPCODE opcode,
           dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm8<opcode, DstReg, SrcImm, Size4Bytes, outs, ins, asmstr, pattern>;

class I8ri_WELL<IOPCODE opcode,
         dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm8<opcode, DstReg, SrcImm, Size4Bytes, outs, ins, asmstr, pattern> {
    bits<16> IMM; let Inst{31-16} = IMM;
    bits<4> dst; let Inst{3-0} = dst;
    let Inst{11-8} = 0;
}

class I8ri_CMP<IOPCODE opcode,
         dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm8<opcode, DstReg, SrcImm, Size4Bytes, outs, ins, asmstr, pattern> {
    bits<16> src2; let Inst{31-16} = src2;
    bits<4> src; let Inst{3-0} = src;
    let Inst{11-8} = 0;
}

class I8rm<IOPCODE opcode,
           dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm8<opcode, DstReg, SrcMem, Size4Bytes, outs, ins, asmstr, pattern>;

class I8mr<IOPCODE opcode,
           dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm8<opcode, DstMem, SrcReg, Size4Bytes, outs, ins, asmstr, pattern>;

class I8mi<IOPCODE opcode,
           dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm8<opcode, DstMem, SrcImm, Size6Bytes, outs, ins, asmstr, pattern>;

class I8mm<IOPCODE opcode,
           dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm8<opcode, DstMem, SrcMem, Size6Bytes, outs, ins, asmstr, pattern>;

// 16 bit IForm instructions
class IForm16<IOPCODE opcode, DestMode dest, SourceMode src, SizeVal sz,
              dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm<opcode.Value, dest, 0, src, sz, outs, ins, asmstr, pattern>;

class IForm16_WELL<IOPCODE opcode, DestMode dest, SourceMode src, SizeVal sz,
              dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm<opcode.Value, dest, 0, src, sz, outs, ins, asmstr, pattern>;

class I16rr<IOPCODE opcode,
            dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm16<opcode, DstReg, SrcReg, Size2Bytes, outs, ins, asmstr, pattern>;

class I16rr_WELL<IOPCODE opcode,
            dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm16<opcode, DstReg, SrcReg, Size2Bytes, outs, ins, asmstr, pattern> {
  bits<4> src2; let Inst{11-8} = src2;
  bits<4> dst; let Inst{3-0} = dst;
}

class I16ri<IOPCODE opcode,
            dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm16<opcode, DstReg, SrcImm, Size4Bytes, outs, ins, asmstr, pattern>;

class I16ri_WELL<IOPCODE opcode,
            dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm16<opcode, DstReg, SrcImm, Size4Bytes, outs, ins, asmstr, pattern> {
  bits<16> IMM; let Inst{31-16} = IMM;
  bits<4> dst; let Inst{3-0} = dst;
  let Inst{11-8} = 0;
}

class I16rm<IOPCODE opcode,
            dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm16<opcode, DstReg, SrcMem, Size4Bytes, outs, ins, asmstr, pattern> {
  //bits<4> dst; let Inst{3-0} = dst;
  //bits<4> MEMSRCREG; let Inst{11-8} = MEMSRCREG;
  //bits<16> MEMSRCIMM; let Inst{31-16} = MEMSRCIMM;
}

class I16mr<IOPCODE opcode,
            dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm16<opcode, DstMem, SrcReg, Size4Bytes, outs, ins, asmstr, pattern>;

class I16mr_WELL<IOPCODE opcode,
            dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm16<opcode, DstMem, SrcReg, Size4Bytes, outs, ins, asmstr, pattern> {
  bits<4> MEMDSTREG; let Inst{3-0} = MEMDSTREG;
  bits<16> MEMDSTIMM; let Inst{31-16} = MEMDSTIMM;
  bits<4> src; let Inst{11-8} = src;
}

class I16mi<IOPCODE opcode,
            dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm16<opcode, DstMem, SrcImm, Size6Bytes, outs, ins, asmstr, pattern>
{
  let Inst{11-8} = 0;
  bits<16> src; let Inst{31-16} = src;
  bits<4> MEMDSTREG; let Inst{3-0} = MEMDSTREG;
  bits<16> MEMDSTIMM; let Inst{47-32} = MEMDSTIMM;
}

class I16mi_CMP<IOPCODE opcode,
            dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm16<opcode, DstMem, SrcImm, Size6Bytes, outs, ins, asmstr, pattern>
{
  let Inst{11-8} = 0;
  bits<16> src2; let Inst{31-16} = src2;
  bits<4> MEMSRCREG; let Inst{3-0} = MEMSRCREG;
  bits<16> MEMSRCIMM; let Inst{47-32} = MEMSRCIMM;
}

class I16mm<IOPCODE opcode,
            dag outs, dag ins, string asmstr, list<dag> pattern>
  : IForm16<opcode, DstMem, SrcMem, Size6Bytes, outs, ins, asmstr, pattern> {
  bits<4> MEMDSTREG; let Inst{3-0} = MEMDSTREG;
  bits<16> MEMDSTIMM; let Inst{47-32} = MEMDSTIMM;
  bits<4> MEMSRCREG; let Inst{11-8} = MEMSRCREG;
  bits<16> MEMSRCIMM; let Inst{31-16} = MEMSRCIMM;
}

// MSP430 Single Operand (Format II) Instructions
class IIForm<bits<9> opcode, bit bw, SourceMode src, SizeVal sz,
             dag outs, dag ins, string asmstr, list<dag> pattern>
  : MSP430Inst<outs, ins, sz, SingleOpFrm, asmstr> {
  let Pattern = pattern;
  
  SourceMode as = src;

  let Inst{15-7} = opcode;
  let Inst{6}    = bw;
  let Inst{4-5}  = as.Value;
}

// 8 bit IIForm instructions
class IIForm8<bits<9> opcode, SourceMode src, SizeVal sz,
              dag outs, dag ins, string asmstr, list<dag> pattern>
  : IIForm<opcode, 1, src, sz, outs, ins, asmstr, pattern>;

class II8r<bits<9> opcode,
           dag outs, dag ins, string asmstr, list<dag> pattern>
  : IIForm8<opcode, SrcReg, Size2Bytes, outs, ins, asmstr, pattern>;

class II8m<bits<9> opcode,
           dag outs, dag ins, string asmstr, list<dag> pattern>
  : IIForm8<opcode, SrcMem, Size4Bytes, outs, ins, asmstr, pattern>;

class II8i<bits<9> opcode,
           dag outs, dag ins, string asmstr, list<dag> pattern>
  : IIForm8<opcode, SrcImm, Size4Bytes, outs, ins, asmstr, pattern>;

// 16 bit IIForm instructions
class IIForm16<bits<9> opcode, SourceMode src, SizeVal sz,
               dag outs, dag ins, string asmstr, list<dag> pattern>
  : IIForm<opcode, 0, src, sz, outs, ins, asmstr, pattern>;

class II16r<IIOPCODE opcode,
            dag outs, dag ins, string asmstr, list<dag> pattern>
  : IIForm16<opcode.Value, SrcReg, Size2Bytes, outs, ins, asmstr, pattern> {
  bits<4> dst;
  let Inst{3-0}  = dst;
}

class II16m<bits<9> opcode,
            dag outs, dag ins, string asmstr, list<dag> pattern>
  : IIForm16<opcode, SrcMem, Size4Bytes, outs, ins, asmstr, pattern>;

class II16i<IIOPCODE opcode,
            dag outs, dag ins, string asmstr, list<dag> pattern>
  : IIForm16<opcode.Value, SrcImm, Size4Bytes, outs, ins, asmstr, pattern> {
  bits<16> dst;
  let Inst{3-0} = 0;
  let Inst{31-16} = dst;
}

// MSP430 Conditional Jumps Instructions
class CJForm<dag outs, dag ins, string asmstr, list<dag> pattern>
  : MSP430Inst<outs, ins, Size2Bytes, CondJumpFrm, asmstr> {
  let Pattern = pattern;
  
  bits<10> dst;
  bits<3> cc;

  let Inst{15-13} = 0b001;
  let Inst{12-10} = cc;
  let Inst{9-0} = dst;
}

// Pseudo instructions
class Pseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
  : MSP430Inst<outs, ins, SizeSpecial, PseudoFrm, asmstr> {
  let Pattern = pattern;
  let Inst{15-0} = 0;
}
