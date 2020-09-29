#include"x86.h"

#include<stdlib.h>
#include<string.h>


#ifndef __GNUC__
int __builtin_ctz(uint8_t i) {
	if(i & 1) return 0;
	if(i & 2) return 1;
	if(i & 4) return 2;
	if(i & 8) return 3;
	if(i & 16) return 4;
	if(i & 32) return 5;
	if(i & 64) return 6;
	if(i & 128) return 7;
	abort();
}
#endif

static uint8_t register_size(int i) {
	return 1 << (i / 6);
}

static int cast_register(int i, uint8_t size) {
	return (__builtin_ctz(size) * 6) + (i % 6);
}

static RegisterAllocator *create_ralloc(X86 *X) {
	RegisterState sildil = X->mode == X86_MODE_64 ? REGISTER_FREE : REGISTER_DOESNT_EXIST; /* Is sil & dil also available in 32-bit long mode? */
	
	RegisterAllocator *a = malloc(sizeof(RegisterAllocator) + sizeof(Register) * 18);
	a->registersCount = 18;
	memset(a->registers, 0, sizeof(Register) * 18);
	
	strncpy(a->registers[0].name, "al", 7);
	a->registers[0].state = REGISTER_FREE;
	a->registers[0].size = 1;
	a->registers[0].aliasBitmap = 4161;
	
	strncpy(a->registers[1].name, "bl", 7);
	a->registers[1].state = REGISTER_FREE;
	a->registers[1].size = 1;
	a->registers[1].aliasBitmap = 8322;
	
	strncpy(a->registers[2].name, "cl", 7);
	a->registers[2].state = REGISTER_FREE;
	a->registers[2].size = 1;
	a->registers[2].aliasBitmap = 16644;
	
	strncpy(a->registers[3].name, "dl", 7);
	a->registers[3].state = REGISTER_FREE;
	a->registers[3].size = 1;
	a->registers[3].aliasBitmap = 33288;
	
	strncpy(a->registers[4].name, "sil", 7);
	a->registers[4].state = sildil;
	a->registers[4].size = 1;
	a->registers[4].aliasBitmap = 66576;
	
	strncpy(a->registers[5].name, "dil", 7);
	a->registers[5].state = sildil;
	a->registers[5].size = 1;
	a->registers[5].aliasBitmap = 133152;
	
	strncpy(a->registers[6].name, "ax", 7);
	a->registers[6].state = REGISTER_FREE;
	a->registers[6].size = 2;
	a->registers[6].aliasBitmap = 4161;
	
	strncpy(a->registers[7].name, "bx", 7);
	a->registers[7].state = REGISTER_FREE;
	a->registers[7].size = 2;
	a->registers[7].aliasBitmap = 8322;
	
	strncpy(a->registers[8].name, "cx", 7);
	a->registers[8].state = REGISTER_FREE;
	a->registers[8].size = 2;
	a->registers[8].aliasBitmap = 16644;
	
	strncpy(a->registers[9].name, "dx", 7);
	a->registers[9].state = REGISTER_FREE;
	a->registers[9].size = 2;
	a->registers[9].aliasBitmap = 33288;
	
	strncpy(a->registers[10].name, "si", 7);
	a->registers[10].state = REGISTER_FREE;
	a->registers[10].size = 2;
	a->registers[10].aliasBitmap = 66576;
	
	strncpy(a->registers[11].name, "di", 7);
	a->registers[11].state = REGISTER_FREE;
	a->registers[11].size = 2;
	a->registers[11].aliasBitmap = 133152;
	
	strncpy(a->registers[12].name, "eax", 7);
	a->registers[12].state = REGISTER_FREE;
	a->registers[12].size = 4;
	a->registers[12].aliasBitmap = 4161;
	
	strncpy(a->registers[13].name, "ebx", 7);
	a->registers[13].state = REGISTER_FREE;
	a->registers[13].size = 4;
	a->registers[13].aliasBitmap = 8322;
	
	strncpy(a->registers[14].name, "ecx", 7);
	a->registers[14].state = REGISTER_FREE;
	a->registers[14].size = 4;
	a->registers[14].aliasBitmap = 16644;
	
	strncpy(a->registers[15].name, "edx", 7);
	a->registers[15].state = REGISTER_FREE;
	a->registers[15].size = 4;
	a->registers[15].aliasBitmap = 33288;
	
	strncpy(a->registers[16].name, "esi", 7);
	a->registers[16].state = REGISTER_FREE;
	a->registers[16].size = 4;
	a->registers[16].aliasBitmap = 66576;
	
	strncpy(a->registers[17].name, "edi", 7);
	a->registers[17].state = REGISTER_FREE;
	a->registers[17].size = 4;
	a->registers[17].aliasBitmap = 133152;
	
	return a;
}

static char *yasm_directname(int bytes) {
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

static char *yasm_sizespecifier(int bytes) {
	switch(bytes) {
#ifdef SYNTAX_GAS
	case 1:
		return "b";
	case 2:
		return "w";
	case 4:
		return "l";
	case 8:
		return "q";
#else
	case 1:
		return "byte";
	case 2:
		return "word";
	case 4:
		return "dword";
	case 8:
		return "qword";
#endif
	}

	abort();
	return NULL;
}

/* Use when a register is necessary, else use x86_allocate, which may use the stack instead. */
static X86AllocationInfo *x86_allocreg(X86 *X, size_t sz) {
	X86AllocationInfo *n = malloc(sizeof(*n));
	n->strategy = X86_ALLOC_REG;
	n->size = sz;
	n->refcount = 1;
	
	int ret = ralloc_alloc(X->rallocator, n, sz);
	if(ret != -1) {
		n->regId = ret;
		return n;
	}
	
	ret = cast_register(ralloc_findname(X->rallocator, "edi"), sz); /* There's no particular reason to use edi here. */
	
	X86AllocationInfo *o = X->rallocator->registers[ret].userdata;
	o->strategy = X86_ALLOC_STACK;
	o->stackDepth = X->stackDepth -= ((o->size + 3) % 4);
	
	n->regId = ralloc_alloc(X->rallocator, n, sz);
	return n;
}

static X86AllocationInfo *x86_allocate(X86 *X, size_t sz) {
	X86AllocationInfo *ret = malloc(sizeof(*ret));
	ret->size = sz;
	ret->refcount = 1;
	
	int reg = ralloc_alloc(X->rallocator, ret, sz);
	if(reg != -1) {
		ret->strategy = X86_ALLOC_REG;
		ret->regId = reg;
	} else {
		ret->strategy = X86_ALLOC_STACK;
		ret->stackDepth = X->stackDepth -= ((sz + 3) % 4);
	}
	
	return ret;
}

static void x86_free(X86 *X, X86AllocationInfo *info) {
	if(--info->refcount == 0) {
		if(info->strategy == X86_ALLOC_REG) {
			ralloc_free(X->rallocator, info->regId);
		}
		free(info);
	}
}

static dstr gdescribeinfo(X86 *X, X86AllocationInfo *info, size_t coerceToSize) {
	if(coerceToSize == -1) coerceToSize = info->size;
	
	switch(info->strategy) {
	case X86_ALLOC_REG: {
		int id = cast_register(info->regId, coerceToSize);
#ifdef SYNTAX_GAS
		return dstrfmt(dstrempty(), "%%%s", X->rallocator->registers[id].name);
#else
		return dstrfmt(dstrempty(), "%s", X->rallocator->registers[id].name);
#endif
	}
	case X86_ALLOC_STACK:
#ifdef SYNTAX_GAS
		return dstrfmt(dstrempty(), "%i(%%esp)", X->stackDepth - info->stackDepth);
#else
		return dstrfmt(dstrempty(), "%s[esp%s%i]", yasm_sizespecifier(coerceToSize), (X->stackDepth - info->stackDepth) >= 0 ? "+" : "", X->stackDepth - info->stackDepth);
#endif
	case X86_ALLOC_MEM:
#ifdef SYNTAX_GAS
		return dstrfmt(dstrempty(), "%s", info->memName);
#else
		return dstrfmt(dstrempty(), "%s[%s]", yasm_sizespecifier(coerceToSize), info->memName);
#endif
	}
	return NULL;
}

static void gmovi(X86 *X, X86AllocationInfo *info, size_t val) {
	dstr i = gdescribeinfo(X, info, -1);
	if(val == 0 && info->strategy == X86_ALLOC_REG) {
		X->text = dstrfmt(X->text, "xor %S, %S\n", i, i);
	} else {
#ifdef SYNTAX_GAS
		X->text = dstrfmt(X->text, "mov $%i, %S\n", val, i);
#else
		X->text = dstrfmt(X->text, "mov %S, %i\n", i, val);
#endif
	}
	dstrfree(i);
}

static void gmov(X86 *X, X86AllocationInfo *dst, X86AllocationInfo *src) {
	dstr s = gdescribeinfo(X, src, -1), d = gdescribeinfo(X, dst, -1);
	
	int zx = dst->size > src->size;
	
	if(dst->strategy == X86_ALLOC_REG || src->strategy == X86_ALLOC_REG) {
#ifdef SYNTAX_GAS
		X->text = dstrfmt(X->text, "mov%s%s%s %S, %S\n", zx ? "z" : "", zx ? yasm_sizespecifier(src->size) : "", zx ? yasm_sizespecifier(dst->size) : "", s, d);
#else
		X->text = dstrfmt(X->text, "mov%s %s, %s\n", zx ? "zx" : "", d, s);
#endif
	} else { /* Multiple memory operands which is an invalid combination. Copy src into a register. */
		X86AllocationInfo *tmp = x86_allocreg(X, dst->size);
		
		gmov(X, tmp, src);
		gmov(X, dst, tmp);
		
		x86_free(X, tmp);
	}
	dstrfree(s);
	dstrfree(d);
}

static void gaddi(X86 *X, X86AllocationInfo *info, size_t val) {
	dstr i = gdescribeinfo(X, info, -1);
#ifdef SYNTAX_GAS
	X->text = dstrfmt(X->text, "add $%i, %S\n", val, i);
#else
	X->text = dstrfmt(X->text, "add %S, %i\n", i, val);
#endif
	dstrfree(i);
}

static void gaddsub(X86 *X, X86AllocationInfo *dst, X86AllocationInfo *src, int isSub) {
	dstr s = gdescribeinfo(X, src, dst->size);
	dstr d = gdescribeinfo(X, dst, dst->size);
	
	if(dst->strategy == X86_ALLOC_REG || src->strategy == X86_ALLOC_REG) {
#ifdef SYNTAX_GAS
		X->text = dstrfmt(X->text, "%s %S, %S\n", isSub ? "sub" : "add", s, d);
#else
		X->text = dstrfmt(X->text, "%s %S, %S\n", isSub ? "sub" : "add", d, s);
#endif
	} else { /* Multiple memory operands which is an invalid combination. Copy src into a register. */
		X86AllocationInfo *tmp = x86_allocreg(X, dst->size);
		
		gmov(X, tmp, src);
		gaddsub(X, dst, tmp, isSub);
		
		x86_free(X, tmp);
	}
	dstrfree(s);
	dstrfree(d);
}

static void gcmp0(X86 *X, X86AllocationInfo *info) {
	dstr i = gdescribeinfo(X, info, -1);
	if(info->strategy == X86_ALLOC_REG) {
		X->text = dstrfmt(X->text, "test %S, %S\n", i, i);
	} else {
#ifdef SYNTAX_GAS
		X->text = dstrfmt(X->text, "cmp $0, %S\n", i);
#else
		X->text = dstrfmt(X->text, "cmp %S, 0\n", i);
#endif
	}
	dstrfree(i);
}

static void gcallr(X86 *X, const char *reg) {
#ifdef SYNTAX_GAS
	X->text = dstrfmt(X->text, "call %%%s\n", reg);
#else
	X->text = dstrfmt(X->text, "call %s\n", reg);
#endif
}

static void gcallsym(X86 *X, const char *sym) {
#ifdef SYNTAX_GAS
	X->text = dstrfmt(X->text, "call $%s\n", sym);
#else
	X->text = dstrfmt(X->text, "call %s\n", sym);
#endif
}

static void gpushr(X86 *X, const char *reg) {
#ifdef SYNTAX_GAS
	X->text = dstrfmt(X->text, "push %%%s\n", reg);
#else
	X->text = dstrfmt(X->text, "push %s\n", reg);
#endif
}

static void gpopr(X86 *X, const char *reg) {
#ifdef SYNTAX_GAS
	X->text = dstrfmt(X->text, "pop %%%s\n", reg);
#else
	X->text = dstrfmt(X->text, "pop %s\n", reg);
#endif
}

static void gglobal(X86 *X, const char *sym) {
#ifdef SYNTAX_GAS
	X->text = dstrfmt(X->text, ".global %s\n", sym);
#else
	X->text = dstrfmt(X->text, "global %s\n", sym);
#endif
}

static void gderef(X86 *X, X86AllocationInfo *dst, X86AllocationInfo *src) {
	dstr d = gdescribeinfo(X, dst, -1), s = gdescribeinfo(X, src, -1);
	
#ifdef SYNTAX_GAS
	X->text = dstrfmt(X->text, "mov (%S), %S\n", s, d);
#else
	X->text = dstrfmt(X->text, "mov %S, [%S]\n", d, s);
#endif
	
	dstrfree(d);
	dstrfree(s);
}

void x86_new(X86 *X) {
	X->text = dstrempty();
	X->lidx = 0;
	X->stackDepth = 0;
	
	X->loopStackIndex = 0;
	
	X->target = X86_TARGET_80386;
	X->mode = X86_MODE_32;
	
	X->rallocator = create_ralloc(X);
}

X86AllocationInfo *x86_visit_expression(X86 *X, AST *ast, X86AllocationInfo *dst) {
	if(ast->nodeKind == AST_EXPRESSION_PRIMITIVE) {
		if(!dst) dst = x86_allocate(X, 4);
		
		gmovi(X, dst, ast->expressionPrimitive.numerator / ast->expressionPrimitive.denominator);
		
		return dst;
	} else if(ast->nodeKind == AST_EXPRESSION_VAR) {
		//~ int dst = ralloc_alloc(X->rallocator, coerceToSize);
		
		//~ if(ast->expressionVar.thing->kind == VARTABLEENTRY_VAR) {
			//~ int src = ((X86AllocationInfo*) ast->expressionVar.thing->userdata)->id;
			
			//~ // fixes "mov al, bx"
			//~ if(register_size(src) > register_size(dst)) src = cast_register(src, register_size(dst));
			
			//~ gmovr(X, register_size(dst) > register_size(src), X->rallocator->registers[dst].name, X->rallocator->registers[src].name);
		//~ } else if(ast->expressionVar.thing->kind == VARTABLEENTRY_SYMBOL) {
			//~ gmovsym(X, coerceToSize, coerceToSize, X->rallocator->registers[dst].name, ast->expressionVar.thing->data.symbol.linkName);
		//~ }
		
		//~ return dst;
		
		X86AllocationInfo *vi = ast->expressionVar.thing->userdata;
		if(dst) { /* copy */
			gmov(X, dst, vi);
			return dst;
		} else {
			vi->refcount++;
			return vi;
		}
	} else if(ast->nodeKind == AST_EXPRESSION_BINARY_OP) {
		if(!dst) dst = x86_allocate(X, 4);
		
		x86_visit_expression(X, ast->expressionBinaryOp.operands[0], dst);
		
		for(size_t i = 1; i < ast->expressionBinaryOp.amountOfOperands; i++) {
			AST *operand = ast->expressionBinaryOp.operands[i];
			if(operand->nodeKind == AST_EXPRESSION_PRIMITIVE) {
				gaddi(X, dst, operand->expressionPrimitive.numerator / operand->expressionPrimitive.denominator);
			} else {
				X86AllocationInfo *a = x86_visit_expression(X, operand, NULL);
				gaddsub(X, dst, a, ast->expressionBinaryOp.operators[i - 1] == BINOP_SUB);
				x86_free(X, a);
			}
		}
		
		return dst;
	} else if(ast->nodeKind == AST_EXPRESSION_UNARY_OP) {
		if(!dst) dst = x86_allocate(X, type_size(ast->expressionUnaryOp.chaiuld->expression.type->pointer.of));
		
		X86AllocationInfo *a = x86_visit_expression(X, ast->expressionUnaryOp.chaiuld, NULL);
		gderef(X, dst, a);
		x86_free(X, a);
		
		return dst;
	} else if(ast->nodeKind == AST_EXPRESSION_CALL) {
		/* TODO: add. */
	}

	abort();
	return NULL;
}

AST *x86_visit_statement(X86 *X, AST *ast) {
	if(ast->nodeKind == AST_STATEMENT_DECL) {
		VarTableEntry *ent = ast->statementDecl.thing;
		if(ent->kind == VARTABLEENTRY_SYMBOL) {
			if(ent->data.symbol.isExternal) {
#ifndef SYNTAX_GAS /* GAS considers all undefined symbols to be external */
				X->text = dstrfmt(X->text, "extern %s\n", ent->data.symbol.linkName);
#endif
			} else {
				if(ast->statementDecl.expression->expression.constantType == EXPRESSION_NOT_CONSTANT) {
					fputs("Symbol declaration may contain constant expressions only.\n", stderr);
					abort();
				}
				
				if(!ent->data.symbol.isLocal) {
					gglobal(X, ent->name);
				}
				
				size_t typeSize = type_size(ent->type);
				
				if(ast->statementDecl.expression) {
					AST *expr = ast->statementDecl.expression;
#ifdef SYNTAX_GAS
					X->text = dstrfmt(X->text, "%s: .%s %i\n", ent->data.symbol.linkName, yasm_directname(typeSize), expr->expressionPrimitive.numerator / expr->expressionPrimitive.denominator);
#else
					X->text = dstrfmt(X->text, "%s: %s %i\n", ent->data.symbol.linkName, yasm_directname(typeSize), expr->expressionPrimitive.numerator / expr->expressionPrimitive.denominator);
#endif
				} else {
#ifdef SYNTAX_GAS
					X->text = dstrfmt(X->text, "%s: .skip %i\n", ent->data.symbol.linkName, typeSize);
#else
					X->text = dstrfmt(X->text, "%s: resb %i\n", ent->data.symbol.linkName, typeSize);
#endif
				}
			}
			
			X86AllocationInfo *info = malloc(sizeof(*info));
			info->strategy = X86_ALLOC_MEM;
			info->memName = ent->data.symbol.linkName;
			info->size = type_size(ent->type);
			info->refcount = 1;
			
			ent->userdata = info;
		} else if(ent->kind == VARTABLEENTRY_VAR) {
			X86AllocationInfo *info = x86_allocate(X, type_size(ent->type));
			x86_visit_expression(X, ast->statementDecl.expression, info);
			
			ent->userdata = info;
		}
	} else if(ast->nodeKind == AST_STATEMENT_IF) {
		switch(ast->statementIf.expression->expression.constantType) {
		case EXPRESSION_NOT_CONSTANT: {
			X86AllocationInfo *info = x86_visit_expression(X, ast->statementIf.expression, NULL);
			
			gcmp0(X, info);
#ifdef SYNTAX_GAS
			X->text = dstrfmt(X->text, "jz $.L%i\n", X->lidx);
#else
			X->text = dstrfmt(X->text, "jz .L%i\n", X->lidx);
#endif
			
			x86_visit_chunk(X, ast->statementIf.then);

			X->text = dstrfmt(X->text, ".L%i:\n", X->lidx++);
			break;
		}
		case EXPRESSION_CONSTANT_TRUTHY: {
			x86_visit_chunk(X, ast->statementIf.then);
			break;
		}
		default:
			break;
		}
	} else if(ast->nodeKind == AST_STATEMENT_LOOP) {
		int idStart = X->lidx++;
		int idEnd = X->lidx++;
		
		X->loopStack[X->loopStackIndex++] = idEnd; // TODO: check for overflow
		
		X->text = dstrfmt(X->text, ".L%i:\n", idStart);
		
		x86_visit_chunk(X, ast->statementLoop.body);
		
#ifdef SYNTAX_GAS
		X->text = dstrfmt(X->text, "jmp $.L%i\n", idStart);
#else
		X->text = dstrfmt(X->text, "jmp .L%i\n", idStart);
#endif
		
		X->text = dstrfmt(X->text, ".L%i:\n", idEnd);
		
		X->loopStackIndex--;
	} else if(ast->nodeKind == AST_STATEMENT_BREAK) {
#ifdef SYNTAX_GAS
		X->text = dstrfmt(X->text, "jmp $.L%i\n", X->loopStack[X->loopStackIndex - 1]);
#else
		X->text = dstrfmt(X->text, "jmp .L%i\n", X->loopStack[X->loopStackIndex - 1]);
#endif
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
	fprintf(stdout, ".section .text\n%s", X->text);
#else
	fprintf(stdout, "section .text\n%s", X->text);
#endif
}
