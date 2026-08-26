#include <stdio.h>
#include "codegen.h"

static int variable_offset(int variable) {
    return -sizeof(long) * (variable + 1);
}

static int temp_offset(int temp, int variableCount) {
    return -sizeof(long) * (variableCount + temp + 1);
}

static StackLayout calculate_stack_layout(const IR *ir) {
    StackLayout layout = {
        .tempCount = 0,
        .variableCount = 0
    };

    for (int i = 0; i < ir->count; i++) {
        const IRInstruction *instruction = &ir->instructions[i];

        IROperand operands[] = {
            instruction->dest,
            instruction->left,
            instruction->right
        };

        for (int j = 0; j < 3; j++) {
            IROperand operand = operands[j];

            if (operand.type == IR_OPERAND_TEMP && operand.value.temp + 1 > layout.tempCount) layout.tempCount = operand.value.temp + 1;

            if (operand.type == IR_OPERAND_VARIABLE && operand.value.variable + 1 > layout.variableCount) layout.variableCount = operand.value.variable + 1;
        }
    }

    return layout;
}

int codegen_generate(const IR *ir, const char *outputPath){
    FILE *output = fopen(outputPath, "w");
    if (!output) return 0;

    StackLayout layout = calculate_stack_layout(ir);
    int stack_size = (layout.tempCount + layout.variableCount) * sizeof(long);

    fprintf(output, ".section .text\n");
    fprintf(output, ".global _start\n\n");
    fprintf(output, "_start:\n");
    fprintf(output, "   mov %%rsp, %%rbp\n");
    fprintf(output, "   sub $%i, %%rsp\n", stack_size);
    
    for (int i = 0; i < ir->count; i++) {
        const IRInstruction *instruction = &ir->instructions[i];

        switch (instruction->op) {
            case IR_CONST:
                fprintf(output, "   mov $%i, %%rax\n", instruction->left.value.literal);
                fprintf(output, "   mov %%rax, %i(%%rbp)\n", temp_offset(instruction->dest.value.temp,
                    layout.variableCount));
                break;

            case IR_LOAD:
                break;

            case IR_STORE:
                fprintf(output, "   mov %i(%%rbp), %%rax\n", temp_offset(instruction->left.value.temp,
                    layout.variableCount));
                fprintf(output, "   mov %%rax, %i(%%rbp)\n", variable_offset(instruction->dest.value.variable));
                break;

            case IR_ADD:
                fprintf(output, "   mov %i(%%rbp), %%rax\n", temp_offset(instruction->left.value.temp,
                    layout.variableCount));
                fprintf(output, "   add %i(%%rbp), %%rax\n", temp_offset(instruction->right.value.temp,
                    layout.variableCount));
                fprintf(output, "   mov %%rax, %i(%%rbp)\n", temp_offset(instruction->dest.value.temp,
                    layout.variableCount));
                break;

            case IR_SUB:
                fprintf(output, "   mov %i(%%rbp), %%rax\n", temp_offset(instruction->left.value.temp,
                    layout.variableCount));
                fprintf(output, "   sub %i(%%rbp), %%rax\n", temp_offset(instruction->right.value.temp,
                    layout.variableCount));
                fprintf(output, "   mov %%rax, %i(%%rbp)\n", temp_offset(instruction->dest.value.temp,
                    layout.variableCount));
                break;

            case IR_MUL:
                fprintf(output, "   mov %i(%%rbp), %%rax\n", temp_offset(instruction->left.value.temp,
                    layout.variableCount));
                fprintf(output, "   imul %i(%%rbp), %%rax\n", temp_offset(instruction->right.value.temp,
                    layout.variableCount));
                fprintf(output, "   mov %%rax, %i(%%rbp)\n", temp_offset(instruction->dest.value.temp,
                    layout.variableCount));;
                break;

            case IR_DIV:
                fprintf(output, "    mov %i(%%rbp), %%rax\n", temp_offset(instruction->left.value.temp,
                    layout.variableCount));
                fprintf(output, "    cqto\n");
                fprintf(output, "    idiv %i(%%rbp)\n", temp_offset(instruction->right.value.temp,
                    layout.variableCount));
                fprintf(output, "    mov %%rax, %i(%%rbp)\n", temp_offset(instruction->dest.value.temp,
                    layout.variableCount));
                break;
        }
    }

    fprintf(output, "   mov $60, %%rax\n");
    fprintf(output, "   xor %%rdi, %%rdi\n");
    fprintf(output, "   syscall\n");

    fclose(output);

    return 1;
}