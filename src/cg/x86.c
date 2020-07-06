#include"x86.h"

#include<stdlib.h>

static char *yasm_typename(int bytes) {
	switch(bytes) {
	case 1:
		return "db";
	case 2:
		return "dw";
	case 4:
		return "dd";
	case 8:
		return "dq";
	}

	abort();
	return NULL;
}

void x86_new(X86 *this) {
	this->text = sdsempty();
}

AST *x86_visit_statement(X86 *this, AST *ast) {
	if(ast->nodeKind == AST_STATEMENT_SYMBOL) {
		if(!ast->statementSymbol.expression->expressionBase.isConstant) {
			fputs("Not a constant expression used in symbol statement.\n", stderr);
			abort();
		}
		
		if(!ast->statementSymbol.isLocal) {
			this->text = sdscatfmt(this->text, "global %s\n", ast->statementSymbol.identifier.content);
		}
		
		size_t typeSize = type_size((Type*) primitive_parse(ast->statementSymbol.typename->identifier.content));
		
		if(ast->statementSymbol.expression) {
			AST *expr = ast->statementSymbol.expression;
			this->text = sdscatfmt(this->text, "%s: %s %i\n", ast->statementSymbol.identifier.content, yasm_typename(typeSize), expr->expressionPrimitive.numerator / expr->expressionPrimitive.denominator);
		} else {
			this->text = sdscatfmt(this->text, "%s: resb %i\n", ast->statementSymbol.identifier.content, type_size((Type*) primitive_parse(ast->statementSymbol.typename->identifier.content)));
		}
		
		return ast->statementSymbol.next;
	}
	
	abort(); /* TODO: better error handling, maybe with setjmp? */
	return NULL;
}

void x86_visit_compilation_unit(X86 *this, ASTCompilationUnit *cunit) {
	AST *stmt = cunit->statements;
	while(stmt) {
		stmt = x86_visit_statement(this, stmt);
	}
}

void x86_finish(X86 *this) {
	fprintf(stdout, "section .text\n%.*s", sdslen(this->text), this->text);
}
