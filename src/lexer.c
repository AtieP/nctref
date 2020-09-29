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
	
	Token tok;
	tok.content = NULL;
	
	if(c == -1) {
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
		
		while(c = fgetc(f), (isAlphanum(c) || c == '@')) {
			assert(i != 63 && "Identifiers have a maximum size of 63.");
			
			content[i++] = c;
		}
		
		ungetc(c, f);
		
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
		
		tok.type = TOKEN_NUMBER;
		tok.content = content;
		return tok;
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
	
	return NULL; /* Doesn't reach here. */
}
