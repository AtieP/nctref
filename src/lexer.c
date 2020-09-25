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

static int isAlphanum(char c) {
	return isAlpha(c) || isNum(c);
}

static int isWS(char c) {
	return c == ' ' || c == '\n' || c == '\r' || c == '\b' || c == '\t';
}

Token nct_tokenize(FILE *f) {
	int c = fgetc(f);
	
	if(c == -1) {
		return (Token) {.type = TOKEN_EOF, .content = NULL};
	}
	
	if(c == ';') {
		return (Token) {.type = TOKEN_SEMICOLON, .content = NULL};
	} else if(c == ':') {
		return (Token) {.type = TOKEN_COLON, .content = NULL};
	} else if(c == '(') {
		return (Token) {.type = TOKEN_PAREN_L, .content = NULL};
	} else if(c == ')') {
		return (Token) {.type = TOKEN_PAREN_R, .content = NULL};
	} else if(c == '{') {
		return (Token) {.type = TOKEN_SQUIGGLY_L, .content = NULL};
	} else if(c == '}') {
		return (Token) {.type = TOKEN_SQUIGGLY_R, .content = NULL};
	} else if(c == '+') {
		return (Token) {.type = TOKEN_PLUS, .content = NULL};
	} else if(c == '-') {
		return (Token) {.type = TOKEN_MINUS, .content = NULL};
	} else if(c == '*') {
		return (Token) {.type = TOKEN_STAR, .content = NULL};
	} else if(c == '/') {
		return (Token) {.type = TOKEN_SLASH, .content = NULL};
	} else if(c == '=') {
		return (Token) {.type = TOKEN_EQUALS, .content = NULL};
	} else if(c == ',') {
		return (Token) {.type = TOKEN_COMMA, .content = NULL};
	} else if(isAlpha(c) || c == '@') {
		char *content = calloc(64, 1);
		
		size_t i = 0;
		content[i++] = c;
		
		while(c = fgetc(f), (isAlphanum(c) || c == '@')) {
			assert(i != 63 && "Identifiers have a maximum size of 63.");
			
			content[i++] = c;
		}
		
		ungetc(c, f);
		
		if(!strcmp(content, "local")) {
			free(content);
			return (Token) {.type = TOKEN_LOCAL, .content = NULL};
		} else if(!strcmp(content, "if")) {
			free(content);
			return (Token) {.type = TOKEN_IF, .content = NULL};
		} else if(!strcmp(content, "extern")) {
			free(content);
			return (Token) {.type = TOKEN_EXTERN, .content = NULL};
		} else if(!strcmp(content, "loop")) {
			free(content);
			return (Token) {.type = TOKEN_LOOP, .content = NULL};
		} else if(!strcmp(content, "break")) {
			free(content);
			return (Token) {.type = TOKEN_BREAK, .content = NULL};
		}
		
		return (Token) {.type = TOKEN_IDENTIFIER, .content = content};
	} else if(isNum(c)) {
		char *content = calloc(64, 1);
		
		size_t i = 0;
		content[i++] = c;
		
		while(c = fgetc(f), isNum(c)) {
			assert(i != 63 && "Numbers have a maximum size of 63.");
			
			content[i++] = c;
		}
		
		int base = strtol(content, NULL, 10);
		
		if(c == 'r') {
			content[i++] = c;
			
			while(c = fgetc(f), (isNum(c) || (base > 10 && c >= 'A' && c < ('A' + base - 10)))) {
				assert(i != 63 && "Numbers have a maximum size of 63.");

				content[i++] = c;
			}
		}
		
		ungetc(c, f);
		
		return (Token) {.type = TOKEN_NUMBER, .content = content};
	} else if(isWS(c)) {
		char c;

		while(c = fgetc(f), isWS(c)) {
		}

		ungetc(c, f);
		
		return nct_tokenize(f);
	}

	stahp("Invalid character '%c'", c);
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
}
