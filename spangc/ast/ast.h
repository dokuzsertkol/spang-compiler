#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct AST_Node AST_Node;
typedef struct AST_Expression AST_Expression;

// literal
typedef enum {
    AST_LITERAL_INT,
    AST_LITERAL_FLOAT,
    AST_LITERAL_BOOL,
    AST_LITERAL_C1,
    AST_LITERAL_C2,
    AST_LITERAL_C4,
    AST_LITERAL_S1,
    AST_LITERAL_S2,
    AST_LITERAL_S4,
} AST_LiteralType;

typedef struct {
    AST_LiteralType type;
    size_t length;
    union {
        uint64_t intValue;
        double floatValue;
        bool boolValue;

        uint8_t c1Value;
        uint16_t c2Value;
        uint32_t c4Value;

        uint8_t *s1Value;
        uint16_t *s2Value;
        uint32_t *s4Value;
    };
} AST_Literal;

// location
typedef enum {
    AST_LOCATION_SP,
    AST_LOCATION_FP,
    AST_LOCATION_HP,
    AST_LOCATION_BP,
} AST_BaseType;

typedef struct {
    AST_BaseType base;
    AST_Expression *offset;
    AST_Expression *size;
    AST_Expression *readAs;
} AST_Location;

// variable
typedef struct {
    const char *name;
    size_t length;
} AST_Variable;

typedef struct {
    AST_Variable var;
    AST_Location location;
    AST_Expression *initializer;
} AST_VariableDeclaration;

// field
typedef struct {
    const char *name;
    size_t length;
    AST_Expression *offset;
    AST_Expression *size;
    AST_Expression *readAs;
} AST_Field;

// struct
typedef struct {
    const char *name;
    size_t length;
    AST_Field *fields;
    size_t fieldCount;
} AST_StructDeclaration;

// location access
typedef struct {
    AST_Expression *offset;
    AST_Expression *size;
    AST_Expression *readAs;
    AST_Expression *parent;
} AST_LocationAccess;

// member access
typedef struct {
    const char *name;
    size_t length;
    AST_Expression *parent;
} AST_MemberAccess;

// block
typedef struct {
    AST_Node **statements;
    size_t capacity;
    size_t count;
} AST_Block;

// function
typedef struct {
    const char *name;
    size_t length;

    AST_Expression *returnSize;

    AST_Field *parameters;
    size_t parameterCount;

    AST_Block *body;
} AST_FunctionDeclaration;

// operator
typedef enum {
    AST_OP_PLUS,
    AST_OP_MINUS,
    AST_OP_MULTIPLY,
    AST_OP_DIVIDE,
    AST_OP_EQUAL,
    AST_OP_NOT,
    AST_OP_NOT_EQUAL,
    AST_OP_LESS,
    AST_OP_LESS_EQUAL,
    AST_OP_GREATER,
    AST_OP_GREATER_EQUAL,
    AST_OP_AND,
    AST_OP_OR,
} AST_OperatorType;

// data types
typedef enum {
    AST_DATA_I1,
    AST_DATA_I2,
    AST_DATA_I4,
    AST_DATA_I8,
    AST_DATA_U1,
    AST_DATA_U2,
    AST_DATA_U4,
    AST_DATA_U8,
    AST_DATA_C1,
    AST_DATA_C2,
    AST_DATA_C4,
    AST_DATA_F4,
    AST_DATA_F8,
    AST_DATA_B1,
    AST_DATA_V0,
} AST_DataType;

// sp
typedef struct {
    AST_Location location;
    AST_Expression *initializer;
} AST_SPLocation;

// expression
typedef enum {
    AST_EX_LITERAL,
    AST_EX_LOCATION,
    AST_EX_VARIABLE,
    AST_EX_BINARY,
    AST_EX_CALL,
    AST_EX_UNARY,
    AST_EX_MEMBER_ACCESS,
    AST_EX_LOCATION_ACCESS,
    AST_EX_DATA_TYPE,
    AST_EX_SP,
} AST_ExpressionType;

struct AST_Expression {
    AST_ExpressionType type;
    union {
        AST_Literal literal;
        AST_Location location;
        AST_Variable variable;
        AST_MemberAccess memberAccess;
        AST_LocationAccess locationAccess;
        AST_DataType dataType;

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

typedef struct {
    AST_Expression *expression;
} AST_ExpressionStatement;

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
    AST_Node **statements;
    size_t capacity;
    size_t count;
} AST_Program;

// node
typedef enum {
    AST_PROGRAM,

    AST_FUNCTION_DECLARATION,
    AST_VARIABLE_DECLARATION,
    AST_STRUCT_DECLARATION,

    AST_ASSIGNMENT,
    AST_EXPRESSION_STATEMENT,
    AST_SP_LOCATION,

    AST_IF,
    AST_WHILE,
    AST_RETURN,
    AST_BREAK,
    AST_CONTINUE,

    AST_END,
} AST_NodeType;

struct AST_Node {
    AST_NodeType type;
    union {
        AST_Program program;

        AST_FunctionDeclaration functionDeclaration;
        AST_Loop loop;
        AST_Conditional conditional;
        AST_Expression expression;
        AST_ExpressionStatement expressionStatement;
        AST_Assignment assignment;
        AST_VariableDeclaration variableDeclaration;
        AST_Return returnStatement;
        AST_StructDeclaration structDeclaration;
        AST_SPLocation spLocation;
    };
};

static void ast_program_clear(AST_Program *program);
void ast_expression_free(AST_Expression *exp);
void ast_field_free(AST_Field *fields, size_t count);
void ast_block_free(AST_Block *block);
void ast_program_free(AST_Program *program);
void ast_node_free(AST_Node *node);
int block_add_statement(AST_Block *block, AST_Node *statement);
int program_add_statement(AST_Program *program, AST_Node *statement);

// debug
#define AST_PRINT_MAX_DEPTH 256
typedef struct {
    bool branch[AST_PRINT_MAX_DEPTH];
    size_t depth;
} AST_PrintContext;

static void node_print(AST_Node *node, AST_PrintContext *ctx, bool last);
static void expression_print(AST_Expression *expression, AST_PrintContext *ctx, bool last);
static void location_print(AST_Location *location, AST_PrintContext *ctx, bool last);
static void block_print(AST_Block *block, AST_PrintContext *ctx, bool last);
static void field_print(AST_Field *field, AST_PrintContext *ctx, bool last);
void program_print(AST_Program *program);