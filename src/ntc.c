#include"lexer.h"
#include"parse.h"

#include"ntc.h"

#include"reporting.h"
#include<errno.h>
#include<string.h>

static int argc;
static char **argv;

X86 Xinst;

const char* ntc_get_arg(const char *name) {
	for(int i = 1; i < argc; i++) {
		if(strstr(argv[i], name) == argv[i]) {
			return strchr(argv[i], '=') + 1;
		}
	}
	return NULL;
}

int main(int argc_, char **argv_) {
	argc = argc_;
	argv = argv_;
	
	const char *argFile = ntc_get_arg("in");
	
	if(!argFile) stahp(3, 1415, "No file specified.");
	
	FILE *f = !strcmp(argFile, "-") ? stdin : fopen(argFile, "rb");
	
	if(!f) stahp(2, 7182, "%s: %s", argFile, strerror(errno));
	
	Token *tokens = nct_lex(f);
	
	ASTChunk *chunk = nct_parse(tokens);
	
	x86_new(&Xinst);
	x86_visit_chunk(&Xinst, chunk);
	x86_finish(&Xinst);
}
