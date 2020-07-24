#ifndef NCTREF_LEXER_H
#define NCTREF_LEXER_H

#include<stdio.h>

extern char *TOKEN_NAMES[];

typedef enum {
	TOKEN_IDENTIFIER, TOKEN_LOCAL, TOKEN_EOF, TOKEN_NUMBER, TOKEN_SEMICOLON, TOKEN_COLON, TOKEN_IF, TOKEN_PAREN_L, TOKEN_PAREN_R, TOKEN_SQUIGGLY_L, TOKEN_SQUIGGLY_R, TOKEN_EQUALS, TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH
} TokenKind;

typedef struct {
	TokenKind type;
	
	char *content; /* NULL for keywords. */
} Token;

Token nct_tokenize(FILE*);
Token *nct_lex(FILE*);

#endif
