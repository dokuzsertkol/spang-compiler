#pragma once
#include "../token/token.h"

typedef struct {
    char *source;
    const char *current;

    char *path;
    size_t line;
    size_t column;
} Lexer;

Lexer *lexer_init(const char *path);
void lexer_free(Lexer *lexer);
Token lexer_next_token(Lexer* lexer);