#ifndef CG_X86_H
#define CG_X86_H

#include"sds.h"
#include"ast.h"
#include"regalloc.h"

typedef enum {
	X86_TARGET_80386
} X86Target;

// Used to determine how to interact with SP, IP.
typedef enum {
	X86_MODE_16, X86_MODE_32, X86_MODE_64
} X86Mode;

typedef struct {
	sds text;
	size_t lidx; // ID of next .L label
	size_t stackDepth;
	
	size_t loopStack[64];
	size_t loopStackIndex;
	
	X86Target target;
	X86Mode mode;
	
	RegisterAllocator *rallocator;
} X86;

typedef struct {
	int isInRegister;
	int id;
} X86VarEntryInfo;

void x86_new(X86*);

void x86_visit_chunk(X86*, ASTChunk*);

void x86_finish(X86*);

#endif
