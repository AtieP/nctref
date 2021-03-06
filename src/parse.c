#include"parse.h"

#include<assert.h>
#include<stdlib.h>
#include<string.h>
#include"utils.h"
#include"vartable.h"
#include"reporting.h"

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

AST *nct_parse_expression(Parser *P, int lOP) {
	if(lOP == 5) {
		if(peek(P, 0).type == TOKEN_NUMBER) {
			ASTExpressionPrimitive *ret = malloc(sizeof(*ret));
			ret->nodeKind = AST_EXPRESSION_PRIMITIVE;
			ret->type = (Type*) primitive_parse("s16");
			
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
			
			ret->type = ret->thing->type;
			
			ret->constantType = EXPRESSION_NOT_CONSTANT;
			
			return (AST*) ret;
		}
	} else if(lOP == 4) {
		if(maybe(P, TOKEN_STAR)) {
			ASTExpressionUnaryOp *astop = malloc(sizeof(*astop));
			astop->nodeKind = AST_EXPRESSION_UNARY_OP;
			astop->constantType = EXPRESSION_NOT_CONSTANT;
			astop->operator = UNOP_DEREF;
			astop->chaiuld = nct_parse_expression(P, lOP); /* Not +1! */
			astop->type = astop->chaiuld->expression.type->pointer.of;
			
			if(astop->chaiuld->nodeKind == AST_EXPRESSION_VAR) {
				astop->chaiuld->expressionVar.thing->isDereferenced = 1;
			}
			
			return (AST*) astop;
		} else if(maybe(P, TOKEN_MINUS)) {
			AST *chaiuld = nct_parse_expression(P, lOP);
			
			if(chaiuld->nodeKind == AST_EXPRESSION_PRIMITIVE) {
				chaiuld->expressionPrimitive.numerator *= -1;
				return chaiuld;
			} else {
				ASTExpressionUnaryOp *astop = malloc(sizeof(*astop));
				astop->nodeKind = AST_EXPRESSION_UNARY_OP;
				astop->constantType = EXPRESSION_NOT_CONSTANT;
				astop->operator = UNOP_NEGATE;
				astop->chaiuld = chaiuld;
				astop->type = chaiuld->expression.type;

				return (AST*) astop;
			}
		} else if(maybe(P, TOKEN_TILDE)) {
			AST *child = nct_parse_expression(P, lOP);

			if(child->nodeKind == AST_EXPRESSION_PRIMITIVE) {
				child->expressionPrimitive.numerator = \
				    ~child->expressionPrimitive.numerator;
				return child;
			} else {
				ASTExpressionUnaryOp *astop = malloc(sizeof(*astop));
				astop->nodeKind = AST_EXPRESSION_UNARY_OP;
				astop->constantType = EXPRESSION_NOT_CONSTANT;
				astop->operator = UNOP_BITWISE_NOT;
				astop->chaiuld = child;
				astop->type = child->expression.type;

				return (AST *) astop;
			}
		} else return nct_parse_expression(P, lOP + 1);
	} else if(lOP == 3) {
		AST *ret = nct_parse_expression(P, lOP + 1);

		while(maybe(P, TOKEN_PAREN_L)) {
			if(ret->expression.type->type != TYPE_TYPE_FUNCTION) {
				stahp(P->tokens[P->i].row, P->tokens[P->i].column, "Only function types may be called.");
			}
			
			ASTExpressionCall *call = malloc(sizeof(*ret));
			call->nodeKind = AST_EXPRESSION_CALL;
			call->constantType = EXPRESSION_NOT_CONSTANT;
			call->type = ret->expression.type->function.ret;
			call->what = ret;
			call->args = NULL;
			ret = (AST*) call;
			
			int argCount = 0;
			
			if(!maybe(P, TOKEN_PAREN_R)) {
				while(peek(P, 0).type != TOKEN_PAREN_R && peek(P, 0).type != TOKEN_COMMA) {
					call->args = realloc(call->args, (argCount + 1) * sizeof(AST*));
					call->args[argCount] = nct_parse_expression(P, 0);
					
					argCount++;
					
					if(maybe(P, TOKEN_PAREN_R)) {
						break;
					} else expect(P, TOKEN_COMMA);
				}
			}
			
			/* TODO: Check argument count. */
		}
		
		return ret;
	} else if(lOP == 2) {
		AST *ret = nct_parse_expression(P, lOP + 1);
		
		if(peek(P, 0).type == TOKEN_STAR || peek(P, 0).type == TOKEN_SLASH) {
			ASTExpressionBinaryOp *astop = malloc(sizeof(*astop));
			astop->nodeKind = AST_EXPRESSION_BINARY_OP;
			astop->constantType = EXPRESSION_NOT_CONSTANT;
			astop->amountOfOperands = 1;
			
			size_t capacity = 2;
			astop->operands = malloc(sizeof(*astop->operands) * capacity);
			astop->operators = malloc(sizeof(*astop->operators) * (capacity - 1));
			
			astop->operands[0] = ret;
			
			while(1) {
				BinaryOp op;
				if(maybe(P, TOKEN_STAR)) op = BINOP_MUL;
				else if(maybe(P, TOKEN_SLASH)) op = BINOP_DIV;
				else break;
				
				if(astop->amountOfOperands == capacity) {
					capacity *= 2;
					
					astop->operands = realloc(astop->operands, sizeof(*astop->operands) * capacity);
					astop->operators = realloc(astop->operators, sizeof(*astop->operators) * (capacity - 1));
				}
				
				astop->operators[astop->amountOfOperands - 1] = op;
				ASTExpression *operand = &(astop->operands[astop->amountOfOperands++] = nct_parse_expression(P, lOP + 1))->expression;
				
				if(operand->type->type != TYPE_TYPE_PRIMITIVE) {
					stahp(P->tokens[P->i].row, P->tokens[P->i].column, "Invalid combination of operator and operand types.");
  				}
  				
  				if(!astop->type) {
					astop->type = operand->type;
				} else {
					if(type_size(operand->type) > type_size(astop->type)) {
						astop->type = operand->type;
					}
				}
			}
			
			ret = (AST*) astop;
		}
		
		return ret;
	} else if(lOP == 1) {
		AST *ret = nct_parse_expression(P, lOP + 1);
		
		if(
			peek(P, 0).type == TOKEN_PLUS
			|| peek(P, 0).type == TOKEN_MINUS
			|| peek(P, 0).type == TOKEN_AMPERSAND
			|| peek(P, 0).type == TOKEN_VERTICAL_BAR
			|| peek(P, 0).type == TOKEN_CARET
		) {
			ASTExpressionBinaryOp *astop = malloc(sizeof(*astop));
			astop->nodeKind = AST_EXPRESSION_BINARY_OP;
			astop->constantType = EXPRESSION_NOT_CONSTANT;
			astop->amountOfOperands = 1;
			astop->type = NULL;
			
			size_t capacity = 2;
			astop->operands = malloc(sizeof(*astop->operands) * capacity);
			astop->operators = malloc(sizeof(*astop->operators) * (capacity - 1));
			
			astop->operands[0] = ret;
			
			while(1) {
				BinaryOp op;
				if(maybe(P, TOKEN_PLUS)) op = BINOP_ADD;
				else if(maybe(P, TOKEN_MINUS)) op = BINOP_SUB;
				else if(maybe(P, TOKEN_AMPERSAND)) op = BINOP_BITWISE_AND;
				else if(maybe(P, TOKEN_VERTICAL_BAR)) op = BINOP_BITWISE_OR;
				else if(maybe(P, TOKEN_CARET)) op = BINOP_BITWISE_XOR;
				else break;
				
				if(astop->amountOfOperands == capacity) {
					capacity *= 2;
					
					astop->operands = realloc(astop->operands, sizeof(*astop->operands) * capacity);
					astop->operators = realloc(astop->operators, sizeof(*astop->operators) * (capacity - 1));
				}
				
				astop->operators[astop->amountOfOperands - 1] = op;
				ASTExpression *operand = &(astop->operands[astop->amountOfOperands++] = nct_parse_expression(P, lOP + 1))->expression;
				
				if(operand->type->type != TYPE_TYPE_PRIMITIVE) {
					stahp(P->tokens[P->i].row, P->tokens[P->i].column, "Invalid combination of operator and operand types.");
  				}
  				
  				if(!astop->type) {
					astop->type = operand->type;
				} else {
					if(type_size(operand->type) > type_size(astop->type)) {
						astop->type = operand->type;
					}
				}
			}
			
			ret = (AST*) astop;
		}
		
		return ret;
	} else if(lOP == 0) {
		AST *ret = nct_parse_expression(P, lOP + 1);
		
		if(peek(P, 0).type == TOKEN_DOUBLE_EQUALS) {
			ASTExpressionBinaryOp *astop = malloc(sizeof(*astop));
			astop->nodeKind = AST_EXPRESSION_BINARY_OP;
			astop->constantType = EXPRESSION_NOT_CONSTANT;
			astop->amountOfOperands = 1;
			astop->type = NULL;
			
			size_t capacity = 2;
			astop->operands = malloc(sizeof(*astop->operands) * capacity);
			astop->operators = malloc(sizeof(*astop->operators) * (capacity - 1));
			
			astop->operands[0] = ret;
			
			while(1) {
				BinaryOp op;
				if(maybe(P, TOKEN_DOUBLE_EQUALS)) op = BINOP_EQUAL;
				else break;
				
				if(astop->amountOfOperands == capacity) {
					capacity *= 2;
					
					astop->operands = realloc(astop->operands, sizeof(*astop->operands) * capacity);
					astop->operators = realloc(astop->operators, sizeof(*astop->operators) * (capacity - 1));
				}
				
				astop->operators[astop->amountOfOperands - 1] = op;
				ASTExpression *operand = &(astop->operands[astop->amountOfOperands++] = nct_parse_expression(P, lOP + 1))->expression;
				
				if(operand->type->type != TYPE_TYPE_PRIMITIVE) {
					stahp(P->tokens[P->i].row, P->tokens[P->i].column, "Invalid combination of operator and operand types.");
  				}
  				
  				if(!astop->type) {
					astop->type = operand->type;
				} else {
					if(type_size(operand->type) > type_size(astop->type)) {
						astop->type = operand->type;
					}
				}
			}
		}
		
		ret = ast_expression_optimize(ret);
		
		return ret;
	}
#ifdef DEBUG
	else abort();
#endif
	
	return NULL;
}

/* Since this function backtracks, don't use aborting functions like expect. */
Type *nct_parse_typename(Parser *P) {
	int oldIdx = P->i;
	
	if(peek(P, 0).type != TOKEN_IDENTIFIER) {
		goto backtrack;
	}
	
	Type *ret = (Type*) primitive_parse(expect(P, TOKEN_IDENTIFIER).content);
	
	while(peek(P, 0).type == TOKEN_PAREN_L || peek(P, 0).type == TOKEN_STAR) {
		if(maybe(P, TOKEN_STAR)) {
			TypePointer *ptr = malloc(sizeof(*ptr));
			ptr->type = TYPE_TYPE_POINTER;
			ptr->of = ret;
			
			ret = (Type*) ptr;
		} else if(maybe(P, TOKEN_PAREN_L)) {
			TypeFunction *fun = malloc(sizeof(*fun));
			fun->type = TYPE_TYPE_FUNCTION;
			fun->ret = ret;
			fun->argCount = 0;
			fun->args = malloc(0);
			
			if(!maybe(P, TOKEN_PAREN_R)) {
				while(1) {
					fun->argCount++;
					fun->args = realloc(fun->args, sizeof(Type*) * fun->argCount);
					if((fun->args[fun->argCount - 1] = nct_parse_typename(P)) == NULL) {
						free(fun);
						goto backtrack;
					}
					
					if(maybe(P, TOKEN_PAREN_R)) {
						break;
					} else expect(P, TOKEN_COMMA);
				}
			}
			
			ret = (Type*) fun;
		}
	}
	
	return ret;
backtrack:
	P->i = oldIdx;
	return NULL;
}

/* Potentially backtracking. Returns NULL upon failure. */
static AST *parse_declaration(Parser *P) {
	int oldIdx = P->i;
	
	int isLocal = maybe(P, TOKEN_LOCAL);
	int isExternal = 0;
	if(!isLocal) {
		isExternal = maybe(P, TOKEN_EXTERN);
	}
	
	Type *type = nct_parse_typename(P);
	
	if(!type) goto backtrack;
	if(peek(P, 0).type != TOKEN_IDENTIFIER) goto backtrack;
	
	Token name = expect(P, TOKEN_IDENTIFIER);
		
	VarTableEntry *entry = malloc(sizeof(*entry));
	entry->name = name.content;
	entry->type = type;
	entry->isDereferenced = 0;
	vartable_set(P->scope, entry);
	
	ASTStatementDecl *ret = malloc(sizeof(*ret));
	ret->nodeKind = AST_STATEMENT_DECL;
	ret->thing = entry;
	ret->next = NULL;
	
	if(maybe(P, TOKEN_EQUALS)) {
		if(isLocal || isExternal) { /* Impossible, error. */
			fputs("'local' and 'extern' keywords are to be used for symbol declaration only.\n", stderr);
			abort();
			return NULL;
		}
		
		entry->kind = VARTABLEENTRY_VAR;
		
		ret->expression = nct_parse_expression(P, 0);
	} else if(maybe(P, TOKEN_COLON)) {
		if(isExternal) {
			fputs("External symbols may not be defined.\n", stderr);
			abort();
			return NULL;
		}
		
		entry->kind = VARTABLEENTRY_SYMBOL;
		entry->data.symbol.isLocal = isLocal;
		entry->data.symbol.isExternal = isExternal;
		entry->data.symbol.linkName = name.content;
		
		ret->expression = nct_parse_expression(P, 0);
	} else if(isExternal) {
		entry->kind = VARTABLEENTRY_SYMBOL;
		entry->data.symbol.isLocal = isLocal;
		entry->data.symbol.isExternal = isExternal;
		entry->data.symbol.linkName = name.content;
	}
	
	expect(P, TOKEN_SEMICOLON);
	
	return (AST*) ret;
backtrack:
	P->i = oldIdx;
	return NULL;
}

ASTChunk *nct_parse_chunk(Parser*, int);
AST *nct_parse_statement(Parser *P) {
	if(maybe(P, TOKEN_IF)) {
		ASTStatementIf *ret = malloc(sizeof(*ret));
		ret->nodeKind = AST_STATEMENT_IF;
		ret->next = NULL;
		
		expect(P, TOKEN_PAREN_L);
		ret->expression = nct_parse_expression(P, 0);
		expect(P, TOKEN_PAREN_R);
		
		expect(P, TOKEN_SQUIGGLY_L);
		ret->then = nct_parse_chunk(P, 0);
		expect(P, TOKEN_SQUIGGLY_R);
		
		return (AST*) ret;
	} else if(maybe(P, TOKEN_LOOP)) {
		ASTStatementLoop *ret = malloc(sizeof(*ret));
		ret->nodeKind = AST_STATEMENT_LOOP;
		ret->next = NULL;
		
		expect(P, TOKEN_SQUIGGLY_L);
		ret->body = nct_parse_chunk(P, 0);
		expect(P, TOKEN_SQUIGGLY_R);
		
		return (AST*) ret;
	} else if(maybe(P, TOKEN_BREAK)) {
		ASTStatementLoop *ret = malloc(sizeof(*ret));
		ret->nodeKind = AST_STATEMENT_BREAK;
		ret->next = NULL;
		
		expect(P, TOKEN_SEMICOLON);
		
		return (AST*) ret;
	}
	
	{
		AST *decl = parse_declaration(P);
		if(decl) {
			return decl;
		}
	}
	
	ASTStatementExpr *ret = malloc(sizeof(*ret));
	ret->nodeKind = AST_STATEMENT_EXPR;
	ret->next = NULL;
	ret->expr = nct_parse_expression(P, 0);
	expect(P, TOKEN_SEMICOLON);
	
	return (AST*) ret;
}

ASTChunk *nct_parse_chunk(Parser *P, int isTopLevel) {
	P->scope = vartable_new(P->scope);
	
	ASTChunk *ret = malloc(sizeof(*ret));
	ret->nodeKind = AST_CHUNK;
	ret->statements = NULL;
	ret->scope = P->scope;
	
	AST **ptr = &ret->statements;
	
	while(peek(P, 0).type != (isTopLevel ? TOKEN_EOF : TOKEN_SQUIGGLY_R)) {
		*ptr = nct_parse_statement(P);
		ptr = &(((ASTStatement*) (*ptr))->next); // ok
	}
	
	P->scope = P->scope->parent;
	
	return ret;
}

ASTChunk *nct_parse(Token *tokens) {
	Parser P;
	P.tokens = tokens;
	P.i = 0;
	P.scope = NULL;
	return nct_parse_chunk(&P, 1);
}
