#ifndef NCTREF_AST_H
#define NCTREF_AST_H

#include"types.h"
#include"lexer.h"
#include"vartable.h"

typedef enum {
	AST_CHUNK, AST_STATEMENT_SYMBOL, AST_TYPENAME, AST_EXPRESSION_PRIMITIVE, AST_STATEMENT_IF, AST_STATEMENT_VAR, AST_EXPRESSION_BINARY_OP, AST_EXPRESSION_VAR
} ASTKind;

typedef enum {
	BINOP_ADD = 0, BINOP_SUB = 1, BINOP_MUL = 2, BINOP_DIV = 3
} BinaryOp;
extern int BINOP_COMMUTATIVE[];

typedef enum {
	EXPRESSION_CONSTANT_TRUTHY,
	EXPRESSION_CONSTANT_FALSY,
	EXPRESSION_NOT_CONSTANT
} ASTExpressionConstantType;

union AST;

typedef struct {
	ASTKind nodeKind;
	TypePrimitive *type;
	ASTExpressionConstantType constantType;
} ASTExpression;

typedef struct {
	ASTExpression;
	
	int numerator;
	int denominator;
} ASTExpressionPrimitive;

/* This node has no precedence, the parser handles that. */
typedef struct {
	ASTExpression;
	
	size_t amountOfOperands;
	union AST **operands;
	BinaryOp *operators;
} ASTExpressionBinaryOp;

typedef struct {
	ASTExpression;
	
	VarTableEntry *thing;
} ASTExpressionVar;

typedef struct {
	ASTKind nodeKind;
	
	Token identifier;
} ASTTypename;

typedef struct {
	ASTKind nodeKind;
	union AST *next;
} ASTStatement;

typedef struct {
	ASTStatement;
	
	int isLocal;
	ASTTypename *typename;
	Token identifier;
	
	union AST *expression; /* Must be CT expression. */
} ASTStatementSymbol;

typedef struct {
	ASTStatement;
	
	VarTableEntry *thing;
	
	union AST *expression;
} ASTStatementVar;

typedef struct {
	ASTKind nodeKind;
	
	union AST *statements;
} ASTChunk;

typedef struct {
	ASTStatement;
	
	union AST *expression;
	
	ASTChunk *then;
} ASTStatementIf;

typedef union AST {
	ASTKind nodeKind;
	
	ASTChunk chunk;
	ASTStatement statement;
	ASTStatementSymbol statementSymbol;
	ASTStatementVar statementVar;
	ASTStatementIf statementIf;
	ASTExpression expression;
	ASTExpressionPrimitive expressionPrimitive;
	ASTExpressionBinaryOp expressionBinaryOp;
	ASTExpressionVar expressionVar;
} AST;

AST *ast_expression_optimize(AST*);

#endif
