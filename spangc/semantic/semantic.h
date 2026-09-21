#pragma once
#include "../ast/ast.h"
#include "semantic_error.h"

typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_STRUCT,
    SYMBOL_FUNTCION,
} SymbolType;

typedef struct {
    SymbolType type;
    char *name;
    size_t length;
} Symbol;

typedef struct {
    Symbol *symbols;
    size_t capacity;
    size_t count;
} SymbolTable;

typedef struct SemanticAnalyser {
    SymbolTable table;
    SemanticError error;
    int hasError;
} SemanticAnalyser;

SemanticAnalyser *semantic_analyser_init();
int semantic_analyse(SemanticAnalyser *analyser, const AST_Program *program);