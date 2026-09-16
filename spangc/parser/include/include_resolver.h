#pragma once
#include "../../ast/ast.h"
#include <stddef.h>

typedef struct Parser Parser;

typedef enum {
    INCLUDE_RESULT_ERROR,
    INCLUDE_RESULT_SKIP,
    INCLUDE_RESULT_SUCCESS
} IncludeResult;

typedef struct {
    Parser *parser;

    char **includedPaths;
    size_t includedCount;
    size_t includedCapacity;

    char **activePaths;
    size_t activeCount;
    size_t activeCapacity;
} IncludeResolver;

IncludeResolver *include_resolver_init(Parser *parser);
void include_resolver_free(IncludeResolver *resolver);
IncludeResult include_resolver_parse(IncludeResolver *resolver, const char *path, AST_Program **program);