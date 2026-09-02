#pragma once
#include <stdio.h>
#include "token.h"

typedef struct {
    const char* source;
    const char *current;
} Lexer;

Lexer lexer_init(const char* source);
Token lexer_next_token(Lexer* lexer);

// debug
void lexer_print(const Lexer *lexer);
