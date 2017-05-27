#ifndef _MSP430DC_HPP
#define _MSP430DC_HPP 1

#include "llvm/Support/TargetRegistry.h"
#include "llvm/CodeGen/CommandFlags.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Object/ELFObjectFile.h"

namespace Msp430DC {

using namespace llvm;

class Msp430DC {
	Triple TheTriple;
	LLVMContext &context;

	std::unique_ptr<Module> getModule(StringRef text);
	object::OwningBinary<object::ObjectFile> compileModule(Module *M);
	StringRef extractCode(object::OwningBinary<object::ObjectFile> &binary);
public:
	Msp430DC(LLVMContext &llvmctx) : TheTriple(Triple::normalize("msp430")), context(llvmctx) {}
	StringRef compileFunction(StringRef text) {
		auto M = getModule(text);
		auto obj = compileModule(M.get());
		auto code = extractCode(obj);
		return code;
	}
};

}


/*
class MSPExprAbstractBase {};

template<typename T>
class MSPExprBase {};

template<typename T>
struct MSPExpr {
MSPExprBase<T> *contents;
public:
MSPExpr(MSPExprBase<T> *val): contents(val) {}

MSPExpr(MSPExpr<T> &&src) { std::swap(contents, src.contents);  };
MSPExpr(const MSPExpr<T> &src) = delete;
};

template<typename T>
struct MSPExprVar: public MSPExprBase<T> {
int addr;
public:
MSPExprVar(int _addr) : addr(_addr) {}
};

template<typename T, int S>
struct MSPExprArray: public MSPExprBase<T[S]> {
int addr;
public:
MSPExprArray(int _addr): addr(_addr) {}
};

class MSPStmt {};

template<typename T>
class MSPStmtExpr: public MSPStmt {
MSPExpr<T> contents;
public:
MSPStmtExpr(MSPExpr<T> src): contents(src) {}
};

class MSPStmtGroup {
std::vector<MSPStmt*> stmts;
public:
template<typename T> MSPStmtGroup& X(MSPExpr<T> expr) {
return *this;
}
};

class MSP {
public:
template<typename T> static MSPExpr<T> Var(int addr = -1) {
return MSPExpr<T>(new MSPExprVar<T>(addr));
}
template<typename T, int S> static MSPExpr <T[S]> Array(int addr) {
return MSPExpr<T[S]>(new MSPExprArray<T, S>(addr));
}
template<typename T> static MSPStmtGroup X(MSPExpr<T> expr) {
return MSPStmtGroup();
}
};*/

#endif