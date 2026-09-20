#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "source_manager.h"

SourceManager *source_manager_init() {
    SourceManager *manager = malloc(sizeof(*manager));
    if (!manager) return NULL;

    *manager = (SourceManager){0};
    return manager;
}

static char *get_canonical_path(const char* path) {
    const char *dot = strrchr(path, '.');

    if (!dot || strcmp(dot, ".spg") != 0) {
        fprintf(stderr, "Error: expected a .spg source file\n");
        return NULL;
    }

    char *canonicalPath = realpath(path, NULL);
    if (!canonicalPath) {
        fprintf(stderr, "Error: included file not found: %s\n", path);
    }
    return canonicalPath;
}

const char *source_manager_add(SourceManager *manager, const char *path) {
    if (!manager || !path) return NULL;

    char *canonicalPath = get_canonical_path(path);
    if (!canonicalPath) return NULL;

    for (size_t i = 0; i < manager->count; i++) {
        if (strcmp(manager->paths[i], canonicalPath) == 0) {
            free(canonicalPath);
            return manager->paths[i];
        }
    }

    if (manager->count >= manager->capacity) {
        size_t newCapacity = manager->capacity == 0 ? 8 : manager->capacity * 2;

        char **newPaths = realloc(manager->paths, sizeof(char *) * newCapacity);
        if (!newPaths) {
            free(canonicalPath);
            return NULL;
        }

        manager->paths = newPaths;
        manager->capacity = newCapacity;
    }

    manager->paths[manager->count++] = canonicalPath;

    return canonicalPath;
}

void source_manager_free(SourceManager *manager) {
    if (!manager) return;

    for (size_t i = 0; i < manager->count; i++) free(manager->paths[i]);

    free(manager->paths);

    manager->paths = NULL;
    manager->count = 0;
    manager->capacity = 0;
}