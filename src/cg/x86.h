#ifndef CG_X86_H
#define CG_X86_H

#include"dstr.h"
#include"ast.h"
#include"regalloc.h"

typedef enum {
	X86_TARGET_80386, X86_TARGET_8086, X86_TARGET_M
} X86Target;

// Used to determine how to interact with SP, IP.
typedef enum {
	X86_MODE_UNSELECTED, X86_MODE_16, X86_MODE_32, X86_MODE_64
} X86Mode;

typedef struct {
	dstr text;
	size_t lidx; // ID of next .L label
	size_t stackDepth;
	
	size_t loopStack[64];
	size_t loopStackIndex;
	
	X86Target target;
	X86Mode mode;
	
	RegisterAllocator *rallocator;
} X86;

typedef enum {
	X86_ALLOC_STACK, X86_ALLOC_REG, X86_ALLOC_MEM, X86_ALLOC_MEM_DEREF, X86_ALLOC_REG_DEREF
} X86AllocationStrategy;

typedef struct X86AllocationInfo {
	X86AllocationStrategy strategy;
	size_t size;
	union {
		int stackDepth;
		int regId;
		const char *memName;
	};
	struct X86AllocationInfo *parent; /* Used by derefs and cast allocations */
	int refcount;
} X86AllocationInfo;

void x86_new(X86*);

void x86_visit_chunk(X86*, ASTChunk*);

void x86_finish(X86*);

#endif
