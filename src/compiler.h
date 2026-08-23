#pragma once
#include "ast.h"

typedef enum {
    OP_PUSH,
    OP_LOAD,
    OP_STORE,
    OP_HALT,
} OpCode;

typedef struct {
    OpCode opcode;
    int operand;
} Instruction;

typedef struct {
    Instruction instructions[256];
    int count;
} Bytecode;

Bytecode compile(ASTProgram *program);