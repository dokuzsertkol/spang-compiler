#pragma once
#include "ast.h"

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