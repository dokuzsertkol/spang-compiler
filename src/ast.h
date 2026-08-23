#pragma once

typedef enum {
    AST_PROGRAM,
    AST_VARIABLE,
    AST_NUMBER,
    AST_OPERATOR,
    AST_END,
} ASTType;

typedef struct ASTNode {
    ASTType type;

    int value;
    const char* name;
    int length;

    struct ASTNode *left;
    struct ASTNode *right;
} ASTNode;

typedef struct ASTVariable {
    ASTType type;
    const char *start;
    int length;
} ASTVariable;

typedef struct {
    ASTType type;

    int variableCount, nodeCount, statementCount;
    struct ASTVariable variables[256];
    struct ASTNode nodes[1024];
    struct ASTNode statements[256];
} ASTProgram;