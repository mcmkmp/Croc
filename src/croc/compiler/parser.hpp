#ifndef CROC_COMPILER_PARSER_HPP
#define CROC_COMPILER_PARSER_HPP

#include <functional>

#include "croc/compiler/ast.hpp"
#include "croc/compiler/lexer.hpp"
#include "croc/compiler/types.hpp"
#include "croc/types.hpp"

namespace croc
{
	struct Parser
	{
	private:
		Compiler& c;
		Lexer& l;
		bool mDanglingDoc;
		uword mDummyNameCounter;
		const char* mCurrentClassName;

	public:
		Parser(Compiler& compiler, Lexer& lexer) :
			c(compiler),
			l(lexer),
			mDanglingDoc(false),
			mDummyNameCounter(0),
			mCurrentClassName(nullptr)
		{}

		inline bool danglingDoc()
		{
			return mDanglingDoc;
		}

		const char* capture(std::function<void()> dg);
		const char* parseName();
		Expression* parseDottedName();
		Identifier* parseIdentifier();
		DArray<Expression*> parseArguments();
		Module* parseModule();
		FuncDef* parseStatements(const char* name);
		FuncDef* parseExpressionFunc(const char* name);
		Statement* parseStatement(bool needScope = true);
		Statement* parseExpressionStmt();
		Decorator* parseDecorator();
		Decorator* parseDecorators();
		Statement* parseDeclStmt();
		VarDecl* parseVarDecl();
		FuncDecl* parseFuncDecl(Decorator* deco);
		FuncDef* parseFuncBody(CompileLoc location, Identifier* name);
		DArray<FuncParam> parseFuncParams(bool& isVararg);
		uint32_t parseParamType(DArray<Expression*>& classTypes, const char*& typeString, Expression*& customConstraint);
		FuncDef* parseSimpleFuncDef();
		FuncDef* parseFuncLiteral();
		FuncDef* parseHaskellFuncLiteral();
		ClassDecl* parseClassDecl(Decorator* deco);
		NamespaceDecl* parseNamespaceDecl(Decorator* deco);
		BlockStmt* parseBlockStmt();
		AssertStmt* parseAssertStmt();
		BreakStmt* parseBreakStmt();
		ContinueStmt* parseContinueStmt();
		DoWhileStmt* parseDoWhileStmt();
		Statement* parseForStmt();
		ForeachStmt* parseForeachStmt();
		IfStmt* parseIfStmt();
		ImportStmt* parseImportStmt();
		ReturnStmt* parseReturnStmt();
		SwitchStmt* parseSwitchStmt();
		CaseStmt* parseCaseStmt();
		DefaultStmt* parseDefaultStmt();
		ThrowStmt* parseThrowStmt();
		ScopeActionStmt* parseScopeActionStmt();
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
		ForComprehension* parseForComprehension();
		IfComprehension* parseIfComprehension();
		void propagateFuncLiteralNames(DArray<AstNode*> lhs, DArray<Expression*> rhs);
		void propagateFuncLiteralName(AstNode* lhs, FuncLiteralExp* fl);
		Identifier* dummyForeachIndex(CompileLoc loc);
		Identifier* dummyFuncLiteralName(CompileLoc loc);
		bool isPrivateFieldName(const char* name);
		const char* checkPrivateFieldName(const char* fieldName);
		Expression* decoToExp(Decorator* dec, Expression* exp);

		template<typename T>
		void attachDocs(T& t, const char* preDocs, CompileLoc preDocsLoc)
		{
			if(!c.docComments())
				return;

			if(strlen(preDocs) > 0)
			{
				if(l.tok().postComment != nullptr)
					c.synException(preDocsLoc, "Cannot have two doc comments on one declaration");
				else
				{
					t.docs = preDocs;
					t.docsLoc = preDocsLoc;
				}
			}
			else if(l.tok().postComment != nullptr)
			{
				t.docs = l.tok().postComment;
				t.docsLoc = l.tok().postCommentLoc;
			}
		}
	};
}

#endif