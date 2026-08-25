#include <stdlib.h>
#include <stdio.h>
#include "ir.h"

static IROperand operand_none(void) {
    return (IROperand) {
        .type = IR_OPERAND_NONE
    };
}

static IROperand operand_temp(int temp) {
    return (IROperand) {
        .type = IR_OPERAND_TEMP,
        .value.temp = temp
    };
}

static IROperand operand_literal(long value) {
    return (IROperand) {
        .type = IR_OPERAND_LITERAL,
        .value.literal = value
    };
}

static IROperand operand_variable(const char *name, int length) {
    return (IROperand) {
        .type = IR_OPERAND_VARIABLE,
        .value.variable = {
            .start = name,
            .length = length,
        },
    };
}

static int new_temp(int *tempCount) {
    return (*tempCount)++;
}

static int ir_add_instruction(IR *program, IRInstruction instruction) {
    if (program->count >= program->capacity) {
        int new_capacity = program->capacity == 0 ? 8 : program->capacity * 2;

        IRInstruction *new_instructions = realloc(program->instructions, sizeof(*program->instructions) * new_capacity);

        if (!new_instructions) return 0;

        program->instructions = new_instructions;
        program->capacity = new_capacity;
    }

    program->instructions[program->count++] = instruction;
    return 1;
}

static int generate_expression(const ASTNode *node, IR *program, int *tempCount, IROperand *result) {
    switch (node->type) {
        case AST_LITERAL: {
            int temp = new_temp(tempCount);

            if (!ir_add_instruction(program, (IRInstruction) {
                .op = IR_CONST,
                .dest = operand_temp(temp),
                .left = operand_literal(node->data.literal.value),
                .right = operand_none()
            })) return 0;

            *result = operand_temp(temp);
            return 1;
        }

        case AST_VARIABLE: {
            int temp = new_temp(tempCount);

            if (!ir_add_instruction(program, (IRInstruction) {
                .op = IR_LOAD,
                .dest = operand_temp(temp),
                .left = operand_variable(
                    node->data.variable.start,
                    node->data.variable.length
                ),
                .right = operand_none()
            })) return 0;

            *result = operand_temp(temp);
            return 1;
        }

        case AST_OPERATOR: {
            IROperand left;
            IROperand right;

            if (!generate_expression(node->data.operation.left, program, tempCount, &left)) return 0;

            if (!generate_expression(node->data.operation.right, program, tempCount, &right)) return 0;

            int temp = new_temp(tempCount);

            IROp op;

            switch (node->data.operation.operatorType) {
                case OP_PLUS: op = IR_ADD; break;

                case OP_MINUS: op = IR_SUB; break;

                case OP_MULTIPLY: op = IR_MUL; break;

                case OP_DIVIDE: op = IR_DIV; break;

                default: return 0;
            }

            if (!ir_add_instruction(program, (IRInstruction) {
                .op = op,
                .dest = operand_temp(temp),
                .left = left,
                .right = right
            })) {
                return 0;
            }

            *result = operand_temp(temp);
            return 1;
        }

        default: return 0;
    }
}

static int generate_statement(const ASTNode *node, IR *program, int *tempCount) {
    switch (node->type) {

        case AST_VARIABLE_DECLARATION: {
            return 1;
        }

        case AST_ASSIGNMENT: {
            IROperand value;

            if (!generate_expression(node->data.binary.right, program, tempCount, &value)) return 0;

            return ir_add_instruction(program, (IRInstruction) {
                .op = IR_STORE,
                .dest = operand_variable(
                    node->data.binary.left->data.variable.start,
                    node->data.binary.left->data.variable.length
                ),
                .left = value,
                .right = operand_none()
            });
        }

        default: return 0;
    }
}

IR ir_generate(const ASTNode *program) {
    IR ir = {
        .instructions = NULL,
        .count = 0,
        .capacity = 0
    };

    int tempCount = 0;

    for (int i = 0; i < program->data.program.count; i++) {
        const ASTNode *statement = program->data.program.statements[i];

        if (!generate_statement(statement, &ir, &tempCount)) {
            free(ir.instructions);

            return (IR) {
                .instructions = NULL,
                .count = 0,
                .capacity = 0
            };
        }
    }
    return ir;
}

// debug
static void print_operand(IROperand operand) {
    switch (operand.type) {
        case IR_OPERAND_NONE: break;

        case IR_OPERAND_TEMP: printf("t%i", operand.value.temp); break;

        case IR_OPERAND_LITERAL: printf("%ld", operand.value.literal); break;

        case IR_OPERAND_VARIABLE:
            for (int i = 0; i < operand.value.variable.length; i++) printf("%c", operand.value.variable.start[i]);
            break;
    }
}

static void print_instruction(const IRInstruction *instruction) {
    switch (instruction->op) {
        case IR_CONST:
            printf("CONST ");
            print_operand(instruction->left);
            printf(" -> ");
            print_operand(instruction->dest);
            break;

        case IR_LOAD:
            printf("LOAD ");
            print_operand(instruction->left);
            printf(" -> ");
            print_operand(instruction->dest);
            break;

        case IR_STORE:
            printf("STORE ");
            print_operand(instruction->left);
            printf(" -> ");
            print_operand(instruction->dest);
            break;

        case IR_ADD:
            printf("ADD ");
            print_operand(instruction->left);
            printf(", ");
            print_operand(instruction->right);
            printf(" -> ");
            print_operand(instruction->dest);
            break;

        case IR_SUB:
            printf("SUB ");
            print_operand(instruction->left);
            printf(", ");
            print_operand(instruction->right);
            printf(" -> ");
            print_operand(instruction->dest);
            break;

        case IR_MUL:
            printf("MUL ");
            print_operand(instruction->left);
            printf(", ");
            print_operand(instruction->right);
            printf(" -> ");
            print_operand(instruction->dest);
            break;

        case IR_DIV:
            printf("DIV ");
            print_operand(instruction->left);
            printf(", ");
            print_operand(instruction->right);
            printf(" -> ");
            print_operand(instruction->dest);
            break;
    }

    putchar('\n');
}

void ir_print(const IR *program) {
    printf("=== IR PRINT ===\n");
    for (int i = 0; i < program->count; i++) {
        printf("%i: ", i);
        print_instruction(&program->instructions[i]);
    }
    printf("=== END ===\n\n");
}