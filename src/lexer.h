#pragma once
#include <stdio.h>

typedef enum {
    TOKEN_EOF,
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
    TOKEN_LET,
    TOKEN_PLUS,
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

