#pragma once
#include "ast.h"

typedef struct {
    const char *name;
    int length;
} Symbol;

typedef struct {
    Symbol *symbols;
    int capacity;
    int count;
} SymbolTable;

int semantic_analyse(const ASTNode *root);