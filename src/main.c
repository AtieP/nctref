#include"lexer.h"
#include"parse.h"

#include"cg/x86.h"

#include"reporting.h"
#include<errno.h>
#include<string.h>

int main(int argc, char **argv) {
	if(argc == 1) stahp(3, 1415, "No file specified.");
	
	FILE *f = !strcmp(argv[1], "-") ? stdin : fopen(argv[1], "rb");
	
	if(!f) stahp(2, 7182, "%s: %s", argv[1], strerror(errno));
	
	Token *tokens = nct_lex(f);
	
	ASTChunk *chunk = nct_parse(tokens);
	
	X86 x86;
	x86_new(&x86);
	x86_visit_chunk(&x86, chunk);
	x86_finish(&x86);
}
