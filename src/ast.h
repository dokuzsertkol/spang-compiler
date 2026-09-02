#pragma once
#include <stddef.h>
#include <stdbool.h>

typedef struct AST_Node AST_Node;
typedef struct AST_Expression AST_Expression;

// data type
typedef enum {
    AST_DATA_INT,
    AST_DATA_CHAR,
    AST_DATA_BOOL,
    AST_DATA_FLOAT,
    AST_DATA_VOID,
} AST_DataType;

// literal
typedef struct {
    AST_DataType type;
    union {
        long intValue;
        char charValue;
        bool boolValue;
        double floatValue;
    };
} AST_Literal;

// adress
typedef enum {
    AST_LOCATION_SP,
    AST_LOCATION_FP,
    AST_LOCATION_HP,
    AST_LOCATION_BP,
} AST_LocationType;

typedef struct {
    AST_LocationType base;
    AST_Expression *offset;
    AST_Expression *size;
} AST_Location;

// variable
typedef struct {
    const char *name;
    size_t length;
} AST_Variable;

typedef struct {
    const char *name;
    size_t length;
    AST_Location location;
    AST_Expression *initializer;
} AST_VariableDeclaration;

// operator
typedef enum {
    AST_OP_PLUS,
    AST_OP_MINUS,
    AST_OP_MULTIPLY,
    AST_OP_DIVIDE,
    AST_OP_EQUAL,
    AST_OP_NOT_EQUAL,
    AST_OP_LESS,
    AST_OP_LESS_EQUAL,
    AST_OP_GREATER,
    AST_OP_GREATER_EQUAL,
} AST_OperatorType;

// expression
typedef enum {
    AST_EX_LITERAL,
    AST_EX_LOCATION,
    AST_EX_VARIABLE,
    AST_EX_BINARY,
    AST_EX_CALL,
    AST_EX_UNARY,
} AST_ExpressionType;

struct AST_Expression {
    AST_ExpressionType type;
    union {
        AST_Literal literal;
        AST_Location location;
        AST_Variable variable;

        struct {
            AST_Expression *operand;
            AST_OperatorType op;
        } unary;

        struct {
            AST_Expression *left;
            AST_Expression *right;
            AST_OperatorType op;
        } binary;

        struct {
            AST_Expression *function;
            AST_Expression **arguments;
            size_t argumentCount;
        } call;
    };
};

// block
typedef struct {
    AST_Node **statements;
    size_t capacity;
    size_t count;
} AST_Block;

// conditional
typedef struct {
    AST_Expression *condition;
    AST_Block *thenBody;
    AST_Block *elseBody;
} AST_Conditional;

// loop
typedef struct {
    AST_Expression *condition;
    AST_Block *body;
} AST_Loop;

// function
typedef struct {
    const char *name;
    AST_DataType type;
} AST_Parameter;

typedef struct {
    const char *name;

    AST_DataType returnType;

    AST_Parameter *parameters;
    size_t parameterCount;

    AST_Block *body;
} AST_Function;

typedef struct {
    AST_Expression *value;
} AST_Return;

// assignment
typedef struct {
    AST_Expression *target;
    AST_Expression *value;
} AST_Assignment;

// program
typedef struct {
    AST_Block block;
} AST_Program;

// node
typedef enum {
    AST_PROGRAM,
    AST_BLOCK,

    AST_FUNCTION,
    AST_VARIABLE_DECLARATION,
    AST_ASSIGNMENT,

    AST_IF,
    AST_WHILE,
    AST_RETURN,
    AST_BREAK,

    AST_END,
} AST_NodeType;

struct AST_Node {
    AST_NodeType type;
    union {
        AST_Program program;
        AST_Block block;

        AST_Function function;
        AST_Loop loop;
        AST_Conditional conditional;
        AST_Expression expression;
        AST_Assignment assignment;
        AST_VariableDeclaration variableDeclaration;
        AST_Return returnStatement;
    };
};

//debug
void ast_print(const AST_Program *program);