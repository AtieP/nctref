#include"lexer.h"
#include"parse.h"

#include"cg/x86.h"

int main(int argc, char **argv) {
	FILE *f = fopen(argv[1], "rb");
	
	Token *tokens = nct_lex(f);
	
	ASTCompilationUnit *ast = nct_parse(tokens);
	
	X86 x86;
	x86_new(&x86);
	x86_visit_compilation_unit(&x86, ast);
	x86_finish(&x86);
}
