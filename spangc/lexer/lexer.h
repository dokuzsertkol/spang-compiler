#pragma once
#include "../token/token.h"

typedef struct {
    const char *path;
    
    char *source;
    const char *current;

    size_t line;
    size_t column;
} Lexer;

int lexer_init(Lexer *lexer, const char *path);
void lexer_free(Lexer *lexer);
Token lexer_next_token(Lexer* lexer);

// debug
void lexer_print(const Lexer *lexer);
