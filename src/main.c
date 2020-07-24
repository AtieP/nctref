#include"lexer.h"
#include"parse.h"

#include"cg/x86.h"

int main(int argc, char **argv) {
	FILE *f = fopen(argv[1], "rb");
	
	Token *tokens = nct_lex(f);
	
	ASTChunk *chunk = nct_parse(tokens);
	
	X86 x86;
	x86_new(&x86);
	x86_visit_chunk(&x86, chunk);
	x86_finish(&x86);
}
