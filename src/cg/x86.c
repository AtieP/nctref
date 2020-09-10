#include"x86.h"

#include<stdlib.h>

#define SYNTAX_GAS

static uint8_t register_size(int i) {
	return 1 << (i / 5);
}

static int cast_register(int i, uint8_t size) {
	return (__builtin_ctz(size) * 5) + (i % 5);
}

static RegisterAllocator *create_ralloc(X86 *X) {
	RegisterAllocator *a = malloc(sizeof(*a) + sizeof(Register) * 15);
	a->registersCount = 15;
	a->registers[0] = (Register) {.name = "al", .state = REGISTER_FREE, .size = 1, .aliasBitmap = 1057};
	a->registers[1] = (Register) {.name = "bl", .state = REGISTER_FREE, .size = 1, .aliasBitmap = 2114};
	a->registers[2] = (Register) {.name = "cl", .state = REGISTER_FREE, .size = 1, .aliasBitmap = 4228};
	a->registers[3] = (Register) {.name = "dl", .state = REGISTER_FREE, .size = 1, .aliasBitmap = 8456};
	a->registers[4] = (Register) {.name = "sil", .state = X->mode == X86_MODE_64 ? REGISTER_FREE : REGISTER_DOESNT_EXIST, .size = 1, .aliasBitmap = 16912}; /* Is sil also available in 32-bit long mode? */
	a->registers[5] = (Register) {.name = "ax", .state = REGISTER_FREE, .size = 2, .aliasBitmap = 1057};
	a->registers[6] = (Register) {.name = "bx", .state = REGISTER_FREE, .size = 2, .aliasBitmap = 2114};
	a->registers[7] = (Register) {.name = "cx", .state = REGISTER_FREE, .size = 2, .aliasBitmap = 4228};
	a->registers[8] = (Register) {.name = "dx", .state = REGISTER_FREE, .size = 2, .aliasBitmap = 8456};
	a->registers[9] = (Register) {.name = "si", .state = REGISTER_FREE, .size = 2, .aliasBitmap = 16912};
	a->registers[10] = (Register) {.name = "eax", .state = REGISTER_FREE, .size = 4, .aliasBitmap = 1057};
	a->registers[11] = (Register) {.name = "ebx", .state = REGISTER_FREE, .size = 4, .aliasBitmap = 2114};
	a->registers[12] = (Register) {.name = "ecx", .state = REGISTER_FREE, .size = 4, .aliasBitmap = 4228};
	a->registers[13] = (Register) {.name = "edx", .state = REGISTER_FREE, .size = 4, .aliasBitmap = 8456};
	a->registers[14] = (Register) {.name = "esi", .state = REGISTER_FREE, .size = 4, .aliasBitmap = 16912};
	return a;
}

static char *yasm_typename(int bytes) {
	switch(bytes) {
#ifdef SYNTAX_GAS
	case 1:
		return "byte";
	case 2:
		return "word";
	case 4:
		return "long";
	case 8:
		return "quad";
#else
	case 1:
		return "db";
	case 2:
		return "dw";
	case 4:
		return "dd";
	case 8:
		return "dq";
#endif
	}

	abort();
	return NULL;
}

static void gmovi(X86 *X, const char *dst, size_t val) {
#ifdef SYNTAX_GAS
	X->text = sdscatfmt(X->text, "mov $%i, %%%s\n", val, dst);
#else
	X->text = sdscatfmt(X->text, "mov $%i, %%%s\n", val, dst);
#endif
}

static void gmovr(X86 *X, int zx, const char *dst, const char *src) {
#ifdef SYNTAX_GAS
	X->text = sdscatfmt(X->text, "mov%s %%%s, %%%s\n", zx ? "zx" : "", src, dst);
#else
	X->text = sdscatfmt(X->text, "mov%s %s, %s\n", zx ? "zx" : "", dst, src);
#endif
}

static void gaddi(X86 *X, const char *dst, size_t val) {
#ifdef SYNTAX_GAS
	X->text = sdscatfmt(X->text, "add $%i, %%%s\n", val, dst);
#else
	X->text = sdscatfmt(X->text, "add %s, %i\n", dst, val);
#endif
}

static void gaddr(X86 *X, const char *dst, const char *src) {
#ifdef SYNTAX_GAS
	X->text = sdscatfmt(X->text, "add %%%s, %%%s\n", src, dst);
#else
	X->text = sdscatfmt(X->text, "add %s, %s\n", dst, src);
#endif
}

static void gtest1(X86 *X, const char *reg) {
#ifdef SYNTAX_GAS
	X->text = sdscatfmt(X->text, "test %%%s, %%%s\n", reg, reg);
#else
	X->text = sdscatfmt(X->text, "test %s, %s\n", reg, reg);
#endif
}

void x86_new(X86 *X) {
	X->text = sdsempty();
	X->lidx = 0;
	X->stackDepth = 0;
	
	X->target = X86_TARGET_80386;
	X->mode = X86_MODE_32;
	
	X->rallocator = create_ralloc(X);
}

int x86_visit_expression(X86 *X, AST *ast, size_t coerceToSize) {
	if(coerceToSize == -1) coerceToSize = 4;

	if(ast->nodeKind == AST_EXPRESSION_PRIMITIVE) {
		int dst = ralloc_alloc(X->rallocator, coerceToSize);

		gmovi(X, X->rallocator->registers[dst].name, ast->expressionPrimitive.numerator / ast->expressionPrimitive.denominator);

		return dst;
	} else if(ast->nodeKind == AST_EXPRESSION_VAR) {
		int dst = ralloc_alloc(X->rallocator, coerceToSize);
		int src = ((X86VarEntryInfo*) ast->expressionVar.thing->userdata)->id;
		
		// fixes "mov al, bx"
		if(register_size(src) > register_size(dst)) src = cast_register(src, register_size(dst));
		
		gmovr(X, register_size(dst) > register_size(src), X->rallocator->registers[dst].name, X->rallocator->registers[src].name);

		return dst;
	} else if(ast->nodeKind == AST_EXPRESSION_BINARY_OP) {
		int dst = x86_visit_expression(X, ast->expressionBinaryOp.operands[0], coerceToSize);
		
		for(size_t i = 1; i < ast->expressionBinaryOp.amountOfOperands; i++) {
			AST *operand = ast->expressionBinaryOp.operands[i];
			if(operand->nodeKind == AST_EXPRESSION_PRIMITIVE) {
				gaddi(X, X->rallocator->registers[dst].name, operand->expressionPrimitive.numerator / operand->expressionPrimitive.denominator);
			} else if(operand->nodeKind == AST_EXPRESSION_VAR) {
				int addend = ((X86VarEntryInfo*) operand->expressionVar.thing->userdata)->id;

				gaddr(X, X->rallocator->registers[dst].name, X->rallocator->registers[cast_register(addend, register_size(dst))].name);
			}
		}
		
		return dst;
	}

	abort();
	return -1;
}

AST *x86_visit_statement(X86 *X, AST *ast) {
	if(ast->nodeKind == AST_STATEMENT_DECL) {
		VarTableEntry *ent = ast->statementDecl.thing;
		if(ent->kind == VARTABLEENTRY_SYMBOL) {
			if(ast->statementDecl.expression->expression.constantType == EXPRESSION_NOT_CONSTANT) {
				fputs("Symbol declaration may contain constant expressions only.\n", stderr);
				abort();
			}
			
			if(!ent->data.symbol.isLocal) {
				X->text = sdscatfmt(X->text, ".global %s\n", ent->name);
			}
			
			size_t typeSize = type_size(ent->type);
			
			if(ast->statementDecl.expression) {
				AST *expr = ast->statementDecl.expression;
				X->text = sdscatfmt(X->text, "%s: .%s %i\n", ent->data.symbol.linkName, yasm_typename(typeSize), expr->expressionPrimitive.numerator / expr->expressionPrimitive.denominator);
			} else {
#ifdef SYNTAX_GAS
				X->text = sdscatfmt(X->text, "%s: .skip %i\n", ent->data.symbol.linkName, typeSize);
#else
				X->text = sdscatfmt(X->text, "%s: resb %i\n", ent->data.symbol.linkName, typeSize);
#endif
			}
		} else if(ent->kind == VARTABLEENTRY_VAR) {
			X86VarEntryInfo *info = ent->userdata = malloc(sizeof(*info));
			info->isInRegister = 1;
			info->id = x86_visit_expression(X, ast->statementDecl.expression, ent->type->primitive.width / 8);
		}
	} else if(ast->nodeKind == AST_STATEMENT_IF) {
		switch(ast->statementIf.expression->expression.constantType) {
		case EXPRESSION_NOT_CONSTANT: {
			const char *reg = X->rallocator->registers[x86_visit_expression(X, ast->statementIf.expression, 4)].name;

			gtest1(X, reg);
#ifdef SYNTAX_GAS
			X->text = sdscatfmt(X->text, "jz $.L%i\n", X->lidx);
#else
			X->text = sdscatfmt(X->text, "jz .L%i\n", X->lidx);
#endif

			x86_visit_chunk(X, ast->statementIf.then);

			X->text = sdscatfmt(X->text, ".L%i:", X->lidx++);
			break;
		}
		case EXPRESSION_CONSTANT_TRUTHY: {
			x86_visit_chunk(X, ast->statementIf.then);
			break;
		}
		default:
			break;
		}
	} else {
		abort(); /* TODO: better error handling, maybe with setjmp? */
		return NULL;
	}
	
	return ast->statement.next;
}

void x86_visit_chunk(X86 *X, ASTChunk *chunk) {
	AST *stmt = chunk->statements;
	while(stmt) {
		stmt = x86_visit_statement(X, stmt);
	}
}

void x86_finish(X86 *X) {
#ifdef SYNTAX_GAS
	fprintf(stdout, ".section .text\n%.*s", sdslen(X->text), X->text);
#else
	fprintf(stdout, "section .text\n%.*s", sdslen(X->text), X->text);
#endif
}
