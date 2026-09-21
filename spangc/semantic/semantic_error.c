#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "semantic.h"
#include "semantic_error.h"

static void semantic_error_at(SemanticAnalyser *analyser, SemanticErrorType type, const SourceLocation *source, char* token, const char *message) {
    if (analyser->hasError) return;

    analyser->hasError = true;

    analyser->error = (SemanticError) {
        .type = type,
        .line = source->line,
        .column = source->column,
        .path = source->sourcePath,
        .tokenStr = token,
        .message = message,
    };
}

void semantic_error(SemanticAnalyser *analyser, SemanticErrorType type, const SourceLocation *source, const char* name, size_t length) {
    const char *message = NULL;

    switch (type) {
        case SEMANTIC_ERROR_REDECLARATION:
            message = "redeclaration of identifier";
            break;
    }

    char *token = strndup(name, length);
    semantic_error_at(analyser, type, source, token, message);
}

void semantic_print_error(const SemanticAnalyser *analyser) {
    if (!analyser || !analyser->hasError) return;

    const SemanticError *error = &analyser->error;

    fprintf(stderr, "%s:%zu:%zu: semantic error: %s %s", error->path, error->line, error->column, error->message, error->tokenStr);

    fprintf(stderr, "\n");
}