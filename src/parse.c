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
	if(levelOfPrecedence == 2) {
		if(peek(P, 0).type == TOKEN_NUMBER) {
			ASTExpressionPrimitive *ret = malloc(sizeof(*ret));
			ret->nodeKind = AST_EXPRESSION_PRIMITIVE;
			
			Token tok = get(P);
			
			const char *str = tok.content;
			int base = 10;
			if(strchr(str, 'r')) {
				base = strtol(str, (char**) &str, 10);
				str++; /* Go past the r. */
			}
			
			ret->numerator = strtol(str, NULL, base);
			ret->denominator = 1;
			
			ret->constantType = ret->numerator == 0 ? EXPRESSION_CONSTANT_FALSY : EXPRESSION_CONSTANT_TRUTHY;
			
			return (AST*) ret;
		} else if(peek(P, 0).type == TOKEN_IDENTIFIER) {
			ASTExpressionVar *ret = malloc(sizeof(*ret));
			ret->nodeKind = AST_EXPRESSION_VAR;
			
			ret->thing = vartable_find(P->scope, get(P).content);
			
			ret->constantType = EXPRESSION_NOT_CONSTANT;
			
			return (AST*) ret;
		}
	} else if(levelOfPrecedence == 1) {
		AST *ret = nct_parse_expression(P, 2);
		
		while(1) {
			BinaryOp op;
			if(maybe(P, TOKEN_STAR)) op = BINOP_MUL;
			else if(maybe(P, TOKEN_SLASH)) op = BINOP_DIV;
			else break;
			
			ASTExpressionBinaryOp *astop = malloc(sizeof(*ret));
			astop->nodeKind = AST_EXPRESSION_BINARY_OP;
			astop->op = op;
			astop->left = ret;
			astop->right = nct_parse_expression(P, 2);
			ret = (AST*) astop;
		}
		
		return ret;
	} else if(levelOfPrecedence == 0) {
		AST *ret = nct_parse_expression(P, 1);
		
		while(1) {
			BinaryOp op;
			if(maybe(P, TOKEN_PLUS)) op = BINOP_ADD;
			else if(maybe(P, TOKEN_MINUS)) op = BINOP_SUB;
			else break;
			
			ASTExpressionBinaryOp *astop = malloc(sizeof(*ret));
			astop->nodeKind = AST_EXPRESSION_BINARY_OP;
			astop->op = op;
			astop->left = ret;
			astop->right = nct_parse_expression(P, 1);
			
			/* Commutative operations have the primitive on the left always */
			//~ if(BINOP_COMMUTATIVE[op] && astop->right->nodeKind == AST_EXPRESSION_PRIMITIVE) {
				//~ AST *temp = astop->left;
				//~ astop->left = astop->right;
				//~ astop->right = temp;
			//~ }
			
			ret = (AST*) astop;
		}
		
		ast_expression_optimize(ret);
		
		return ret;
	}
}

ASTTypename *nct_parse_typename(Parser *P) {
	ASTTypename *ret = malloc(sizeof(*ret));
	ret->nodeKind = AST_TYPENAME;
	
	ret->identifier = expect(P, TOKEN_IDENTIFIER);
	
	return ret;
}

static AST *parse_declaration(Parser *P) {
	int isLocal = maybe(P, TOKEN_LOCAL);
	ASTTypename *typename = nct_parse_typename(P);
	Token name = expect(P, TOKEN_IDENTIFIER);
	
	if(maybe(P, TOKEN_EQUALS)) {
		if(isLocal) { /* Impossible, error. */
			fputs("`local` keyword is to be used for symbol declaration only.\n", stderr);
			abort();
			return NULL;
		}
		
		VarTableEntry *entry = malloc(sizeof(*entry));
		entry->name = name.content;
		entry->type = (Type*) primitive_parse(typename->identifier.content);
		entry->kind = VARTABLEENTRY_VAR;
		vartable_set(P->scope, entry);
		
		ASTStatementVar *ret = malloc(sizeof(*ret));
		ret->nodeKind = AST_STATEMENT_VAR;
		ret->thing = entry;
		ret->expression = nct_parse_expression(P, 0);
		ret->next = NULL;
		
		expect(P, TOKEN_SEMICOLON);
		
		return (AST*) ret;
	} else if(maybe(P, TOKEN_COLON)) {
		ASTStatementSymbol *ret = malloc(sizeof(*ret));
		ret->nodeKind = AST_STATEMENT_SYMBOL;
		ret->isLocal = isLocal;
		ret->typename = typename;
		ret->identifier = name;
		ret->expression = nct_parse_expression(P, 0);
		ret->next = NULL;
		
		VarTableEntry *entry = malloc(sizeof(*entry));
		entry->name = ret->identifier.content;
		entry->type = (Type*) primitive_parse(ret->typename->identifier.content);
		entry->kind = VARTABLEENTRY_SYMBOL;
		entry->data.symbol.linkName = ret->identifier.content;
		vartable_set(P->scope, entry);
		
		expect(P, TOKEN_SEMICOLON);
		
		return (AST*) ret;
	}
	
	abort();
	return NULL;
}

ASTChunk *nct_parse_chunk(Parser*, int);
AST *nct_parse_statement(Parser *P) {
	if(maybe(P, TOKEN_IF)) {
		ASTStatementIf *ret = malloc(sizeof(*ret));
		ret->nodeKind = AST_STATEMENT_IF;
		
		expect(P, TOKEN_PAREN_L);
		ret->expression = nct_parse_expression(P, 0);
		expect(P, TOKEN_PAREN_R);
		
		expect(P, TOKEN_SQUIGGLY_L);
		ret->then = nct_parse_chunk(P, 0);
		expect(P, TOKEN_SQUIGGLY_R);
		
		ret->next = NULL;
		
		return (AST*) ret;
	}
	
	return parse_declaration(P);
}

ASTChunk *nct_parse_chunk(Parser *P, int isTopLevel) {
	ASTChunk *ret = malloc(sizeof(*ret));
	ret->nodeKind = AST_CHUNK;
	
	AST **ptr = &ret->statements;
	while(peek(P, 0).type != (isTopLevel ? TOKEN_EOF : TOKEN_SQUIGGLY_R)) {
		*ptr = nct_parse_statement(P);
		ptr = &(((ASTStatement*) (*ptr))->next); // ok
	}
	
	return ret;
}

ASTChunk *nct_parse(Token *tokens) {
	Parser P = {.tokens = tokens, .i = 0, .scope = vartable_new(NULL)};
	return nct_parse_chunk(&P, 1);
}
