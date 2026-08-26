#pragma once
#include "ir.h"

typedef struct {
    int tempCount;
    int variableCount;
} StackLayout;

int codegen_generate(const IR *ir, const char *output_path);