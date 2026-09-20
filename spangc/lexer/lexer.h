#pragma once
#include "../token/token.h"
#include "../source/source_manager.h"

typedef struct {
    char *source;
    const char *current;

    const char *path;
    size_t line;
    size_t column;
} Lexer;

Lexer *lexer_init(SourceManager *manager, const char *path);
void lexer_free(Lexer *lexer);
Token lexer_next_token(Lexer* lexer);