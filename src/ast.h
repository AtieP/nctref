#ifndef NCTREF_AST_H
#define NCTREF_AST_H

#include"types.h"
#include"lexer.h"
#include"vartable.h"

typedef enum {
	AST_CHUNK, AST_STATEMENT_DECL, AST_TYPE_IDENTIFIER, AST_EXPRESSION_PRIMITIVE, AST_STATEMENT_IF, AST_EXPRESSION_BINARY_OP, AST_EXPRESSION_VAR, AST_TYPE_POINTER, AST_EXPRESSION_UNARY_OP, AST_STATEMENT_LOOP, AST_STATEMENT_BREAK, AST_EXPRESSION_CALL
} ASTKind;

typedef enum {
	BINOP_ADD = 0, BINOP_SUB = 1, BINOP_MUL = 2, BINOP_DIV = 3
} BinaryOp;
extern int BINOP_COMMUTATIVE[];

typedef enum {
	UNOP_DEREF = 0, UNOP_NEGATE = 1
} UnaryOp;

typedef enum {
	EXPRESSION_CONSTANT_TRUTHY,
	EXPRESSION_CONSTANT_FALSY,
	EXPRESSION_NOT_CONSTANT
} ASTExpressionConstantType;

union AST;

typedef struct {
	ASTKind nodeKind;
	Type *type;
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
	
	UnaryOp operator;
	union AST *chaiuld;
} ASTExpressionUnaryOp;

typedef struct {
	ASTExpression;
	
	VarTableEntry *thing;
} ASTExpressionVar;

typedef struct {
	ASTExpression;
	
	union AST *what;
	union AST **args;
} ASTExpressionCall;

typedef struct {
	ASTKind nodeKind;
	
	size_t size;
} ASTType;

typedef struct {
	ASTType;
	
	Token identifier;
} ASTTypeIdentifier;

typedef struct {
	ASTType;
	
	union AST *child;
	int levels;
} ASTTypePointer;

typedef struct {
	ASTKind nodeKind;
	union AST *next;
} ASTStatement;

typedef struct {
	ASTStatement;
	
	VarTableEntry *thing;
	
	union AST *expression;
} ASTStatementDecl;

typedef struct {
	ASTKind nodeKind;
	
	union AST *statements;
} ASTChunk;

typedef struct {
	ASTStatement;
	
	union AST *expression;
	
	ASTChunk *then;
} ASTStatementIf;

typedef struct {
	ASTStatement;
	
	ASTChunk *body;
} ASTStatementLoop;

typedef struct {
	ASTStatement;
} ASTStatementBreak;

typedef union AST {
	ASTKind nodeKind;
	
	ASTChunk chunk;
	ASTStatement statement;
	ASTStatementDecl statementDecl;
	ASTStatementIf statementIf;
	ASTStatementLoop statementLoop;
	ASTStatementBreak statementBreak;
	ASTExpression expression;
	ASTExpressionPrimitive expressionPrimitive;
	ASTExpressionBinaryOp expressionBinaryOp;
	ASTExpressionUnaryOp expressionUnaryOp;
	ASTExpressionVar expressionVar;
	ASTExpressionCall expressionCall;
} AST;

AST *ast_expression_optimize(AST*);
int ast_expression_equal(AST*, AST*);

#endif
