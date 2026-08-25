#include <stdio.h>
#include "ast.h"

static void print_indent(int depth) {
    for (int i = 0; i < depth; i++)
        printf("    ");
}

static void print_node(const ASTNode *node, int depth) {
    if (!node) return;

    print_indent(depth);

    switch (node->type) {
        case AST_PROGRAM:
            printf("PROGRAM\n");
            for (int i = 0; i < node->data.program.count; i++) {
                print_node(node->data.program.statements[i], depth + 1);
            }
            break;

        case AST_VARIABLE_DECLARATION:
            printf("VARIABLE_DECLARATION %.*s\n", node->data.variable.length, node->data.variable.start);
            break;

        case AST_VARIABLE:
            printf("VARIABLE %.*s\n", node->data.variable.length,node->data.variable.start);
            break;

        case AST_LITERAL:
            printf("LITERAL %d\n", node->data.literal.value);
            break;

        case AST_ASSIGNMENT:
            printf("ASSIGNMENT\n");

            print_node(node->data.binary.left, depth + 1);

            print_node(node->data.binary.right, depth + 1);

            break;


        case AST_OPERATOR:
            printf("OPERATOR ");

            switch (node->data.operation.operatorType) {
                case OP_PLUS: printf("+"); break;

                case OP_MINUS: printf("-"); break;

                case OP_MULTIPLY: printf("*"); break;

                case OP_DIVIDE: printf("/"); break;
            }

            printf("\n");

            print_node(node->data.operation.left, depth + 1);

            print_node(node->data.operation.right, depth + 1);

            break;

        default: break;
    }
}

void ast_print(const ASTNode *program) {
    printf("=== AST PRINT ===\n");
    print_node(program, 0);
    printf("=== END ===\n\n");
}