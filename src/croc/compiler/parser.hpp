#ifndef CROC_COMPILER_PARSER_HPP
#define CROC_COMPILER_PARSER_HPP

#include <functional>

#include "croc/compiler/ast.hpp"
#include "croc/compiler/lexer.hpp"
#include "croc/compiler/types.hpp"

struct Parser
{
private:
	Compiler& c;
	Lexer& l;
	uword mDummyNameCounter;

public:
	Parser(Compiler& compiler, Lexer& lexer) :
		c(compiler),
		l(lexer),
		mDummyNameCounter(0)
	{}

	crocstr capture(std::function<void()> dg);
	crocstr parseName();
	Expression* parseDottedName();
	Identifier* parseIdentifier();
	DArray<Expression*> parseArguments();
	BlockStmt* parseModule();
	Statement* parseStatement(bool needScope = true);
	Statement* parseExpressionStmt();
	Decorator* parseDecorator();
	Decorator* parseDecorators();
	Statement* parseDeclStmt();
	VarDecl* parseVarDecl();
	FuncDecl* parseFuncDecl(Decorator* deco);
	FuncDef* parseFuncBody(CompileLoc location, Identifier* name);
	DArray<FuncParam> parseFuncParams(bool& isVararg);
	uint32_t parseType(const char* kind, DArray<Expression*>& classTypes, crocstr& typeString, Expression*& customConstraint);
	uint32_t parseParamType(DArray<Expression*>& classTypes, crocstr& typeString, Expression*& customConstraint);
	uint32_t parseReturnType(DArray<Expression*>& classTypes, crocstr& typeString, Expression*& customConstraint);
	FuncDef* parseSimpleFuncDef();
	FuncDef* parseFuncLiteral();
	FuncDef* parseHaskellFuncLiteral();
	BlockStmt* parseBlockStmt();
	BreakStmt* parseBreakStmt();
	ContinueStmt* parseContinueStmt();
	DoWhileStmt* parseDoWhileStmt();
	Statement* parseForStmt();
	ForeachStmt* parseForeachStmt();
	IfStmt* parseIfStmt();
	ImportStmt* parseImportStmt();
	ReturnStmt* parseReturnStmt();
	Statement* parseTryStmt();
	WhileStmt* parseWhileStmt();
	Statement* parseStatementExpr();
	AssignStmt* parseAssignStmt(Expression* firstLHS);
	Statement* parseOpAssignStmt(Expression* exp1);
	Expression* parseExpression();
	Expression* parseCondExp(Expression* exp1 = nullptr);
	Expression* parseLogicalCondExp();
	Expression* parseOrOrExp(Expression* exp1 = nullptr);
	Expression* parseAndAndExp(Expression* exp1 = nullptr);
	Expression* parseOrExp();
	Expression* parseXorExp();
	Expression* parseAndExp();
	Expression* parseCmpExp();
	Expression* parseShiftExp();
	Expression* parseAddExp();
	Expression* parseMulExp();
	Expression* parseUnExp();
	Expression* parsePrimaryExp();
	IdentExp* parseIdentExp();
	ThisExp* parseThisExp();
	NullExp* parseNullExp();
	BoolExp* parseBoolExp();
	VarargExp* parseVarargExp();
	IntExp* parseIntExp();
	FloatExp* parseFloatExp();
	StringExp* parseStringExp();
	FuncLiteralExp* parseFuncLiteralExp();
	FuncLiteralExp* parseHaskellFuncLiteralExp();
	Expression* parseParenExp();
	Expression* parseTableCtorExp();
	PrimaryExp* parseArrayCtorExp();
	YieldExp* parseYieldExp();
	Expression* parseMemberExp();
	Expression* parsePostfixExp(Expression* exp);
	void propagateFuncLiteralNames(DArray<AstNode*> lhs, DArray<Expression*> rhs);
	void propagateFuncLiteralName(AstNode* lhs, FuncLiteralExp* fl);
	Identifier* dummyForeachIndex(CompileLoc loc);
	Identifier* dummyFuncLiteralName(CompileLoc loc);
	Expression* decoToExp(Decorator* dec, Expression* exp);
};

#endif