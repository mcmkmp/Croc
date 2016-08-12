#ifndef CROC_COMPILER_AST_HPP
#define CROC_COMPILER_AST_HPP

#include "croc/compiler/types.hpp"

inline void* operator new(uword size, Compiler& c)
{
	return c.allocNode(size);
}

#define AST_AS(T, exp) ((exp)->type == AstTag_##T ? cast(T*)(exp) : nullptr)

#define AST_LIST(X)\
	X(Identifier,           "identifier",                      AstNode)\
\
	X(FuncDef,              "function definition",             AstNode)\
\
	X(Decorator,            "decorator",                       AstNode)\
\
	X(VarDecl,              "variable declaration",            Statement)\
	X(FuncDecl,             "function declaration",            Statement)\
\
	X(ImportStmt,           "import statement",                Statement)\
	X(BlockStmt,            "block statement",                 Statement)\
	X(ScopeStmt,            "scope statement",                 Statement)\
	X(ExpressionStmt,       "expression statement",            Statement)\
	X(IfStmt,               "'if' statement",                  Statement)\
	X(WhileStmt,            "'while' statement",               Statement)\
	X(DoWhileStmt,          "'do-while' statement",            Statement)\
	X(ForStmt,              "'for' statement",                 Statement)\
	X(ForNumStmt,           "numeric 'for' statement",         Statement)\
	X(ForeachStmt,          "'foreach' statement",             Statement)\
	X(ContinueStmt,         "'continue' statement",            Statement)\
	X(BreakStmt,            "'break' statement",               Statement)\
	X(ReturnStmt,           "'return' statement",              Statement)\
\
	X(AssignStmt,           "assignment",                      Statement)\
	X(AddAssignStmt,        "addition assignment",             Statement)\
	X(SubAssignStmt,        "subtraction assignment",          Statement)\
	X(MulAssignStmt,        "multiplication assignment",       Statement)\
	X(DivAssignStmt,        "division assignment",             Statement)\
	X(ModAssignStmt,        "modulo assignment",               Statement)\
	X(OrAssignStmt,         "bitwise 'or' assignment",         Statement)\
	X(XorAssignStmt,        "bitwise 'xor' assignment",        Statement)\
	X(AndAssignStmt,        "bitwise 'and' assignment",        Statement)\
	X(ShlAssignStmt,        "left-shift assignment",           Statement)\
	X(ShrAssignStmt,        "right-shift assignment",          Statement)\
	X(UShrAssignStmt,       "unsigned right-shift assignment", Statement)\
	X(CondAssignStmt,       "conditional assignment",          Statement)\
	X(IncStmt,              "increment",                       Statement)\
	X(DecStmt,              "decrement",                       Statement)\
\
	X(CondExp,              "conditional expression",          Expression)\
	X(OrOrExp,              "logical 'or' expression",         Expression)\
	X(AndAndExp,            "logical 'and' expression",        Expression)\
	X(OrExp,                "bitwise 'or' expression",         Expression)\
	X(XorExp,               "bitwise 'xor' expression",        Expression)\
	X(AndExp,               "bitwise 'and' expression",        Expression)\
	X(EqualExp,             "equality expression",             Expression)\
	X(NotEqualExp,          "inequality expression",           Expression)\
	X(IsExp,                "identity expression",             Expression)\
	X(NotIsExp,             "non-identity expression",         Expression)\
	X(LTExp,                "less-than expression",            Expression)\
	X(LEExp,                "less-or-equals expression",       Expression)\
	X(GTExp,                "greater-than expression",         Expression)\
	X(GEExp,                "greater-or-equals expression",    Expression)\
	X(ShlExp,               "left-shift expression",           Expression)\
	X(ShrExp,               "right-shift expression",          Expression)\
	X(UShrExp,              "unsigned right-shift expression", Expression)\
	X(AddExp,               "addition expression",             Expression)\
	X(SubExp,               "subtraction expression",          Expression)\
	X(CatExp,               "concatenation expression",        Expression)\
	X(MulExp,               "multiplication expression",       Expression)\
	X(DivExp,               "division expression",             Expression)\
	X(ModExp,               "modulo expression",               Expression)\
	X(NegExp,               "negation expression",             Expression)\
	X(NotExp,               "logical 'not' expression",        Expression)\
	X(ComExp,               "bitwise complement expression",   Expression)\
	X(LenExp,               "length expression",               Expression)\
	X(VargLenExp,           "vararg length expression",        Expression)\
	X(DotExp,               "dot expression",                  Expression)\
	X(IndexExp,             "index expression",                Expression)\
	X(VargIndexExp,         "vararg index expression",         Expression)\
	X(CallExp,              "call expression",                 Expression)\
	X(MethodCallExp,        "method call expression",          Expression)\
	X(IdentExp,             "identifier expression",           Expression)\
	X(ThisExp,              "'this' expression",               Expression)\
	X(NullExp,              "'null' expression",               Expression)\
	X(BoolExp,              "boolean constant expression",     Expression)\
	X(VarargExp,            "'vararg' expression",             Expression)\
	X(IntExp,               "integer constant expression",     Expression)\
	X(FloatExp,             "float constant expression",       Expression)\
	X(StringExp,            "string constant expression",      Expression)\
	X(FuncLiteralExp,       "function literal expression",     Expression)\
	X(ParenExp,             "parenthesized expression",        Expression)\
	X(TableCtorExp,         "table constructor expression",    Expression)\
	X(ArrayCtorExp,         "array constructor expression",    Expression)\
	X(YieldExp,             "yield expression",                Expression)

enum AstTag
{
#define POOP(Tag, _, __) AstTag_##Tag,
	AST_LIST(POOP)
	AstTag_NUMBER
#undef POOP
};

extern const char* AstTagNames[AstTag_NUMBER];
extern const char* NiceAstTagNames[AstTag_NUMBER];

enum class Protection
{
	Default,
	Local,
	Global
};

struct Decorator;
struct Expression;
struct FuncLiteralExp;
struct IdentExp;
struct Identifier;
struct Statement;
struct VarDecl;

struct FuncParam
{
	Identifier* name;

	FuncParam() :
		name()
	{}

	FuncParam(Identifier* name) :
		name(name)
	{}
};

struct ForStmtInit
{
	bool isDecl;

	union
	{
		Statement* stmt;
		VarDecl* decl;
	};
};

struct TableCtorField
{
	Expression* key;
	Expression* value;

	TableCtorField() :
		key(),
		value()
	{}

	TableCtorField(Expression* key, Expression* value) :
		key(key),
		value(value)
	{}
};

struct AstNode
{
	CompileLoc location;
	CompileLoc endLocation;
	AstTag type;

	AstNode(CompileLoc location, CompileLoc endLocation, AstTag type) :
		location(location),
		endLocation(endLocation),
		type(type)
	{}

	inline const char* toString()   { return AstTagNames[type];     }
	inline const char* niceString() { return NiceAstTagNames[type]; }
};

struct Statement : public AstNode
{
	Statement(CompileLoc location, CompileLoc endLocation, AstTag type) :
		AstNode(location, endLocation, type)
	{}
};

struct OpAssignStmt : public Statement
{
	Expression* lhs;
	Expression* rhs;

	OpAssignStmt(CompileLoc location, CompileLoc endLocation, AstTag type, Expression* lhs, Expression* rhs) :
		Statement(location, endLocation, type),
		lhs(lhs),
		rhs(rhs)
	{}
};

struct Expression : public AstNode
{
	crocstr sourceStr;

	Expression(CompileLoc location, CompileLoc endLocation, AstTag type) :
		AstNode(location, endLocation, type)
	{}

	inline void checkToNothing(Compiler& c)
	{
		if(!hasSideEffects())
			c.loneStmtException(location, "%s cannot exist on its own", niceString());
	}

	inline void checkMultRet(Compiler& c)
	{
		if(!isMultRet())
			c.semException(location, "%s cannot be the source of a multi-target assignment", niceString());
	}

	inline void checkLHS(Compiler& c)
	{
		if(!isLHS())
			c.semException(location, "%s cannot be the target of an assignment", niceString());
	}

	bool hasSideEffects();
	bool isMultRet();
	bool isLHS();
	bool isConstant();
	bool isTrue();
	bool isNull();
	bool isBool();
	bool isInt();
	bool isFloat();
	bool isNum();
	bool isString();
	bool asBool();
	crocint asInt();
	crocfloat asFloat();
	crocstr asString();
	int crocType();
};

struct BinaryExp : public Expression
{
	Expression* op1;
	Expression* op2;

	BinaryExp(CompileLoc location, CompileLoc endLocation, AstTag type, Expression* op1, Expression* op2) :
		Expression(location, endLocation, type),
		op1(op1),
		op2(op2)
	{}
};

struct UnExp : public Expression
{
	Expression* op;

	UnExp(CompileLoc location, CompileLoc endLocation, AstTag type, Expression* op) :
		Expression(location, endLocation, type),
		op(op)
	{}
};

struct PostfixExp : public UnExp
{
	PostfixExp(CompileLoc location, CompileLoc endLocation, AstTag type, Expression* op) :
		UnExp(location, endLocation, type, op)
	{}
};

struct PrimaryExp : public Expression
{
	PrimaryExp(CompileLoc location, AstTag type) :
		Expression(location, location, type)
	{}

	PrimaryExp(CompileLoc location, CompileLoc endLocation, AstTag type) :
		Expression(location, endLocation, type)
	{}
};

struct Identifier : public AstNode
{
	crocstr name;

	Identifier(CompileLoc location, crocstr name) :
		AstNode(location, location, AstTag_Identifier),
		name(name)
	{}
};

struct FuncDef : public AstNode
{
	Identifier* name;
	DArray<FuncParam> params;
	bool isVararg;
	Statement* code;

	FuncDef(CompileLoc location, Identifier* name, DArray<FuncParam> params, bool isVararg, Statement* code) :
		AstNode(location, code->endLocation, AstTag_FuncDef),
		name(name),
		params(params),
		isVararg(isVararg),
		code(code)
	{}
};

struct VarDecl : public Statement
{
	Protection protection;
	DArray<Identifier*> names;
	DArray<Expression*> initializer;

	VarDecl(CompileLoc location, CompileLoc endLocation, Protection protection, DArray<Identifier*> names,
		DArray<Expression*> initializer) :
		Statement(location, endLocation, AstTag_VarDecl),
		protection(protection),
		names(names),
		initializer(initializer)
	{}
};

struct Decorator : public AstNode
{
	Expression* func;
	DArray<Expression*> args;
	Decorator* nextDec;

	Decorator(CompileLoc location, CompileLoc endLocation, Expression* func, DArray<Expression*> args,
		Decorator* nextDec) :
		AstNode(location, endLocation, AstTag_Decorator),
		func(func),
		args(args),
		nextDec(nextDec)
	{}
};

struct FuncDecl : public Statement
{
	Protection protection;
	DArray<Identifier*> owner;
	FuncDef* def;
	Decorator* decorator;

	FuncDecl(CompileLoc location, Protection protection, DArray<Identifier*> owner, FuncDef* def, Decorator* decorator) :
		Statement(location, def->endLocation, AstTag_FuncDecl),
		protection(protection),
		owner(owner),
		def(def),
		decorator(decorator)
	{}
};

struct ImportStmt : public Statement
{
	Identifier* importName;
	Expression* expr;
	DArray<Identifier*> symbols;
	DArray<Identifier*> symbolNames;

	ImportStmt(CompileLoc location, CompileLoc endLocation, Identifier* importName, Expression* expr,
		DArray<Identifier*> symbols, DArray<Identifier*> symbolNames) :
		Statement(location, endLocation, AstTag_ImportStmt),
		importName(importName),
		expr(expr),
		symbols(symbols),
		symbolNames(symbolNames)
	{}
};

struct BlockStmt : public Statement
{
	DArray<Statement*> statements;

	BlockStmt(CompileLoc location, CompileLoc endLocation, DArray<Statement*> statements) :
		Statement(location, endLocation, AstTag_BlockStmt),
		statements(statements)
	{}
};

struct ScopeStmt : public Statement
{
	Statement* statement;

	ScopeStmt(Statement* statement) :
		Statement(statement->location, statement->endLocation, AstTag_ScopeStmt),
		statement(statement)
	{}
};

struct ExpressionStmt : public Statement
{
	Expression* expr;

	ExpressionStmt(CompileLoc location, CompileLoc endLocation, Expression* expr) :
		Statement(location, endLocation, AstTag_ExpressionStmt),
		expr(expr)
	{}

	ExpressionStmt(Expression* expr) :
		Statement(expr->location, expr->endLocation, AstTag_ExpressionStmt),
		expr(expr)
	{}
};

struct IfStmt : public Statement
{
	Expression* condition;
	Statement* ifBody;
	Statement* elseBody;

	IfStmt(CompileLoc location, CompileLoc endLocation, Expression* condition, Statement* ifBody, Statement* elseBody) :
		Statement(location, endLocation, AstTag_IfStmt),
		condition(condition),
		ifBody(ifBody),
		elseBody(elseBody)
	{}
};

struct WhileStmt : public Statement
{
	crocstr name;
	Expression* condition;
	Statement* code;

	WhileStmt(CompileLoc location, crocstr name, Expression* condition, Statement* code) :
		Statement(location, code->endLocation, AstTag_WhileStmt),
		name(name),
		condition(condition),
		code(code)
	{}
};

struct DoWhileStmt : public Statement
{
	crocstr name;
	Statement* code;
	Expression* condition;

	DoWhileStmt(CompileLoc location, CompileLoc endLocation, crocstr name, Statement* code, Expression* condition):
		Statement(location, endLocation, AstTag_DoWhileStmt),
		name(name),
		code(code),
		condition(condition)
	{}
};

struct ForStmt : public Statement
{
	crocstr name;
	DArray<ForStmtInit> init;
	Expression* condition;
	DArray<Statement*> increment;
	Statement* code;

	ForStmt(CompileLoc location, crocstr name, DArray<ForStmtInit> init, Expression* cond,
		DArray<Statement*> inc, Statement* code) :
		Statement(location, endLocation, AstTag_ForStmt),
		name(name),
		init(init),
		condition(cond),
		increment(inc),
		code(code)
	{}
};

struct ForNumStmt : public Statement
{
	crocstr name;
	Identifier* index;
	Expression* lo;
	Expression* hi;
	Expression* step;
	Statement* code;

	ForNumStmt(CompileLoc location, crocstr name, Identifier* index, Expression* lo, Expression* hi,
		Expression* step, Statement* code) :
		Statement(location, code->endLocation, AstTag_ForNumStmt),
		name(name),
		index(index),
		lo(lo),
		hi(hi),
		step(step),
		code(code)
	{}
};

struct ForeachStmt : public Statement
{
	crocstr name;
	DArray<Identifier*> indices;
	DArray<Expression*> container;
	Statement* code;

	ForeachStmt(CompileLoc location, crocstr name, DArray<Identifier*> indices, DArray<Expression*> container,
		Statement* code) :
		Statement(location, code->endLocation, AstTag_ForeachStmt),
		name(name),
		indices(indices),
		container(container),
		code(code)
	{}
};

struct ContinueStmt : public Statement
{
	crocstr name;

	ContinueStmt(CompileLoc location, crocstr name) :
		Statement(location, location, AstTag_ContinueStmt),
		name(name)
	{}
};

struct BreakStmt : public Statement
{
	crocstr name;

	BreakStmt(CompileLoc location, crocstr name) :
		Statement(location, location, AstTag_BreakStmt),
		name(name)
	{}
};

struct ReturnStmt : public Statement
{
	DArray<Expression*> exprs;

	ReturnStmt(CompileLoc location, CompileLoc endLocation, DArray<Expression*> exprs) :
		Statement(location, endLocation, AstTag_ReturnStmt),
		exprs(exprs)
	{}
};

struct AssignStmt : public Statement
{
	DArray<Expression*> lhs;
	DArray<Expression*> rhs;

	AssignStmt(CompileLoc location, CompileLoc endLocation, DArray<Expression*> lhs, DArray<Expression*> rhs) :
		Statement(location, endLocation, AstTag_AssignStmt),
		lhs(lhs),
		rhs(rhs)
	{}
};

#define OPASSIGNSTMTCTOR(Tag)\
	Tag(CompileLoc location, CompileLoc endLocation, Expression* lhs, Expression* rhs) :\
		OpAssignStmt(location, endLocation, AstTag_##Tag, lhs, rhs)\
	{}

struct AddAssignStmt  : public OpAssignStmt { OPASSIGNSTMTCTOR(AddAssignStmt)  };
struct SubAssignStmt  : public OpAssignStmt { OPASSIGNSTMTCTOR(SubAssignStmt)  };
struct MulAssignStmt  : public OpAssignStmt { OPASSIGNSTMTCTOR(MulAssignStmt)  };
struct DivAssignStmt  : public OpAssignStmt { OPASSIGNSTMTCTOR(DivAssignStmt)  };
struct ModAssignStmt  : public OpAssignStmt { OPASSIGNSTMTCTOR(ModAssignStmt)  };
struct OrAssignStmt   : public OpAssignStmt { OPASSIGNSTMTCTOR(OrAssignStmt)   };
struct XorAssignStmt  : public OpAssignStmt { OPASSIGNSTMTCTOR(XorAssignStmt)  };
struct AndAssignStmt  : public OpAssignStmt { OPASSIGNSTMTCTOR(AndAssignStmt)  };
struct ShlAssignStmt  : public OpAssignStmt { OPASSIGNSTMTCTOR(ShlAssignStmt)  };
struct ShrAssignStmt  : public OpAssignStmt { OPASSIGNSTMTCTOR(ShrAssignStmt)  };
struct UShrAssignStmt : public OpAssignStmt { OPASSIGNSTMTCTOR(UShrAssignStmt) };
struct CondAssignStmt : public OpAssignStmt { OPASSIGNSTMTCTOR(CondAssignStmt) };

struct IncStmt : public Statement
{
	Expression* exp;

	IncStmt(CompileLoc location, CompileLoc endLocation, Expression* exp) :
		Statement(location, endLocation, AstTag_IncStmt),
		exp(exp)
	{}
};

struct DecStmt : public Statement
{
	Expression* exp;

	DecStmt(CompileLoc location, CompileLoc endLocation, Expression* exp) :
		Statement(location, endLocation, AstTag_DecStmt),
		exp(exp)
	{}
};

struct CondExp : public Expression
{
	Expression* cond;
	Expression* op1;
	Expression* op2;

	CondExp(CompileLoc location, CompileLoc endLocation, Expression* cond, Expression* op1, Expression* op2) :
		Expression(location, endLocation, AstTag_CondExp),
		cond(cond),
		op1(op1),
		op2(op2)
	{}

};

#define BINEXPCTOR(Tag)\
	Tag(CompileLoc location, CompileLoc endLocation, Expression* left, Expression* right) :\
		BinaryExp(location, endLocation, AstTag_##Tag, left, right)\
	{}

struct OrOrExp     : public BinaryExp { BINEXPCTOR(OrOrExp)     };
struct AndAndExp   : public BinaryExp { BINEXPCTOR(AndAndExp)   };

struct OrExp       : public BinaryExp { BINEXPCTOR(OrExp)       };
struct XorExp      : public BinaryExp { BINEXPCTOR(XorExp)      };
struct AndExp      : public BinaryExp { BINEXPCTOR(AndExp)      };
struct ShlExp      : public BinaryExp { BINEXPCTOR(ShlExp)      };
struct ShrExp      : public BinaryExp { BINEXPCTOR(ShrExp)      };
struct UShrExp     : public BinaryExp { BINEXPCTOR(UShrExp)     };
struct AddExp      : public BinaryExp { BINEXPCTOR(AddExp)      };
struct SubExp      : public BinaryExp { BINEXPCTOR(SubExp)      };
struct MulExp      : public BinaryExp { BINEXPCTOR(MulExp)      };
struct DivExp      : public BinaryExp { BINEXPCTOR(DivExp)      };
struct ModExp      : public BinaryExp { BINEXPCTOR(ModExp)      };

struct EqualExp    : public BinaryExp { BINEXPCTOR(EqualExp)    };
struct NotEqualExp : public BinaryExp { BINEXPCTOR(NotEqualExp) };
struct IsExp       : public BinaryExp { BINEXPCTOR(IsExp)       };
struct NotIsExp    : public BinaryExp { BINEXPCTOR(NotIsExp)    };

struct LTExp       : public BinaryExp { BINEXPCTOR(LTExp)       };
struct LEExp       : public BinaryExp { BINEXPCTOR(LEExp)       };
struct GTExp       : public BinaryExp { BINEXPCTOR(GTExp)       };
struct GEExp       : public BinaryExp { BINEXPCTOR(GEExp)       };

struct CatExp      : public BinaryExp { BINEXPCTOR(CatExp)      };

#define UNEXPCTOR(Tag)\
	Tag(CompileLoc location, Expression* op) :\
		UnExp(location, op->endLocation, AstTag_##Tag, op)\
	{}

struct NegExp : public UnExp { UNEXPCTOR(NegExp) };
struct NotExp : public UnExp { UNEXPCTOR(NotExp) };
struct ComExp : public UnExp { UNEXPCTOR(ComExp) };
struct LenExp : public UnExp { UNEXPCTOR(LenExp) };

struct DotExp : public PostfixExp
{
	Expression* name;

	DotExp(Expression* op, Expression* name) :
		PostfixExp(op->location, name->endLocation, AstTag_DotExp, op),
		name(name)
	{}
};

struct IndexExp : public PostfixExp
{
	Expression* index;

	IndexExp(CompileLoc endLocation, Expression* op, Expression* index) :
		PostfixExp(op->location, endLocation, AstTag_IndexExp, op),
		index(index)
	{}
};

struct CallExp : public PostfixExp
{
	DArray<Expression*> args;

	CallExp(CompileLoc endLocation, Expression* op, DArray<Expression*> args) :
		PostfixExp(op->location, endLocation, AstTag_CallExp, op),
		args(args)
	{}
};

struct MethodCallExp : public PostfixExp
{
	Identifier* method;
	DArray<Expression*> args;

	MethodCallExp(CompileLoc location, CompileLoc endLocation, Expression* op, Identifier* method,
		DArray<Expression*> args) :
		PostfixExp(location, endLocation, AstTag_MethodCallExp, op),
		method(method),
		args(args)
	{}
};

struct IdentExp : public PrimaryExp
{
	Identifier* name;

	IdentExp(Identifier* name) :
		PrimaryExp(name->location, AstTag_IdentExp),
		name(name)
	{}
};

struct ThisExp : public PrimaryExp
{
	ThisExp(CompileLoc location) :
		PrimaryExp(location, AstTag_ThisExp)
	{}
};

struct NullExp : public PrimaryExp
{
	NullExp(CompileLoc location) :
		PrimaryExp(location, AstTag_NullExp)
	{}
};

struct BoolExp : public PrimaryExp
{
	bool value;

	BoolExp(CompileLoc location, bool value) :
		PrimaryExp(location, AstTag_BoolExp),
		value(value)
	{}
};

struct VarargExp : public PrimaryExp
{
	VarargExp(CompileLoc location) :
		PrimaryExp(location, AstTag_VarargExp)
	{}
};

struct VargLenExp : public PrimaryExp
{
	VargLenExp(CompileLoc location, CompileLoc endLocation) :
		PrimaryExp(location, endLocation, AstTag_VargLenExp)
	{}
};

struct VargIndexExp : public PrimaryExp
{
	Expression* index;

	VargIndexExp(CompileLoc location, CompileLoc endLocation, Expression* index) :
		PrimaryExp(location, endLocation, AstTag_VargIndexExp),
		index(index)
	{}
};

struct IntExp : public PrimaryExp
{
	crocint value;
	NumFormat format;

	IntExp(CompileLoc location, crocint value, NumFormat format) :
		PrimaryExp(location, AstTag_IntExp),
		value(value),
		format(format)
	{}
};

struct FloatExp : public PrimaryExp
{
	crocfloat value;

	FloatExp(CompileLoc location, crocfloat value) :
		PrimaryExp(location, AstTag_FloatExp),
		value(value)
	{}
};

struct StringExp : public PrimaryExp
{
	crocstr value;

	StringExp(CompileLoc location, crocstr value) :
		PrimaryExp(location, AstTag_StringExp),
		value(value)
	{}
};

struct FuncLiteralExp : public PrimaryExp
{
	FuncDef* def;

	FuncLiteralExp(CompileLoc location, FuncDef* def) :
		PrimaryExp(location, def->endLocation, AstTag_FuncLiteralExp),
		def(def)
	{}
};

struct ParenExp : public PrimaryExp
{
	Expression* exp;

	ParenExp(CompileLoc location, CompileLoc endLocation, Expression* exp) :
		PrimaryExp(location, endLocation, AstTag_ParenExp),
		exp(exp)
	{}
};

struct TableCtorExp : public PrimaryExp
{
	DArray<TableCtorField> fields;

	TableCtorExp(CompileLoc location, CompileLoc endLocation, DArray<TableCtorField> fields) :
		PrimaryExp(location, endLocation, AstTag_TableCtorExp),
		fields(fields)
	{}
};

struct ArrayCtorExp : public PrimaryExp
{
	DArray<Expression*> values;

	ArrayCtorExp(CompileLoc location, CompileLoc endLocation, DArray<Expression*> values) :
		PrimaryExp(location, endLocation, AstTag_ArrayCtorExp),
		values(values)
	{}
};

struct YieldExp : public PrimaryExp
{
	DArray<Expression*> args;

	YieldExp(CompileLoc location, CompileLoc endLocation, DArray<Expression*> args) :
		PrimaryExp(location, endLocation, AstTag_YieldExp),
		args(args)
	{}
};

#endif