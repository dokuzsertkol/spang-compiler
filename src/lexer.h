#pragma once
#include "token.h"

typedef struct {
    const char *path;
    const char *source;
    const char *current;

    size_t line;
    size_t column;
} Lexer;

Lexer lexer_init(const char* source);
Token lexer_next_token(Lexer* lexer);

// debug
void lexer_print(const Lexer *lexer);
