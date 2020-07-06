#ifndef CG_X86_H
#define CG_X86_H

#include"sds.h"
#include"ast.h"

typedef struct {
	sds text;
} X86;

void x86_new(X86*);

void x86_visit_compilation_unit(X86*, ASTCompilationUnit*);

#endif
