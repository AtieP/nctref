#include"lexer.h"
#include"parse.h"

#include"cg/x86.h"

#include"reporting.h"
#include<errno.h>
#include<string.h>

int main(int argc, char **argv) {
	if(argc == 1) stahp("No file specified.");
	
	FILE *f = fopen(argv[1], "rb");
	
	if(!f) stahp("%s: %s", argv[1], strerror(errno));
	
	Token *tokens = nct_lex(f);
	
	ASTChunk *chunk = nct_parse(tokens);
	
	X86 x86;
	x86_new(&x86);
	x86_visit_chunk(&x86, chunk);
	x86_finish(&x86);
}
