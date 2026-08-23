#pragma once
#include "compiler.h"
typedef struct {
    int stack[256];
    int sp;

    int variables[256];
} VM;

void vm_run(VM *vm, Bytecode *bytecode);