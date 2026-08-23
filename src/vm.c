#include "vm.h"
#include "compiler.h"

void vm_run(VM *vm, Bytecode *bytecode) {
    for (int i = 0; i < bytecode->count; i++) {
        Instruction instruction = bytecode->instructions[i];

        switch (instruction.opcode) {
            case OP_PUSH:
                vm->stack[vm->sp++] = instruction.operand;
                break;

            case OP_LOAD:
                vm->stack[vm->sp++] = vm->variables[instruction.operand];
                break;

            case OP_STORE:
                vm->variables[instruction.operand] = vm->stack[--vm->sp];
                break;

            case OP_HALT:
                return;
        }
    }
}