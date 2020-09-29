#include"lexer.h"

#include<stdlib.h>
#include<assert.h>
#include<string.h>
#include"reporting.h"

char *TOKEN_NAMES[] = {
	"identifier", "'local'", "EOF", "number", "';'", "':'", "'if'", "'('", "')'", "'{'", "'}'", "'='", "'+'", "'-'", "'*'", "'/'", "'extern'", "'loop'", "'break'", "','"
};

static int isAlpha(int c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int isNum(int c) {
	return c >= '0' && c <= '9';
}

static int isAlphanum(int c) {
	return isAlpha(c) || isNum(c);
}

static int isWS(int c) {
	return c == ' ' || c == '\n' || c == '\r' || c == '\b' || c == '\t';
}

static size_t currentRow = 0;
static size_t currentColumn = 0;
static int ungetted = EOF;

int nextc(FILE *f) {
	if(ungetted != EOF) {
		int ret = ungetted;
		ungetted = EOF;
		return ret;
	}
	
	int c = fgetc(f);
	if(c == '\n') {
		currentRow++;
		currentColumn = 0;
	} else if(c != EOF) {
		currentColumn++;
	}
	return c;
}

int pushc(int c, FILE *f) {
	ungetted = c;
}

Token nct_tokenize(FILE *f) {
	Token tok;
	tok.content = NULL;
	tok.row = currentRow;
	tok.column = currentColumn;
	
	int c = nextc(f);
	
	if(c == EOF) {
		tok.type = TOKEN_EOF;
		return tok;
	}
	
	if(c == ';') {
		tok.type = TOKEN_SEMICOLON;
		return tok;
	} else if(c == ':') {
		tok.type = TOKEN_COLON;
		return tok;
	} else if(c == '(') {
		tok.type = TOKEN_PAREN_L;
		return tok;
	} else if(c == ')') {
		tok.type = TOKEN_PAREN_R;
		return tok;
	} else if(c == '{') {
		tok.type = TOKEN_SQUIGGLY_L;
		return tok;
	} else if(c == '}') {
		tok.type = TOKEN_SQUIGGLY_R;
		return tok;
	} else if(c == '+') {
		tok.type = TOKEN_PLUS;
		return tok;
	} else if(c == '-') {
		tok.type = TOKEN_MINUS;
		return tok;
	} else if(c == '*') {
		tok.type = TOKEN_STAR;
		return tok;
	} else if(c == '/') {
		tok.type = TOKEN_SLASH;
		return tok;
	} else if(c == '=') {
		tok.type = TOKEN_EQUALS;
		return tok;
	} else if(c == ',') {
		tok.type = TOKEN_COMMA;
		return tok;
	} else if(isAlpha(c) || c == '@') {
		char *content = calloc(64, 1);
		
		size_t i = 0;
		content[i++] = c;
		
		while(c = nextc(f), (isAlphanum(c) || c == '@')) {
			assert(i != 63 && "Identifiers have a maximum size of 63.");
			
			content[i++] = c;
		}
		
		pushc(c, f);
		
		if(!strcmp(content, "local")) {
			free(content);
			tok.type = TOKEN_LOCAL;
			return tok;
		} else if(!strcmp(content, "if")) {
			free(content);
			tok.type = TOKEN_IF;
			return tok;
		} else if(!strcmp(content, "extern")) {
			free(content);
			tok.type = TOKEN_EXTERN;
			return tok;
		} else if(!strcmp(content, "loop")) {
			free(content);
			tok.type = TOKEN_LOOP;
			return tok;
		} else if(!strcmp(content, "break")) {
			free(content);
			tok.type = TOKEN_BREAK;
			return tok;
		}
		
		tok.type = TOKEN_IDENTIFIER;
		tok.content = content;
		return tok;
	} else if(isNum(c)) {
		char *content = calloc(64, 1);
		
		size_t i = 0;
		content[i++] = c;
		
		while(c = nextc(f), isNum(c)) {
			assert(i != 63 && "Numbers have a maximum size of 63.");
			
			content[i++] = c;
		}
		
		int base = strtol(content, NULL, 10);
		
		if(c == 'r') {
			content[i++] = c;
			
			while(c = nextc(f), (isNum(c) || (base > 10 && c >= 'A' && c < ('A' + base - 10)))) {
				assert(i != 63 && "Numbers have a maximum size of 63.");

				content[i++] = c;
			}
		}
		
		pushc(c, f);
		
		tok.type = TOKEN_NUMBER;
		tok.content = content;
		return tok;
	} else if(isWS(c)) {
		int c;

		while(c = nextc(f), isWS(c)) {
		}

		pushc(c, f);
		
		return nct_tokenize(f);
	}

	stahp(currentRow, currentColumn, "Invalid character '%c' (byte %i)", c, c);
}

Token *nct_lex(FILE *f) {
	size_t length = 8, index = 0;
	Token *list = malloc(sizeof(*list) * length);
	
	while(1) {
		list[index] = nct_tokenize(f);
		
		if(list[index].type == TOKEN_EOF) {
			return list;
		}
		
		index++;
		
		if(index == length) {
			length *= 2;
			list = realloc(list, sizeof(*list) * length);
		}
	}
	
	return NULL; /* Doesn't reach here. */
}
