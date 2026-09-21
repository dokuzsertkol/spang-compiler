#pragma once
#include <stddef.h>
#include "../ast/ast.h"

typedef struct SemanticAnalyser SemanticAnalyser;

typedef enum {
    SEMANTIC_ERROR_REDECLARATION,
} SemanticErrorType;

typedef struct {
    SemanticErrorType type;
    char *tokenStr;
    const char *message;

    const char *path;
    size_t line;
    size_t column;
} SemanticError;

void semantic_error(SemanticAnalyser *analyser, SemanticErrorType type, const SourceLocation *source, const char* name, size_t length);
void semantic_print_error(const SemanticAnalyser *analyser);