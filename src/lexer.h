#pragma once
#include <stdio.h>

typedef enum {
    TOKEN_EOF,
    TOKEN_LITERAL,
    TOKEN_IDENTIFIER,
    TOKEN_LET,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_EQUAL,
    TOKEN_ENDLINE,
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    int length;
} Token;

typedef struct {
    const char* source;
    const char *current;
} Lexer;

Lexer lexer_init(const char* source);
Token lexer_next_token(Lexer* lexer);
int token_to_int(Token* token);
char token_to_char(Token* token);

// debug
void lexer_print(const Lexer *lexer);
