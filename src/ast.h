#pragma once

typedef enum {
    AST_PROGRAM,
    AST_VARIABLE_DECLARATION,
    AST_VARIABLE,
    AST_LITERAL,
    AST_OPERATOR,
    AST_ASSIGNMENT,
    AST_END,
} ASTType;

typedef enum {
    OP_PLUS,
    OP_MINUS,
    OP_MULTIPLY,
    OP_DIVIDE,
} OperatorType;

typedef struct ASTNode {
    ASTType type;

   union {
        struct {
            int value;
        } literal;

        struct {
            const char *start;
            int length;
        } variable;

        struct {
            OperatorType operatorType;
            struct ASTNode *left;
            struct ASTNode *right;
        } operation;
        
        struct {
            struct ASTNode *left;
            struct ASTNode *right;
        } binary;

        struct {
            struct ASTNode **statements;
            int capacity;
            int count;
        } program;
    } data;

} ASTNode;

//debug
void ast_print(const ASTNode *program);