#pragma once
#include "ast.h"

#define AST_PRINT_MAX_DEPTH 256

typedef struct {
    bool branch[AST_PRINT_MAX_DEPTH];
    size_t depth;
} AST_PrintContext;

void program_print(AST_Program *program);