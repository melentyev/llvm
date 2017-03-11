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

#include <string>
#include <vector>
#include <unordered_set>
#include <functional>

struct xy {
	int x;
	int y;
};

const uint16_t MSPADDR_ADC10CTL0 = 0x0740;
const uint16_t MSPADDR_ADC10CTL1 = 0x0742;
const uint16_t MSPADDR_ADC10MCTL0 = 0x074A;

#define ADC10ENC               (0x0002)       /* ADC10 Enable Conversion */

namespace Msp430DC {

using namespace llvm;

class IRDumper {
	std::string text;
	unsigned counter;
public:
	IRDumper() : counter(0) {}
	void append(const char *line) { text += line; }
	std::string nextName() { return "x" + std::to_string(counter++); }
};

class Msp430Base {
	virtual void toIR(IRDumper &dumper) = 0;
};

class Msp430AbstractExprBase {};
typedef std::unordered_set<Msp430AbstractExprBase*> RootsDfsItems;

template<typename T>
class Msp430ExprBase: public Msp430AbstractExprBase {
public:
	virtual void rootsDfs(RootsDfsItems &items) = 0;
};

class Msp430Context {
	typedef std::vector<Msp430Base*>  StmtsType;
	std::unique_ptr<StmtsType> stmts;
	static std::vector<Msp430Context> stack;
public:
	Msp430Context() {}
	static Msp430Context& Current() { return stack.back();  }
	void Push(Msp430Base *val) { stmts->push_back(val); }
	StmtsType& Stmts() { return *stmts; }
	std::unique_ptr<StmtsType> ExtractAllStmts() {
		return std::move(stmts);
	}
	class Local {
		Msp430Context *ctx;
	public:
		Local() {
			stack.push_back(Msp430Context());
			ctx = &stack.back();
		}
		Msp430Context& GetContext() { return *ctx; }
		~Local() { stack.pop_back(); }
	};
};


#define OPER(oper, func) Msp430<T> operator oper(const Msp430<T> &rhs) { \
Msp430<T> expr = Msp430<T>(Msp430Binary<T, T, T>::func(*this, rhs)); \
Msp430Context::Current().Push(&expr); \
return expr; \
}
#define OPER_ASSIGN_WITH(oper, func) OPER(oper, func)

#define OPER_CMP(oper, func) Msp430<bool> operator oper(const Msp430<T> &rhs) { \
Msp430<bool> expr = Msp430<bool>(Msp430Binary<bool, T, T>::func(*this, rhs)); \
Msp430Context::Current().Push(&expr); \
return expr; \
}

#define OPER_INCR_DECR(oper, args, incdec, post) Msp430<T> operator oper(args) { \
Msp430<T> expr = Msp430<T>(new Msp430IncrDecr<T, incdec, post>(*this)); \
Msp430Context::Current().Push(&expr); \
return expr; \
}

template<typename T>
class Msp430B: public Msp430Base {
protected:
	Msp430ExprBase<T> *contents;
public:
	Msp430B(Msp430ExprBase<T> *expr) : contents(expr) {}

	void toIR(IRDumper &dumper) override {
		/*if (opType == OP_ADD) {
		//"%" + dumper.nextName() + " = add nsw i16 % 3, " + rhs;
		}
		if (opType == OP_ASSIGN) {
		//"store i16 %" + source + ", i16* %" + destAddr + ", align 2"
		}
		% 3 = load i16, i16* %y, align 2
		% 4 = load i16, i16* %z, align 2
		% add = add nsw i16 % 3, % 4*/
	}
};

template<typename T>
class Msp430 : public Msp430B<T> {
public:
	Msp430(Msp430AbstractExprBase* expr): Msp430B((Msp430ExprBase<T> *)expr) {}
	static Msp430<T> Var(int addr) {
		return Msp430<T>(new Msp430Var<T>(addr)); 
	}
	static Msp430<T> Const(T x) {
		return Msp430<T>(new Msp430Const<T>(x));
	}
	/*template<int S> static Msp430<T[S]> Array(int addr) {
		typedef T arrT[S];
		auto arr = new Msp430Array<T, S>(addr);
		Msp430ExprBase<arrT>* cast = (Msp430ExprBase<arrT>*)arr;
		Msp430<arrT> res = Msp430<arrT>(cast);
		return res;
	}*/
	//Msp430(): contents(nullptr) {}
	Msp430(const Msp430<T> &val): Msp430B(val.contents) {}
	Msp430(T val): Msp430B(new Msp430Const<T>(val)) {}
	Msp430<T> Assign(const Msp430<T> &rhs) {
		Msp430<T> expr = Msp430<T>(Msp430Binary<T, T, T>::Assign(*this, rhs));
		Msp430Context::Current().Push(&expr);
		return expr;
	}
	Msp430<T> Zero() {
		Msp430<T> expr = Msp430<T>(Msp430Binary<T, T, T>::Assign(*this, Msp430<T>(new Msp430Const<T>(0))));
		Msp430Context::Current().Push(&expr);
		return expr;
	}
	Msp430<T>& operator=(const Msp430<T> &rhs) = delete;
	
	OPER_ASSIGN_WITH(&=, AssignBitAnd)
	OPER_ASSIGN_WITH(|=, AssignBitAnd)
	OPER_ASSIGN_WITH(+=, AssignPlus)
	OPER_ASSIGN_WITH(-=, AssignMinus)

	OPER(+, Plus)
	OPER(-, Minus)

	OPER_CMP(== , Equal)
	OPER_CMP(!= , NotEqual)
	OPER_CMP(< , Less)
	OPER_CMP(> , Greater)

	OPER_INCR_DECR(++, int, true, true)
	OPER_INCR_DECR(++, , true, false)
	OPER_INCR_DECR(--, int, false, true)
	OPER_INCR_DECR(--, , false, false)

	void rootsDfs(RootsDfsItems &items) override { contents->rootsDfs(items); }
	Msp430AbstractExprBase* Root() {
		RootsDfsItems items;
		this->rootsDfs(items);
	}
	StringRef toIR() {
		IRDumper dumper;
		toIR(dumper);
	}
};
#undef OPER
#undef OPER_ASSIGN_WITH
#undef OPER_CMP

template<typename T> Msp430<T*> Msp430MakeArray(int addr) {
	return Msp430<T*>((Msp430ExprBase<T*>*)(new Msp430Var<T*>(addr)));
}

template<typename T> static Msp430<T> Msp430MakeLocal() {
	return Msp430<T>(static_cast<Msp430AbstractExprBase*>(new Msp430Var<T>()));
}

template<typename T, int S>
class Msp430<T[S]> {
public:
	template<typename I>
	Msp430<T> operator[](const Msp430<I> &idx) {
		Msp430<T> expr = Msp430<T>(Msp430Binary<T, T[S], I>::AccessArrayItem(*this, idx));
		Msp430Context::Current().Push(&expr);
		return expr;
	}
};

template<typename T> class Msp430<T*>: public Msp430B<T*> {
public:
	Msp430(Msp430AbstractExprBase* expr) : Msp430B((Msp430ExprBase<T*>*)expr) {}
	template<typename I>
	Msp430<T> operator[](const Msp430<I> &idx) {
		Msp430<T> expr = Msp430<T>(Msp430Binary<T, T*, I>::AccessArrayItem(*this, idx));
		Msp430Context::Current().Push(&expr);
		return expr;
	}
};

template<typename T>
class Msp430Var : public Msp430ExprBase<T> {
	int addr;
public:
	Msp430Var(int _addr = -1): addr(_addr) {}
	void rootsDfs(RootsDfsItems &items) override {}
};

template<typename T, int S>
class Msp430Array : public Msp430ExprBase<T[S]> {
	int addr;
public:
	Msp430Array(int _addr) : addr(_addr) {}
	void rootsDfs(RootsDfsItems &items) override {}
};

template<typename T>
class Msp430Const : public Msp430ExprBase<T> {
	T value;
public:
	Msp430Const(T _value) : value(_value) {}
	void rootsDfs(RootsDfsItems &items) override {}
};

template<typename T, bool Incr, bool Post>
class Msp430IncrDecr : public Msp430ExprBase<T> {
	Msp430<T> src;
public:
	Msp430IncrDecr(Msp430<T> _src) : src(_src) {}
	void rootsDfs(RootsDfsItems &items) override { src.rootsDfs(items); }
};

#define OPER(name, enummem) static Msp430Binary* name(const Msp430<A> &_lhs, const Msp430<B> &_rhs) { \
        return new Msp430Binary(enummem, _lhs, _rhs); \
    }

template<typename T, typename A, typename B>
class Msp430Binary : public Msp430ExprBase<T> {
	Msp430<A> lhs;
	Msp430<B> rhs;
	enum ExprType {
		OP_ASSIGN,
		OP_ASSIGN_BITAND,
		OP_ASSIGN_BITOR,
		OP_ASSIGN_PLUS,
		OP_ASSIGN_MINUS,
		OP_PLUS,
		OP_MINUS,
		OP_MULT,
		OP_EQ, OP_NE, OP_LT, OP_GT,
		OP_ACCESS_ARRAY_ITEM,
	} et;
	Msp430Binary(ExprType _et, const Msp430<A> &_lhs, const Msp430<B> &_rhs) 
		: lhs(_lhs), rhs(_rhs), et(_et)
	{}
public:
	OPER(Assign, OP_ASSIGN)
	OPER(AssignBitAnd, OP_ASSIGN_BITAND)
	OPER(AssignBitOr, OP_ASSIGN_BITOR)
	OPER(AssignPlus, OP_ASSIGN_PLUS)
	OPER(AssignMinus, OP_ASSIGN_MINUS)

	OPER(Plus, OP_PLUS)
	OPER(Minus, OP_MINUS)
	OPER(Mult, OP_MULT)

	OPER(Equal, OP_EQ)
	OPER(NotEqual, OP_NE)
	OPER(Less, OP_LT)
	OPER(Greater, OP_GT)

	static Msp430Binary* AccessArrayItem(const Msp430<A> &_lhs, const Msp430<B> &_rhs) {
		return new Msp430Binary(OP_ACCESS_ARRAY_ITEM, _lhs, _rhs);
	}
	void rootsDfs(RootsDfsItems &items) override { 
		lhs.rootsDfs(items);
		rhs.rootsDfs(items);
	}
};
#undef OPER

class Msp430Br: public Msp430ExprBase<void> {
protected:
	Msp430Base *target;
	bool after;
public:
	Msp430Br(Msp430Base *tgt, bool _after) : target(tgt), after(_after) {}
};

class Msp430BrFalse: public Msp430Br {
	Msp430<bool> cond;
public:
	Msp430BrFalse(Msp430<bool> _cond, Msp430Base *tgt, bool _after): cond(_cond), Msp430Br(tgt, _after) {}
	void rootsDfs(RootsDfsItems &items) override {}
};

typedef std::function<Msp430<bool>(void)> MspCond;
typedef std::function<void(void)> MspStmt;

class Msp430F2 {
public:
	void operator()(MspStmt stmt) {}
};

class Msp430F1 {
public:
	template<typename T>
	Msp430F2 operator()(Msp430<T> incr) {}
};
class Msp430F0 {
public:
	Msp430F0() {}
	Msp430F1 operator()(Msp430<bool> cond) {}
};

class Msp430S {
public:
	template<typename T>
	static Msp430F0 For(Msp430<T> init) {
		init.Root();
		return Msp430F0();
	}

	static void While(MspCond cond, MspStmt body) {

	}

	static void If(MspCond cond, MspStmt body) {
		std::unique_ptr<std::vector<Msp430Base*> > condStmts, bodyStmts;

		std::unique_ptr<Msp430<bool> > condRes;
		{
			Msp430Context::Local holder;
			condRes = std::unique_ptr<Msp430<bool> >(new Msp430<bool>(cond()));
			condStmts = holder.GetContext().ExtractAllStmts();
		}
		{
			Msp430Context::Local holder;
			body();
			bodyStmts = holder.GetContext().ExtractAllStmts();
		}
		condStmts->push_back(new Msp430<int>(new Msp430BrFalse(*condRes, bodyStmts->back(), true)));
	}

	/*	static void If(MspCond cond, MspStmt thenBranch, MspStmt elseBranch) {
	std::unique_ptr<std::vector<Msp430Base*> > condStmts, thenStmts, elseStmts;

	std::unique_ptr<Msp430<bool> > condRes;
	{
	Msp430Context::Local holder;
	condRes = std::unique_ptr<Msp430<bool> >(new Msp430<bool>(cond()));
	condStmts = holder.GetContext().ExtractAllStmts();
	}
	{
	Msp430Context::Local holder;
	thenBranch();
	bodyStmts = holder.GetContext().ExtractAllStmts();
	}
	{
	Msp430Context::Local holder;
	elseBranch();
	bodyStmts = holder.GetContext().ExtractAllStmts();
	}
	//condStmts->push_back(new Msp430BrFalse(condRes, bodyStmts->back(), true));
	}*/
};

Msp430<uint16_t> ADC10CTL0 = Msp430<uint16_t>::Var(MSPADDR_ADC10CTL0);
Msp430<uint16_t> ADC10CTL1 = Msp430<uint16_t>::Var(MSPADDR_ADC10CTL1);
Msp430<uint16_t> ADC10MCTL0 = Msp430<uint16_t>::Var(MSPADDR_ADC10MCTL0);

class Msp430DC {
	LLVMContext &context;
	Triple TheTriple;

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

	void bubbleSort() {
		auto i = Msp430MakeLocal<uint16_t>();
		auto j = Msp430MakeLocal<uint16_t>();
		auto n = Msp430MakeLocal<uint16_t>();
		auto arr = Msp430MakeLocal<uint16_t*>();
		//Msp430<uint16_t[10]> arr = Msp430<uint16_t>::Array<10>(0x100);
		//auto arr = Msp430MakeArray<uint16_t>(0x100);

		n.Assign(10);
		
		Msp430S::For(i.Zero())(i < n)(i++)([&]() {
			Msp430S::For(i.Zero())(j < n - i)(j++)([&]() {
				Msp430S::If([&]() { return arr[i] > arr[j]; }, [&]() {
					auto t = Msp430MakeLocal<uint16_t>();
					t.Assign(arr[i]);
					arr[i].Assign(arr[j]);
					arr[j].Assign(t);
				});
			});
		});
		/*Msp430S::For(
			[&]() { i.Assign((uint16_t)0); },
			[&]() { return i < n; },
			[&]() { ; }, 
			[&]() {
			Msp430<uint16_t> j = Msp430MakeLocal<uint16_t>();
			Msp430S::For(
				[&]() { j.Assign((uint16_t)0); },
				[&]() { return j < n - i; },
				[&]() { j += Msp430<uint16_t>::Const(1); }, [&]() {
				Msp430S::If([&]() { return arr[i] > arr[j]; }, [&]() {
					auto t = Msp430MakeLocal<uint16_t>();
					t.Assign(arr[i]);
					arr[i].Assign(arr[j]);
					arr[j].Assign(t);
				});
			});
		});*/
	}

	void bubbleSort2() {
		/*auto i = MSP::Var<uint16_t>();
		auto j = MSP::Var<uint16_t>();
		auto n = MSP::Var<uint16_t>();
		auto arr = MSP::Array<uint16_t, 10>(0x100);
		MSP::X(n.Assign(10));
			.X(MSP::For(i.Assign((uint16_t)0), i < n, i += 1,
				MSP::For(j.Assign((uint16_t)0), j < n - i, j += 1,
					MSP::If(arr[i] > arr[j], 
						MSP::X(t.Assign(arr[i]))
							.X(arr[i].Assign(arr[j]))
							.X(arr[j].Assign(t))
					)
				)
			));*/
	}

	void conversationPowerSensor() {
		//ADC10MCTL0.Assign(6);

		//ADC10CTL0 &= ~ADC10ENC;
		
		//ADC10CTL0 |= ADC10ENC + 1;
		//Msp430S::While((ADC10CTL1) == 0x01, [](){});
		/*results.powerSensor = (uint16_t)ADC10MEM0;

		
		auto ctx = Msp430Context();

		auto ADC10CTL1 = ctx.Absolute<uint16_t>(MSPADDR_ADC10CTL1);
		auto y = ctx.Absolute<uint16_t>(0x300);
		auto addr = ctx.Absolute<uint16_t*>(0x400);
		auto z = ctx.Local<uint16_t>();

		auto st1 = ctx.Local<xy>(1);

		x.assign(y + x); //x = y + x;
		z = x * 10;

		*addr = z;

		st1

		st1.fld(st1.StructVal.y) = z;

		Msp430::If(x == 0, []() {
			
		}, []() {

		});

		Msp430::While((ADC10CTL1 & ADC10BUSY) == 0x01, Msp430::Nop);

		auto compiled = ctx.outVar(x).outVar(z).compile();

		for (int i = 0; i < 10; i++) {
			x.set(i * 10); // i2c blocking op
			compiled.execute(); // i2c blocking op
			uint16_t xval = x.get();
			uint16_t yval = z.get();
		}*/
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