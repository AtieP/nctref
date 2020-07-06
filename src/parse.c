#include"parse.h"

#include<assert.h>
#include<stdlib.h>
#include<string.h>
#include"utils.h"
#include"vartable.h"

typedef struct {
	Token *tokens;
	size_t i;
	
	VarTable *scope;
} Parser;

static Token get(Parser *P) {
	if(P->tokens[P->i].type == TOKEN_EOF) {
		return P->tokens[P->i];
	} else {
		return P->tokens[P->i++];
	}
}

static Token expect(Parser *P, TokenKind t) {
	Token tok = get(P);

	assert(tok.type == t && "Unexpected token");
	
	return tok;
}

static Token peek(Parser *P, int depth) {
	int j = 0;
	for(; j < depth; j++) {
		if(P->tokens[P->i + j].type == TOKEN_EOF) {
			break;
		}
	}
	return P->tokens[P->i + j];
}

static int maybe(Parser *P, TokenKind t) {
	if(peek(P, 0).type == t) {
		get(P);
		return 1;
	}
	return 0;
}

AST *nct_parse_expression(Parser *P, int levelOfPrecedence) {
	ASTExpressionPrimitive *ret = malloc(sizeof(*ret));
	ret->nodeKind = AST_EXPRESSION_PRIMITIVE;
	ret->isConstant = 1;
	
	Token tok = expect(P, TOKEN_NUMBER);
	
	const char *str = tok.content;
	int base = 10;
	if(strchr(str, 'r')) {
		base = strtol(str, (char**) &str, 10);
		str++; /* Go past the r. */
	}
	
	ret->numerator = strtol(str, NULL, base);
	ret->denominator = 1;
	
	return (AST*) ret;
}

ASTTypename *nct_parse_typename(Parser *P) {
	ASTTypename *ret = malloc(sizeof(*ret));
	ret->nodeKind = AST_TYPENAME;
	
	ret->identifier = expect(P, TOKEN_IDENTIFIER);
	
	return ret;
}

AST *nct_parse_statement(Parser *P) {
	ASTStatementSymbol *ret = malloc(sizeof(*ret));
	ret->nodeKind = AST_STATEMENT_SYMBOL;
	
	ret->isLocal = maybe(P, TOKEN_LOCAL);
	
	ret->typename = nct_parse_typename(P);
	
	ret->identifier = expect(P, TOKEN_IDENTIFIER);
	
	expect(P, TOKEN_COLON);
	
	ret->expression = nct_parse_expression(P, 0);
	
	ret->next = NULL;
	
	VarTableEntry *entry = malloc(sizeof(*entry));
	entry->name = ret->identifier.content;
	entry->type = (Type*) primitive_parse(ret->typename->identifier.content);
	entry->kind = VARTABLEENTRY_SYMBOL;
	entry->data.symbol.linkName = ret->identifier.content;
	vartable_set(P->scope, entry);
	
	return (AST*) ret;
}

ASTCompilationUnit *nct_parse(Token *tokens) {
	Parser P = {.tokens = tokens, .i = 0, .scope = vartable_new(NULL)};
	
	ASTCompilationUnit *ret = malloc(sizeof(*ret));
	ret->nodeKind = AST_COMPILATION_UNIT;
	
	ret->statements = nct_parse_statement(&P);
	
	return ret;
}
