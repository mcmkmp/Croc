
#include <math.h>

#include "croc/api.h"
#include "croc/base/opcodes.hpp"
#include "croc/base/writebarrier.hpp"
#include "croc/internal/basic.hpp"
#include "croc/internal/calls.hpp"
#include "croc/internal/class.hpp"
#include "croc/internal/debug.hpp"
#include "croc/internal/eh.hpp"
#include "croc/internal/interpreter.hpp"
#include "croc/internal/stack.hpp"
#include "croc/internal/thread.hpp"
#include "croc/internal/variables.hpp"
#include "croc/types/base.hpp"

#define GetRS()\
	do {\
		if((*pc)->uimm & INST_CONSTBIT)\
			RS = &constTable[(*pc)->uimm & ~INST_CONSTBIT];\
		else\
			RS = &t->stack[stackBase + (*pc)->uimm]; (*pc)++;\
	} while(false)

#define GetRT()\
	do {\
		if((*pc)->uimm & INST_CONSTBIT)\
			RT = &constTable[(*pc)->uimm & ~INST_CONSTBIT];\
		else\
			RT = &t->stack[stackBase + (*pc)->uimm]; (*pc)++;\
	} while(false)

#define GetUImm() (((*pc)++)->uimm)
#define GetImm() (((*pc)++)->imm)

#define AdjustParams()\
	do {\
		if(numParams == 0)\
			numParams = t->stackIndex - (stackBase + rd + 1);\
		else\
		{\
			numParams--;\
			t->stackIndex = stackBase + rd + 1 + numParams;\
		}\
	} while(false)

namespace croc
{
	namespace
	{
		void binOpImpl(Thread* t, Op operation, AbsStack dest, Value RS, Value RT)
		{
			crocfloat f1;
			crocfloat f2;

			if(RS.type == CrocType_Int)
			{
				if(RT.type == CrocType_Int)
				{
					auto i1 = RS.mInt;
					auto i2 = RT.mInt;

					switch(operation)
					{
						case Op_Add: t->stack[dest] = Value::from(i1 + i2); return;
						case Op_Sub: t->stack[dest] = Value::from(i1 - i2); return;
						case Op_Mul: t->stack[dest] = Value::from(i1 * i2); return;

						case Op_Div:
							if(i2 == 0)
								croc_eh_throwStd(*t, "ValueError", "Integer divide by zero");

							t->stack[dest] = Value::from(i1 / i2);
							return;

						case Op_Mod:
							if(i2 == 0)
								croc_eh_throwStd(*t, "ValueError", "Integer modulo by zero");

							t->stack[dest] = Value::from(i1 % i2);
							return;

						default:
							assert(false);
					}
				}
				else if(RT.type == CrocType_Float)
				{
					f1 = RS.mInt;
					f2 = RT.mFloat;
					goto _float;
				}
			}
			else if(RS.type == CrocType_Float)
			{
				if(RT.type == CrocType_Int)
				{
					f1 = RS.mFloat;
					f2 = RT.mInt;
					goto _float;
				}
				else if(RT.type == CrocType_Float)
				{
					f1 = RS.mFloat;
					f2 = RT.mFloat;

				_float:
					switch(operation)
					{
						case Op_Add: t->stack[dest] = Value::from(f1 + f2); return;
						case Op_Sub: t->stack[dest] = Value::from(f1 - f2); return;
						case Op_Mul: t->stack[dest] = Value::from(f1 * f2); return;
						case Op_Div: t->stack[dest] = Value::from(f1 / f2); return;
						case Op_Mod: t->stack[dest] = Value::from(fmod(f1, f2)); return;

						default:
							assert(false);
					}
				}
			}

			const char* name;

			switch(operation)
			{
				case Op_Add: name = "add"; break;
				case Op_Sub: name = "subtract"; break;
				case Op_Mul: name = "multiply"; break;
				case Op_Div: name = "divide"; break;
				case Op_Mod: name = "modulo"; break;
				default: assert(false);
			}

			pushTypeStringImpl(t, RS);
			pushTypeStringImpl(t, RT);
			croc_eh_throwStd(*t, "TypeError", "Attempting to %s a '%s' and a '%s'",
				name, croc_getString(*t, -2), croc_getString(*t, -1));
		}

		void reflBinOpImpl(Thread* t, Op operation, AbsStack dest, Value src)
		{
			crocfloat f1;
			crocfloat f2;

			if(t->stack[dest].type == CrocType_Int)
			{
				if(src.type == CrocType_Int)
				{
					auto i2 = src.mInt;

					switch(operation)
					{
						case Op_AddEq: t->stack[dest].mInt += i2; return;
						case Op_SubEq: t->stack[dest].mInt -= i2; return;
						case Op_MulEq: t->stack[dest].mInt *= i2; return;

						case Op_DivEq:
							if(i2 == 0)
								croc_eh_throwStd(*t, "ValueError", "Integer divide by zero");

							t->stack[dest].mInt /= i2;
							return;

						case Op_ModEq:
							if(i2 == 0)
								croc_eh_throwStd(*t, "ValueError", "Integer modulo by zero");

							t->stack[dest].mInt %= i2;
							return;

						default: assert(false);
					}
				}
				else if(src.type == CrocType_Float)
				{
					f1 = t->stack[dest].mInt;
					f2 = src.mFloat;
					goto _float;
				}
			}
			else if(t->stack[dest].type == CrocType_Float)
			{
				if(src.type == CrocType_Int)
				{
					f1 = t->stack[dest].mFloat;
					f2 = src.mInt;
					goto _float;
				}
				else if(src.type == CrocType_Float)
				{
					f1 = t->stack[dest].mFloat;
					f2 = src.mFloat;

				_float:
					t->stack[dest].type = CrocType_Float;

					switch(operation)
					{
						case Op_AddEq: t->stack[dest].mFloat = f1 + f2; return;
						case Op_SubEq: t->stack[dest].mFloat = f1 - f2; return;
						case Op_MulEq: t->stack[dest].mFloat = f1 * f2; return;
						case Op_DivEq: t->stack[dest].mFloat = f1 / f2; return;
						case Op_ModEq: t->stack[dest].mFloat = fmod(f1, f2); return;

						default: assert(false);
					}
				}
			}

			const char* name;

			switch(operation)
			{
				case Op_AddEq: name = "add"; break;
				case Op_SubEq: name = "subtract"; break;
				case Op_MulEq: name = "multiply"; break;
				case Op_DivEq: name = "divide"; break;
				case Op_ModEq: name = "modulo"; break;
				default: assert(false);
			}

			pushTypeStringImpl(t, t->stack[dest]);
			pushTypeStringImpl(t, src);
			croc_eh_throwStd(*t, "TypeError", "Attempting to %s-assign a '%s' and a '%s'",
				name, croc_getString(*t, -2), croc_getString(*t, -1));
		}

		void binaryBinOpImpl(Thread* t, Op operation, AbsStack dest, Value RS, Value RT)
		{
			if(RS.type == CrocType_Int && RT.type == CrocType_Int)
			{
				switch(operation)
				{
					case Op_And:  t->stack[dest] = Value::from(RS.mInt & RT.mInt); return;
					case Op_Or:   t->stack[dest] = Value::from(RS.mInt | RT.mInt); return;
					case Op_Xor:  t->stack[dest] = Value::from(RS.mInt ^ RT.mInt); return;
					case Op_Shl:  t->stack[dest] = Value::from(RS.mInt << RT.mInt); return;
					case Op_Shr:  t->stack[dest] = Value::from(RS.mInt >> RT.mInt); return;

					case Op_UShr:
						t->stack[dest] = Value::from(cast(crocint)(cast(uint64_t)RS.mInt >> cast(uint64_t)RT.mInt));
						return;

					default: assert(false);
				}
			}

			const char* name;

			switch(operation)
			{
				case Op_And:  name = "and"; break;
				case Op_Or:   name = "or"; break;
				case Op_Xor:  name = "xor"; break;
				case Op_Shl:  name = "left-shift"; break;
				case Op_Shr:  name = "right-shift"; break;
				case Op_UShr: name = "unsigned right-shift"; break;
				default: assert(false);
			}

			pushTypeStringImpl(t, RS);
			pushTypeStringImpl(t, RT);
			croc_eh_throwStd(*t, "TypeError", "Attempting to bitwise %s a '%s' and a '%s'",
				name, croc_getString(*t, -2), croc_getString(*t, -1));
		}

		void reflBinaryBinOpImpl(Thread* t, Op operation, AbsStack dest, Value src)
		{
			if(t->stack[dest].type == CrocType_Int && src.type == CrocType_Int)
			{
				switch(operation)
				{
					case Op_AndEq:  t->stack[dest].mInt &= src.mInt; return;
					case Op_OrEq:   t->stack[dest].mInt |= src.mInt; return;
					case Op_XorEq:  t->stack[dest].mInt ^= src.mInt; return;
					case Op_ShlEq:  t->stack[dest].mInt <<= src.mInt; return;
					case Op_ShrEq:  t->stack[dest].mInt >>= src.mInt; return;

					case Op_UShrEq: {
						auto tmp = cast(uint64_t)t->stack[dest].mInt;
						tmp >>= cast(uint64_t)src.mInt;
						t->stack[dest].mInt = cast(crocint)tmp;
						return;
					}
					default: assert(false);
				}
			}

			const char* name;

			switch(operation)
			{
				case Op_AndEq:  name = "and"; break;
				case Op_OrEq:   name = "or"; break;
				case Op_XorEq:  name = "xor"; break;
				case Op_ShlEq:  name = "left-shift"; break;
				case Op_ShrEq:  name = "right-shift"; break;
				case Op_UShrEq: name = "unsigned right-shift"; break;
				default: assert(false);
			}

			pushTypeStringImpl(t, t->stack[dest]);
			pushTypeStringImpl(t, src);
			croc_eh_throwStd(*t, "TypeError", "Attempting to bitwise %s-assign a '%s' and a '%s'",
				name, croc_getString(*t, -2), croc_getString(*t, -1));
		}
	}

	void execute(Thread* t, uword startARIndex)
	{
		assert(t->stackIndex > 1); // for the exec EH frame
		jmp_buf buf;
		pushExecEHFrame(t, buf);
		auto savedNativeDepth = t->nativeCallDepth; // doesn't need to be volatile since it never changes value

	_exceptionRetry:
		auto ehStatus = setjmp(buf);
		if(ehStatus == EHStatus_Okay)
		{
		t->state = CrocThreadState_Running;
		t->vm->curThread = t;

	_reentry:
		Value* RS;
		Value* RT;
		assert(!t->currentAR->func->isNative);
		auto stackBase = t->stackBase;
		auto constTable = t->currentAR->func->scriptFunc->constants;
		auto env = t->currentAR->func->environment;
		auto upvals = t->currentAR->func->scriptUpvals();
		auto pc = &t->currentAR->pc;
		Instruction* oldPC = nullptr;

		while(true)
		{
			if(t->shouldHalt)
				croc_eh_throwStd(*t, "HaltException", "Thread halted");

			// assert(pc == &t->currentAR->pc);
			// pc = &t->currentAR->pc;
			Instruction* i = (*pc)++;

			if(t->hooksEnabled && t->hooks)
			{
				if(t->hooks & CrocThreadHook_Delay)
				{
					assert(t->hookCounter > 0);
					t->hookCounter--;

					if(t->hookCounter == 0)
					{
						t->hookCounter = t->hookDelay;
						callHook(t, CrocThreadHook_Delay);
					}
				}

				if(t->hooks & CrocThreadHook_Line)
				{
					auto curPC = t->currentAR->pc - 1;

					// when oldPC is null, it means we've either just started executing this func,
					// or we've come back from a yield, or we've just caught an exception, or something
					// like that.
					// When curPC < oldPC, we've jumped back, like to the beginning of a loop.

					if(curPC == t->currentAR->func->scriptFunc->code.ptr ||
						curPC < oldPC ||
						pcToLine(t->currentAR, curPC) != pcToLine(t->currentAR, oldPC))
						callHook(t, CrocThreadHook_Line);
				}
			}

			oldPC = *pc;

			auto opcode = cast(Op)INST_GET_OPCODE(*i);
			auto rd = INST_GET_RD(*i);

			switch(opcode)
			{
				// Binary Arithmetic
				case Op_Add:
				case Op_Sub:
				case Op_Mul:
				case Op_Div:
				case Op_Mod: GetRS(); GetRT(); binOpImpl(t, opcode, stackBase + rd, *RS, *RT); break;

				// Reflexive Arithmetic
				case Op_AddEq:
				case Op_SubEq:
				case Op_MulEq:
				case Op_DivEq:
				case Op_ModEq: GetRS(); reflBinOpImpl(t, opcode, stackBase + rd, *RS); break;

				// Binary Bitwise
				case Op_And:
				case Op_Or:
				case Op_Xor:
				case Op_Shl:
				case Op_Shr:
				case Op_UShr: GetRS(); GetRT(); binaryBinOpImpl(t, opcode, stackBase + rd, *RS, *RT); break;

				// Reflexive Bitwise
				case Op_AndEq:
				case Op_OrEq:
				case Op_XorEq:
				case Op_ShlEq:
				case Op_ShrEq:
				case Op_UShrEq: GetRS(); reflBinaryBinOpImpl(t, opcode, stackBase + rd, *RS); break;

				// Unary ops
				case Op_Neg:
					GetRS();

					if(RS->type == CrocType_Int)
						t->stack[stackBase + rd] = Value::from(-RS->mInt);
					else if(RS->type == CrocType_Float)
						t->stack[stackBase + rd] = Value::from(-RS->mFloat);
					else
					{
						pushTypeStringImpl(t, *RS);
						croc_eh_throwStd(*t, "TypeError", "Cannot perform negation on a '%s'", croc_getString(*t, -1));
					}
					break;

				case Op_Com:
					GetRS();

					if(RS->type == CrocType_Int)
						t->stack[stackBase + rd] = Value::from(~RS->mInt);
					else
					{
						pushTypeStringImpl(t, *RS);
						croc_eh_throwStd(*t, "TypeError", "Cannot perform bitwise complement on a '%s'",
							croc_getString(*t, -1));
					}
					break;

				case Op_AsBool:
					GetRS();
					t->stack[stackBase + rd] = Value::from(!RS->isFalse());
					break;

				case Op_AsInt:
					GetRS();

					switch(RS->type)
					{
						case CrocType_Bool:  t->stack[stackBase + rd] = Value::from((crocint)(RS->mBool ? 1 : 0)); break;
						case CrocType_Int:   t->stack[stackBase + rd] = *RS; break;
						case CrocType_Float: t->stack[stackBase + rd] = Value::from((crocint)RS->mFloat); break;

						default:
							pushTypeStringImpl(t, *RS);
							croc_eh_throwStd(*t, "TypeError", "Cannot convert type '%s' to int", croc_getString(*t, -1));
					}
					break;

				case Op_AsFloat:
					GetRS();

					switch(RS->type)
					{
						case CrocType_Int:   t->stack[stackBase + rd] = Value::from((crocfloat)RS->mInt); break;
						case CrocType_Float: t->stack[stackBase + rd] = *RS; break;

						default:
							pushTypeStringImpl(t, *RS);
							croc_eh_throwStd(*t, "TypeError", "Cannot convert type '%s' to float", croc_getString(*t, -1));
					}
					break;

				case Op_AsString:
					GetRS();
					toStringImpl(t, *RS, false);
					t->stack[stackBase + rd] = t->stack[t->stackIndex - 1];
					t->stackIndex--;
					break;

				// Crements
				case Op_Inc: {
					auto dest = stackBase + rd;

					if(t->stack[dest].type == CrocType_Int)
						t->stack[dest].mInt++;
					else if(t->stack[dest].type == CrocType_Float)
						t->stack[dest].mFloat++;
					else
					{
						pushTypeStringImpl(t, t->stack[dest]);
						croc_eh_throwStd(*t, "TypeError", "Cannot increment a '%s'", croc_getString(*t, -1));
					}
					break;
				}
				case Op_Dec: {
					auto dest = stackBase + rd;

					if(t->stack[dest].type == CrocType_Int)
						t->stack[dest].mInt--;
					else if(t->stack[dest].type == CrocType_Float)
						t->stack[dest].mFloat--;
					else
					{
						pushTypeStringImpl(t, t->stack[dest]);
						croc_eh_throwStd(*t, "TypeError", "Cannot decrement a '%s'", croc_getString(*t, -1));
					}
					break;
				}
				// Data Transfer
				case Op_Move:
					GetRS();
					t->stack[stackBase + rd] = *RS;
					break;

				case Op_NewGlobal:
					newGlobalImpl(t, constTable[GetUImm()].mString, env, t->stack[stackBase + rd]);
					break;

				case Op_GetGlobal:
					t->stack[stackBase + rd] = getGlobalImpl(t, constTable[GetUImm()].mString, env);
					break;

				case Op_SetGlobal:
					setGlobalImpl(t, constTable[GetUImm()].mString, env, t->stack[stackBase + rd]);
					break;

				case Op_GetUpval:  t->stack[stackBase + rd] = *upvals[GetUImm()]->value; break;
				case Op_SetUpval: {
					auto uv = upvals[GetUImm()];
					WRITE_BARRIER(t->vm->mem, uv);
					*uv->value = t->stack[stackBase + rd];
					break;
				}
				// Logical and Control Flow
				case Op_Not:
					GetRS();
					t->stack[stackBase + rd] = Value::from(RS->isFalse());
					break;

				case Op_Cmp3:
					GetRS();
					GetRT();
					t->stack[stackBase + rd] = Value::from(cmpImpl(t, *RS, *RT));
					break;

				case Op_Cmp: {
					GetRS();
					GetRT();
					auto jump = GetImm();

					auto cmpValue = cmpImpl(t, *RS, *RT);

					switch(cast(Comparison)rd)
					{
						case Comparison_LT: if(cmpValue < 0) (*pc) += jump; break;
						case Comparison_LE: if(cmpValue <= 0) (*pc) += jump; break;
						case Comparison_GT: if(cmpValue > 0) (*pc) += jump; break;
						case Comparison_GE: if(cmpValue >= 0) (*pc) += jump; break;
						default: assert(false);
					}
					break;
				}
				case Op_SwitchCmp: {
					GetRS();
					GetRT();
					auto jump = GetImm();

					if(switchCmpImpl(t, *RS, *RT))
						(*pc) += jump;
					break;
				}
				case Op_Equals: {
					GetRS();
					GetRT();
					auto jump = GetImm();

					if(equalsImpl(t, *RS, *RT) == cast(bool)rd)
						(*pc) += jump;
					break;
				}
				case Op_Is: {
					GetRS();
					GetRT();
					auto jump = GetImm();

					if((*RS == *RT) == cast(bool)rd)
						(*pc) += jump;

					break;
				}
				case Op_In: {
					GetRS();
					GetRT();
					auto jump = GetImm();

					if(inImpl(t, *RS, *RT) == cast(bool)rd)
						(*pc) += jump;
					break;
				}
				case Op_IsTrue: {
					GetRS();
					auto jump = GetImm();

					if(RS->isFalse() != cast(bool)rd)
						(*pc) += jump;

					break;
				}
				case Op_Jmp: {
					// If we ever change the format of this opcode, check that it's the same length as Switch (codegen
					// can turn Switch into Jmp)!
					auto jump = GetImm();

					if(rd != 0)
						(*pc) += jump;
					break;
				}
				case Op_Switch: {
					// If we ever change the format of this opcode, check that it's the same length as Jmp (codegen can
					// turn Switch into Jmp)!
					auto st = &t->currentAR->func->scriptFunc->switchTables[rd];
					GetRS();

					if(auto ptr = st->offsets.lookup(*RS))
						(*pc) += *ptr;
					else
					{
						if(st->defaultOffset == -1)
							croc_eh_throwStd(*t, "SwitchError", "Switch without default");

						(*pc) += st->defaultOffset;
					}
					break;
				}
				case Op_Close: closeUpvals(t, stackBase + rd); break;

				case Op_For: {
					auto jump = GetImm();
					auto idx = &t->stack[stackBase + rd];
					auto hi = idx + 1;
					auto step = hi + 1;

					if(idx->type != CrocType_Int || hi->type != CrocType_Int || step->type != CrocType_Int)
						croc_eh_throwStd(*t, "TypeError", "Numeric for loop low, high, and step values must be integers");

					auto intIdx = idx->mInt;
					auto intHi = hi->mInt;
					auto intStep = step->mInt;

					if(intStep == 0)
						croc_eh_throwStd(*t, "ValueError", "Numeric for loop step value may not be 0");

					if((intIdx > intHi && intStep > 0) || (intIdx < intHi && intStep < 0))
						intStep = -intStep;

					if(intStep < 0)
					{
						auto newIdx = ((intIdx - intHi) / intStep) * intStep;

						if(newIdx == intIdx)
							newIdx += intStep;

						*idx = Value::from(newIdx);
					}

					*step = Value::from(intStep);
					(*pc) += jump;
					break;
				}
				case Op_ForLoop: {
					auto jump = GetImm();
					auto idx = t->stack[stackBase + rd].mInt;
					auto hi = t->stack[stackBase + rd + 1].mInt;
					auto step = t->stack[stackBase + rd + 2].mInt;

					if(step > 0)
					{
						if(idx < hi)
						{
							t->stack[stackBase + rd + 3] = Value::from(idx);
							t->stack[stackBase + rd] = Value::from(idx + step);
							(*pc) += jump;
						}
					}
					else
					{
						if(idx >= hi)
						{
							t->stack[stackBase + rd + 3] = Value::from(idx);
							t->stack[stackBase + rd] = Value::from(idx + step);
							(*pc) += jump;
						}
					}
					break;
				}
				case Op_Foreach: {
					auto jump = GetImm();
					auto src = t->stack[stackBase + rd];

					if(src.type != CrocType_Function && src.type != CrocType_Thread)
					{
						auto method = getMM(t, src, MM_Apply);

						if(method == nullptr)
						{
							pushTypeStringImpl(t, src);
							croc_eh_throwStd(*t, "TypeError", "No implementation of %s for type '%s'",
								MetaNames[MM_Apply], croc_getString(*t, -1));
						}

						t->stack[stackBase + rd + 2] = t->stack[stackBase + rd + 1];
						t->stack[stackBase + rd + 1] = src;
						t->stack[stackBase + rd] = Value::from(method);

						t->stackIndex = stackBase + rd + 3;
						commonCall(t, stackBase + rd, 3, callPrologue(t, stackBase + rd, 3, 2));
						t->stackIndex = t->currentAR->savedTop;

						src = t->stack[stackBase + rd];

						if(src.type != CrocType_Function && src.type != CrocType_Thread)
						{
							pushTypeStringImpl(t, src);
							croc_eh_throwStd(*t, "TypeError", "Invalid iterable type '%s' returned from opApply",
								croc_getString(*t, -1));
						}
					}

					if(src.type == CrocType_Thread && src.mThread->state != CrocThreadState_Initial)
						croc_eh_throwStd(*t, "StateError",
							"Attempting to iterate over a thread that is not in the 'initial' state");

					(*pc) += jump;
					break;
				}
				case Op_ForeachLoop: {
					auto numIndices = GetUImm();
					auto jump = GetImm();

					auto funcReg = rd + 3;

					t->stack[stackBase + funcReg + 2] = t->stack[stackBase + rd + 2];
					t->stack[stackBase + funcReg + 1] = t->stack[stackBase + rd + 1];
					t->stack[stackBase + funcReg] = t->stack[stackBase + rd];

					t->stackIndex = stackBase + funcReg + 3;
					commonCall(t, stackBase + funcReg, numIndices, callPrologue(t, stackBase + funcReg, numIndices, 2));
					t->stackIndex = t->currentAR->savedTop;

					auto src = &t->stack[stackBase + rd];

					if(src->type == CrocType_Function)
					{
						if(t->stack[stackBase + funcReg].type != CrocType_Null)
						{
							t->stack[stackBase + rd + 2] = t->stack[stackBase + funcReg];
							(*pc) += jump;
						}
					}
					else
					{
						if(src->mThread->state != CrocThreadState_Dead)
							(*pc) += jump;
					}
					break;
				}
				// Exception Handling
				case Op_PushCatch:
				case Op_PushFinally: {
					auto offs = GetImm();
					pushScriptEHFrame(t, opcode == Op_PushCatch, cast(RelStack)rd, t->currentAR->pc + offs);
					break;
				}
				case Op_PopEH: popScriptEHFrame(t); break;

				case Op_EndFinal:
					if(t->vm->exception != nullptr)
						throwImpl(t, Value::from(t->vm->exception), true);

					if(t->currentAR->unwindReturn != nullptr)
						unwind(t);

					break;

				case Op_Throw:
					GetRS();
					throwImpl(t, *RS, cast(bool)rd);
					assert(false); // should never get here

				// Function Calling
			{
				bool isScript;
				bool isTailcall;
				word numResults;
				uword numParams;

				case Op_TailMethod:
				case Op_Method:
					isTailcall = opcode == Op_TailMethod;
					GetRS();
					GetRT();
					numParams = GetUImm();
					numResults = GetUImm() - 1;

					if(isTailcall)
						numResults = -1; // the second uimm is a dummy for these opcodes

					if(RT->type != CrocType_String)
					{
						pushTypeStringImpl(t, *RT);
						croc_eh_throwStd(*t, "TypeError",
							"Attempting to get a method with a non-string name (type '%s' instead)",
							croc_getString(*t, -1));
					}

					AdjustParams();
					isScript = methodCallPrologue(t, stackBase + rd, *RS, RT->mString, numResults, numParams,
						isTailcall);
					goto _commonCall;

				case Op_Call:
				case Op_TailCall:
					isTailcall = opcode == Op_TailCall;
					numParams = GetUImm();
					numResults = GetUImm() - 1;

					if(isTailcall)
						numResults = -1; // second uimm is a dummy

					AdjustParams();
					isScript = callPrologue(t, stackBase + rd, numResults, numParams, isTailcall);

					// fall through
				_commonCall:
					croc_gc_maybeCollect(*t);

					if(!isScript && !isTailcall && numResults >= 0)
						t->stackIndex = t->currentAR->savedTop;

					// We always go to reentry, even with native tailcalls, since script hook funcs may be run before
					// native tailcalls.
					goto _reentry;
			}

				case Op_SaveRets: {
					auto numResults = GetUImm();
					auto firstResult = stackBase + rd;

					if(numResults == 0)
					{
						saveResults(t, t, firstResult, t->stackIndex - firstResult);
						t->stackIndex = t->currentAR->savedTop;
					}
					else
						saveResults(t, t, firstResult, numResults - 1);
					break;
				}
				case Op_Ret: {
					callEpilogue(t);

					if(t->arIndex < startARIndex)
						goto _return;

					goto _reentry;
				}
				case Op_Unwind:
					t->currentAR->unwindReturn = (*pc);
					t->currentAR->unwindCounter = rd;
					unwind(t);
					break;

				case Op_Vararg: {
					uword numNeeded = GetUImm();
					auto numVarargs = stackBase - t->currentAR->vargBase;
					auto dest = stackBase + rd;

					if(numNeeded == 0)
					{
						numNeeded = numVarargs;
						t->stackIndex = dest + numVarargs;
						checkStack(t, t->stackIndex);
					}
					else
						numNeeded--;

					auto src = t->currentAR->vargBase;

					if(numNeeded <= numVarargs)
						memmove(&t->stack[dest], &t->stack[src], numNeeded * sizeof(Value));
					else
					{
						memmove(&t->stack[dest], &t->stack[src], numVarargs * sizeof(Value));
						t->stack.slice(dest + numVarargs, dest + numNeeded).fill(Value::nullValue);
					}

					break;
				}
				case Op_VargLen:
					t->stack[stackBase + rd] = Value::from(cast(crocint)(stackBase - t->currentAR->vargBase));
					break;

				case Op_VargIndex: {
					GetRS();

					auto numVarargs = stackBase - t->currentAR->vargBase;

					if(RS->type != CrocType_Int)
					{
						pushTypeStringImpl(t, *RS);
						croc_eh_throwStd(*t, "TypeError", "Attempting to index 'vararg' with a '%s'",
							croc_getString(*t, -1));
					}

					auto index = RS->mInt;

					if(index < 0)
						index += numVarargs;

					if(index < 0 || cast(uword)index >= numVarargs)
						croc_eh_throwStd(*t, "BoundsError",
							"Invalid 'vararg' index: %" CROC_INTEGER_FORMAT " (only have %" CROC_SIZE_T_FORMAT ")",
							index, numVarargs);

					t->stack[stackBase + rd] = t->stack[t->currentAR->vargBase + cast(uword)index];
					break;
				}
				case Op_VargIndexAssign: {
					GetRS();
					GetRT();

					auto numVarargs = stackBase - t->currentAR->vargBase;

					if(RS->type != CrocType_Int)
					{
						pushTypeStringImpl(t, *RS);
						croc_eh_throwStd(*t, "TypeError", "Attempting to index 'vararg' with a '%s'",
							croc_getString(*t, -1));
					}

					auto index = RS->mInt;

					if(index < 0)
						index += numVarargs;

					if(index < 0 || cast(uword)index >= numVarargs)
						croc_eh_throwStd(*t, "BoundsError",
							"Invalid 'vararg' index: %" CROC_INTEGER_FORMAT " (only have %" CROC_SIZE_T_FORMAT ")",
							index, numVarargs);

					t->stack[t->currentAR->vargBase + cast(uword)index] = *RT;
					break;
				}
				case Op_Yield: {
					auto numParams = cast(word)GetUImm() - 1;
					auto numResults = cast(word)GetUImm() - 1;

					if(t == t->vm->mainThread)
						croc_eh_throwStd(*t, "RuntimeError", "Attempting to yield out of the main thread");

					if(t->nativeCallDepth > 0)
						croc_eh_throwStd(*t, "RuntimeError",
							"Attempting to yield across native / metamethod call boundary");

					t->savedStartARIndex = startARIndex;
					yieldImpl(t, stackBase + rd, numParams, numResults);
					goto _return;
				}
				case Op_CheckParams: {
					auto val = &t->stack[stackBase];
					auto masks = t->currentAR->func->scriptFunc->paramMasks;

					for(uword idx = 0; idx < masks.length; idx++)
					{
						if(!(masks[idx] & (1 << val->type)))
						{
							pushTypeStringImpl(t, *val);

							if(idx == 0)
								croc_eh_throwStd(*t, "TypeError", "'this' parameter: type '%s' is not allowed",
									croc_getString(*t, -1));
							else
								croc_eh_throwStd(*t, "TypeError",
									"Parameter %" CROC_SIZE_T_FORMAT ": type '%s' is not allowed",
									idx, croc_getString(*t, -1));
						}

						val++;
					}
					break;
				}
				case Op_CheckObjParam: {
					auto RD = &t->stack[stackBase + rd];
					GetRS();
					auto jump = GetImm();

					if(RD->type != CrocType_Instance)
						(*pc) += jump;
					else
					{
						if(RS->type != CrocType_Class)
						{
							pushTypeStringImpl(t, *RS);

							if(rd == 0)
								croc_eh_throwStd(*t, "TypeError",
									"'this' parameter: instance type constraint type must be 'class', not '%s'",
									croc_getString(*t, -1));
							else
								croc_eh_throwStd(*t, "TypeError",
									"Parameter %u: instance type constraint type must be 'class', not '%s'",
									rd, croc_getString(*t, -1));
						}

						if(RD->mInstance->derivesFrom(RS->mClass))
							(*pc) += jump;
					}
					break;
				}
				case Op_ObjParamFail: {
					pushTypeStringImpl(t, t->stack[stackBase + rd]);

					if(rd == 0)
						croc_eh_throwStd(*t, "TypeError", "'this' parameter: type '%s' is not allowed",
							croc_getString(*t, -1));
					else
						croc_eh_throwStd(*t, "TypeError", "Parameter %d: type '%s' is not allowed",
							rd, croc_getString(*t, -1));

					break;
				}
				case Op_CustomParamFail: {
					GetRS();

					if(rd == 0)
						croc_eh_throwStd(*t, "TypeError", "'this' parameter: value does not satisfy constraint '%s'",
							RS->mString->toCString());
					else
						croc_eh_throwStd(*t, "TypeError",
							"Parameter %d: value does not satisfy constraint '%s'",
							rd, RS->mString->toCString());
					break;
				}
				case Op_CheckRets: {
					auto val = &t->results[t->currentAR->firstResult];
					auto actualReturns = t->currentAR->numResults;
					auto func = t->currentAR->func->scriptFunc;
					auto masks = func->returnMasks;

					if(!func->isVarret && actualReturns > func->numReturns)
						croc_eh_throwStd(*t, "ParamError", "Function %s expects at most %" CROC_SIZE_T_FORMAT
							" returns but was given %" CROC_SIZE_T_FORMAT,
							func->name->toCString(), func->numReturns, actualReturns);

					for(uword idx = 0; idx < masks.length; idx++)
					{
						auto ok = (idx < actualReturns) ?
							(masks[idx] & (1 << val->type)) :
							(masks[idx] & (1 << CrocType_Null));

						if(!ok)
						{
							if(idx < actualReturns)
								pushTypeStringImpl(t, *val);
							else
								pushTypeStringImpl(t, Value::nullValue);

							croc_eh_throwStd(*t, "TypeError",
								"Return %" CROC_SIZE_T_FORMAT ": type '%s' is not allowed",
								idx + 1, croc_getString(*t, -1));
						}

						val++;
					}
					break;
				}
				case Op_CheckObjRet: {
					auto returns = &t->results[t->currentAR->firstResult];
					auto actualReturns = t->currentAR->numResults;
					auto val = (cast(uword)rd < actualReturns) ? &returns[rd] : &Value::nullValue;
					GetRS();
					auto jump = GetImm();

					if(val->type != CrocType_Instance)
						(*pc) += jump;
					else
					{
						if(RS->type != CrocType_Class)
						{
							pushTypeStringImpl(t, *RS);

							croc_eh_throwStd(*t, "TypeError",
								"Return %u: instance type constraint type must be 'class', not '%s'",
								rd + 1, croc_getString(*t, -1));
						}

						if(val->mInstance->derivesFrom(RS->mClass))
							(*pc) += jump;
					}
					break;
				}
				case Op_ObjRetFail: {
					auto returns = &t->results[t->currentAR->firstResult];
					auto actualReturns = t->currentAR->numResults;
					auto val = (cast(uword)rd < actualReturns) ? &returns[rd] : &Value::nullValue;
					pushTypeStringImpl(t, *val);

					croc_eh_throwStd(*t, "TypeError", "Return %d: type '%s' is not allowed",
						rd + 1, croc_getString(*t, -1));

					break;
				}
				case Op_CustomRetFail: {
					GetRS();

					croc_eh_throwStd(*t, "TypeError", "Return %d: value does not satisfy constraint '%s'",
						rd + 1, RS->mString->toCString());
					break;
				}
				case Op_MoveRet: {
					auto ret = GetUImm();
					auto returns = &t->results[t->currentAR->firstResult];
					auto actualReturns = t->currentAR->numResults;
					auto val = (cast(uword)ret < actualReturns) ? &returns[ret] : &Value::nullValue;
					t->stack[stackBase + rd] = *val;
					break;
				}
				case Op_RetAsFloat: {
					auto returns = &t->results[t->currentAR->firstResult];
					auto actualReturns = t->currentAR->numResults;
					auto val = (cast(uword)rd < actualReturns) ? &returns[rd] : &Value::nullValue;

					switch(val->type)
					{
						case CrocType_Int:   if(cast(uword)rd < actualReturns) returns[rd] = Value::from((crocfloat)val->mInt); break;
						case CrocType_Float: if(cast(uword)rd < actualReturns) returns[rd] = *val; break;

						default:
							pushTypeStringImpl(t, *val);
							croc_eh_throwStd(*t, "TypeError", "Cannot convert type '%s' to float", croc_getString(*t, -1));
					}
					break;
				}
				case Op_AssertFail: {
					auto msg = t->stack[stackBase + rd];

					if(msg.type != CrocType_String)
					{
						pushTypeStringImpl(t, msg);
						croc_eh_throwStd(*t, "AssertError",
							"Assertion failed, but the message is a '%s', not a 'string'", croc_getString(*t, -1));
					}

					croc_eh_throwStd(*t, "AssertError", "%s", msg.mString->toCString());
					assert(false);
				}
				// Array and List Operations
				case Op_Length:       GetRS(); lenImpl(t, stackBase + rd, *RS);  break;
				case Op_LengthAssign: GetRS(); lenaImpl(t, t->stack[stackBase + rd], *RS); break;
				case Op_Append:       GetRS(); t->stack[stackBase + rd].mArray->append(t->vm->mem, *RS); break;

				case Op_SetArray: {
					auto numVals = GetUImm();
					auto block = GetUImm();
					auto sliceBegin = stackBase + rd + 1;
					auto a = t->stack[stackBase + rd].mArray;

					if(numVals == 0)
					{
						a->setBlock(t->vm->mem, block, t->stack.slice(sliceBegin, t->stackIndex));
						t->stackIndex = t->currentAR->savedTop;
					}
					else
						a->setBlock(t->vm->mem, block, t->stack.slice(sliceBegin, sliceBegin + numVals - 1));

					break;
				}
				case Op_Cat: {
					auto rs = GetUImm();
					auto numVals = GetUImm();
					catImpl(t, stackBase + rd, stackBase + rs, numVals);
					croc_gc_maybeCollect(*t);
					break;
				}
				case Op_CatEq: {
					auto rs = GetUImm();
					auto numVals = GetUImm();
					catEqImpl(t, stackBase + rd, stackBase + rs, numVals);
					croc_gc_maybeCollect(*t);
					break;
				}
				case Op_Index:       GetRS(); GetRT(); idxImpl(t, stackBase + rd, *RS, *RT);  break;
				case Op_IndexAssign: GetRS(); GetRT(); idxaImpl(t, stackBase + rd, *RS, *RT); break;

				case Op_Field: {
					GetRS();
					GetRT();

					if(RT->type != CrocType_String)
					{
						pushTypeStringImpl(t, *RT);
						croc_eh_throwStd(*t, "TypeError", "Field name must be a string, not a '%s'",
							croc_getString(*t, -1));
					}

					fieldImpl(t, stackBase + rd, *RS, RT->mString, false);
					break;
				}
				case Op_FieldAssign: {
					GetRS();
					GetRT();

					if(RS->type != CrocType_String)
					{
						pushTypeStringImpl(t, *RS);
						croc_eh_throwStd(*t, "TypeError", "Field name must be a string, not a '%s'",
							croc_getString(*t, -1));
					}

					fieldaImpl(t, stackBase + rd, RS->mString, *RT, false);
					break;
				}
				case Op_Slice: {
					auto rs = GetUImm();
					auto base = &t->stack[stackBase + rs];
					sliceImpl(t, stackBase + rd, base[0], base[1], base[2]);
					break;
				}
				case Op_SliceAssign: {
					GetRS();
					auto base = &t->stack[stackBase + rd];
					sliceaImpl(t, base[0], base[1], base[2], *RS);
					break;
				}
				// Value Creation
				case Op_NewArray: {
					auto size = cast(uword)constTable[GetUImm()].mInt;
					t->stack[stackBase + rd] = Value::from(Array::create(t->vm->mem, size));
					croc_gc_maybeCollect(*t);
					break;
				}
				case Op_NewTable: {
					t->stack[stackBase + rd] = Value::from(Table::create(t->vm->mem));
					croc_gc_maybeCollect(*t);
					break;
				}
				case Op_Closure:
				case Op_ClosureWithEnv: {
					auto closureIdx = GetUImm();
					auto newDef = t->currentAR->func->scriptFunc->innerFuncs[closureIdx];
					auto funcEnv = (opcode == Op_Closure) ? env : t->stack[stackBase + rd].mNamespace;
					auto n = Function::create(t->vm->mem, funcEnv, newDef);

					if(n == nullptr)
					{
						toStringImpl(t, Value::from(newDef), false);
						croc_eh_throwStd(*t, "RuntimeError",
							"Attempting to instantiate %s with a different namespace than was associated with it",
							croc_getString(*t, -1));
					}

					auto uvTable = newDef->upvals;
					auto newUpvals = n->scriptUpvals();

					for(uword id = 0; id < uvTable.length; id++)
					{
						if(uvTable[id].isUpval)
							newUpvals[id] = upvals[uvTable[id].index];
						else
							newUpvals[id] = findUpval(t, uvTable[id].index);
					}

					t->stack[stackBase + rd] = Value::from(n);
					croc_gc_maybeCollect(*t);
					break;
				}
				case Op_Class: {
					GetRS();
					GetRT();

					auto cls = Class::create(t->vm->mem, RS->mString);
					auto numBases = GetUImm();

					for(auto &base: DArray<Value>::n(RT, numBases))
					{
						if(base.type != CrocType_Class)
						{
							pushTypeStringImpl(t, base);
							croc_eh_throwStd(*t, "TypeError", "Attempting to derive a class from a value of type '%s'",
								croc_getString(*t, -1));
						}

						classDeriveImpl(t, cls, base.mClass);
					}

					t->stack[stackBase + rd] = Value::from(cls);
					croc_gc_maybeCollect(*t);
					break;
				}
				case Op_Namespace: {
					auto name = constTable[GetUImm()].mString;
					GetRT();

					if(RT->type == CrocType_Null)
						t->stack[stackBase + rd] = Value::from(Namespace::create(t->vm->mem, name));
					else if(RT->type == CrocType_Namespace)
						t->stack[stackBase + rd] = Value::from(Namespace::create(t->vm->mem, name, RT->mNamespace));
					else
					{
						pushTypeStringImpl(t, *RT);
						push(t, Value::from(name));
						croc_eh_throwStd(*t, "TypeError",
							"Attempted to use a '%s' as a parent namespace for namespace '%s'",
							croc_getString(*t, -2), croc_getString(*t, -1));
					}

					croc_gc_maybeCollect(*t);
					break;
				}
				case Op_NamespaceNP: {
					auto name = constTable[GetUImm()].mString;
					t->stack[stackBase + rd] = Value::from(Namespace::create(t->vm->mem, name, env));
					croc_gc_maybeCollect(*t);
					break;
				}
				case Op_SuperOf: {
					GetRS();
					t->stack[stackBase + rd] = superOfImpl(t, *RS);
					break;
				}
				case Op_AddMember: {
					auto cls = &t->stack[stackBase + rd];
					GetRS();
					GetRT();
					auto flags = GetUImm();

					// should be guaranteed this by codegen
					assert(cls->type == CrocType_Class && RS->type == CrocType_String);

					auto isMethod = (flags & 1) != 0;
					auto isOverride = (flags & 2) != 0;

					auto okay = isMethod?
						cls->mClass->addMethod(t->vm->mem, RS->mString, *RT, isOverride) :
						cls->mClass->addField(t->vm->mem, RS->mString, *RT, isOverride);

					if(!okay)
					{
						auto name = RS->mString->toCString();
						auto clsName = cls->mClass->name->toCString();

						if(isOverride)
							croc_eh_throwStd(*t, "FieldError",
								"Attempting to override %s '%s' in class '%s', but no such member already exists",
								isMethod ? "method" : "field", name, clsName);
						else
							croc_eh_throwStd(*t, "FieldError",
								"Attempting to add a %s '%s' which already exists to class '%s'",
								isMethod ? "method" : "field", name, clsName);
					}
					break;
				}
				default:
					croc_eh_throwStd(*t, "VMError", "Unimplemented opcode %s", OpNames[cast(uword)opcode]);
			}
		}
		}
		else // catch!
		{
			assert(t->vm->curThread == t);
			t->nativeCallDepth = savedNativeDepth;

			if(ehStatus == EHStatus_ScriptFrame)
			{
				popScriptEHFrame(t);
				goto _exceptionRetry;
			}
			else
			{
				assert(ehStatus == EHStatus_NativeFrame);
				popNativeEHFrame(t);
				throwImpl(t, t->stack[t->stackIndex - 1], true);
			}
		}

	_return:
		t->nativeCallDepth = savedNativeDepth;
		popNativeEHFrame(t);
	}
}