#pragma once
#include "ast.h"

typedef enum {
    IR_OPERAND_NONE,
    IR_OPERAND_TEMP,
    IR_OPERAND_LITERAL,
    IR_OPERAND_VARIABLE,
} IROperandType;

typedef struct {
    IROperandType type;

    union {
        int temp;
        long literal;
        
        struct {
            const char *start;
            int length;
        } variable;
    } value;

} IROperand;

typedef enum {
    IR_CONST,
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,

    IR_LOAD,
    IR_STORE,
} IROp;

typedef struct {
    IROp op;
    IROperand dest;
    IROperand left;
    IROperand right;
} IRInstruction;

typedef struct {
    IRInstruction* instructions;
    int capacity;
    int count;
} IR;

IR ir_generate(const ASTNode* program);

// debug
void ir_print(const IR *program);