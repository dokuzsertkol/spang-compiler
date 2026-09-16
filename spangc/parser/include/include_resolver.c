#include "string.h"
#include <stdlib.h>
#include "include_resolver.h"
#include "../parser.h"

IncludeResolver *include_resolver_init(Parser *parser) {
    if (!parser) return NULL;

    IncludeResolver *resolver = malloc(sizeof(*resolver));
    if (!resolver) return NULL;

    *resolver = (IncludeResolver ){ .parser = parser };
    return resolver;
}

static int path_exists(char **paths, size_t count, const char *path) {
    for (size_t i = 0; i < count; i++) if (strcmp(paths[i], path) == 0) return 1;
    return 0;
}

static int active_paths_pop(IncludeResolver *resolver) {
    if (!resolver || resolver->activeCount == 0) return 0;

    resolver->activePaths[--resolver->activeCount] = NULL;
    return 1;
}

static int active_paths_push(IncludeResolver *resolver, char* path) {
    if (!resolver) return 0;
    
    if (resolver->activeCount >= resolver->activeCapacity) {
        int newCapacity = resolver->activeCapacity == 0 ? 8 : resolver->activeCapacity * 2;

        char **newActivePaths = realloc(resolver->activePaths, sizeof(char *) * newCapacity);
        if (!newActivePaths) return 0;

        resolver->activePaths = newActivePaths;
        resolver->activeCapacity = newCapacity;
    }

    resolver->activePaths[resolver->activeCount++] = path;
    return 1;
}

static int included_paths_push(IncludeResolver *resolver, char* path) {
    if (!resolver) return 0;
    
    if (resolver->includedCount >= resolver->includedCapacity) {
        int newCapacity = resolver->includedCapacity == 0 ? 8 : resolver->includedCapacity * 2;

        char **newIncludedPaths = realloc(resolver->includedPaths, sizeof(char *) * newCapacity);
        if (!newIncludedPaths) return 0;

        resolver->includedPaths = newIncludedPaths;
        resolver->includedCapacity = newCapacity;
    }

    resolver->includedPaths[resolver->includedCount++] = path;
    return 1;
}

static char *resolve_include_path(const char *sourcePath, const char *includePath) {
    if (!sourcePath || !includePath) return NULL;

    if (includePath[0] == '/') return strdup(includePath);

    const char *slash = strrchr(sourcePath, '/');

    if (!slash) return strdup(includePath);

    size_t directoryLength = (size_t)(slash - sourcePath);
    size_t includeLength = strlen(includePath);

    char *resolvedPath = malloc(directoryLength + 1 + includeLength + 1);

    if (!resolvedPath)return NULL;

    memcpy(resolvedPath, sourcePath, directoryLength);
    
    resolvedPath[directoryLength] = '/';

    memcpy(resolvedPath + directoryLength + 1, includePath, includeLength + 1);

    return resolvedPath;
}

IncludeResult include_resolver_parse(IncludeResolver *resolver, const char *path, AST_Program **program) {
    if (!resolver || !resolver->parser || !path || !program) return INCLUDE_RESULT_ERROR;

    *program = NULL;

    char *resolvedPath = resolve_include_path(resolver->parser->lexer->path, path);

    if (!resolvedPath) return INCLUDE_RESULT_ERROR;

    Lexer *newLexer = lexer_init(resolvedPath);
    free(resolvedPath);
    if (!newLexer) return INCLUDE_RESULT_ERROR;

    if (path_exists(resolver->includedPaths, resolver->includedCount, newLexer->path)) {
        lexer_free(newLexer);
        return INCLUDE_RESULT_SKIP;
    }

    Lexer *oldLexer = resolver->parser->lexer;
    Token oldCurrent = resolver->parser->current;

    resolver->parser->lexer = newLexer;
    resolver->parser->current = lexer_next_token(resolver->parser->lexer);

    if (path_exists(resolver->activePaths, resolver->activeCount, newLexer->path)) {
        parser_error(resolver->parser, PARSER_ERROR_CIRCULAR_INCLUDE);
        lexer_free(newLexer);
        return INCLUDE_RESULT_ERROR;
    }

    if (!active_paths_push(resolver, newLexer->path)) {
        lexer_free(newLexer);
        return INCLUDE_RESULT_ERROR;
    }

    *program = parse_program(resolver->parser);

    resolver->parser->lexer = oldLexer;
    resolver->parser->current = oldCurrent;

    active_paths_pop(resolver);

    if (!*program) {
        lexer_free(newLexer);
        return INCLUDE_RESULT_ERROR;
    }

    char *includedPath = strdup(newLexer->path);
    if (!includedPath || !included_paths_push(resolver, includedPath)) {
        free(includedPath);
        ast_program_free(*program);
        *program = NULL;
        lexer_free(newLexer);

        return INCLUDE_RESULT_ERROR;
    }

    lexer_free(newLexer);

    return INCLUDE_RESULT_SUCCESS;
}

void include_resolver_free(IncludeResolver *resolver) {
    if (!resolver) return;

    for (size_t i = 0; i < resolver->includedCount; i++) free(resolver->includedPaths[i]);

    free(resolver->includedPaths);
    free(resolver->activePaths);
    free(resolver);
}