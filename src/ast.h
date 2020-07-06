#ifndef NCTREF_AST_H
#define NCTREF_AST_H

#include"types.h"
#include"lexer.h"
#include"vartable.h"

typedef enum ASTKind {
	AST_COMPILATION_UNIT, AST_STATEMENT_SYMBOL, AST_TYPENAME, AST_EXPRESSION_PRIMITIVE
} ASTKind;

union AST;

typedef struct {
	ASTKind nodeKind;
	TypePrimitive *type;
	int isConstant;
} ASTExpressionBase;

typedef struct {
	ASTExpressionBase;
	
	int numerator;
	int denominator;
} ASTExpressionPrimitive;

typedef struct {
	ASTKind nodeKind;
	
	Token identifier;
} ASTTypename;

typedef struct {
	ASTKind nodeKind;
	
	int isLocal;
	ASTTypename *typename;
	Token identifier;
	
	union AST *expression; /* Must be CT expression. */
	
	union AST *next;
} ASTStatementSymbol;

typedef struct {
	ASTKind nodeKind;
	
	union AST *statements;
} ASTCompilationUnit;

typedef union AST {
	ASTKind nodeKind;
	
	ASTCompilationUnit compilationUnit;
	ASTStatementSymbol statementSymbol;
	ASTExpressionPrimitive expressionPrimitive;
	ASTExpressionBase expressionBase;
} AST;

#endif
