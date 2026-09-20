#pragma once
#include <stddef.h>

typedef struct {
    char **paths;
    size_t count;
    size_t capacity;
} SourceManager;

SourceManager *source_manager_init();
void source_manager_free(SourceManager *manager);
const char *source_manager_add(SourceManager *manager, const char *path);