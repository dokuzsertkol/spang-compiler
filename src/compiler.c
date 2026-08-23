#include "compiler.h"
#include "ast.h"
#include <stdio.h>

static int find_identifier_index(ASTProgram *program, ASTNode *node) {
    for (int i = 0; i < program->variableCount; i++) {
        ASTVariable *var = &program->variables[i];

        if (var->length != node->length)continue;

        int equal = 1;

        for (int j = 0; j < var->length; j++) {
            if (var->start[j] != node->name[j]) {
                equal = 0;
                break;
            }
        }

        if (equal) return i;
    }

    return -1;
}

Bytecode compile(ASTProgram *program) {
    Bytecode bytecode = {0};

    for (int i = 0; i < program->statementCount; i++) {
        ASTNode* node = &program->statements[i];

        if (node->type == AST_OPERATOR) {
            if (node->value == '=') {
                // right side
                if (node->right->type == AST_NUMBER) {
                    Instruction instruction = {OP_PUSH, node->right->value};
                    bytecode.instructions[bytecode.count++] = instruction;
                }
                else if (node->right->type == AST_VARIABLE) {
                    int index = find_identifier_index(program, node->right);
                    if (index == -1) {
                        printf("%s", "COMPILE ERROR");
                        return bytecode;
                    }
                    Instruction instruction = {OP_LOAD, index};
                    bytecode.instructions[bytecode.count++] = instruction;
                }
                else printf("%s", "COMPILE ERROR");

                // left side
                if (node->left->type == AST_VARIABLE) {
                    int index = find_identifier_index(program, node->left);
                    if (index == -1) {
                        printf("%s", "COMPILE ERROR");
                        return bytecode;
                    }
                    Instruction instruction = {OP_STORE, index};
                    bytecode.instructions[bytecode.count++] = instruction;
                }
                else printf("%s", "COMPILE ERROR");
            }
        }

        if (node->type == AST_END) {
            Instruction instruction = {OP_HALT};
            bytecode.instructions[bytecode.count++] = instruction;
        }
    }
    return bytecode;
}