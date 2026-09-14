#include <stdlib.h>
#include "ast.h"

static void ast_program_clear(AST_Program *program) {
    if (!program) return;

    for (size_t i = 0; i < program->count; i++)
        ast_node_free(program->statements[i]);

    free(program->statements);
}

void ast_program_free(AST_Program *program) {
    if (!program) return;

    ast_program_clear(program);
    free(program);
}

void ast_expression_free(AST_Expression *exp) {
    if (!exp) return;

    switch (exp->type) {
        case AST_EX_LITERAL:
            switch (exp->literal.type) {
                case AST_LITERAL_S1: free(exp->literal.s1Value); break;

                case AST_LITERAL_S2: free(exp->literal.s2Value); break;

                case AST_LITERAL_S4: free(exp->literal.s4Value); break;

                default: break;
            } break;

        case AST_EX_LOCATION: ast_expression_free(exp->location.offset); ast_expression_free(exp->location.size); break;

        case AST_EX_VARIABLE:break;

        case AST_EX_BINARY: ast_expression_free(exp->binary.left); ast_expression_free(exp->binary.right); break;

        case AST_EX_UNARY: ast_expression_free(exp->unary.operand); break;

        case AST_EX_MEMBER_ACCESS: ast_expression_free(exp->memberAccess.parent); break;

        case AST_EX_LOCATION_ACCESS: ast_expression_free(exp->locationAccess.parent); ast_expression_free(exp->locationAccess.size);
            ast_expression_free(exp->locationAccess.offset); break;

        case AST_EX_DATA_TYPE: break;

        case AST_EX_CALL: ast_expression_free(exp->call.function);
            for (size_t i = 0; i < exp->call.argumentCount; i++) ast_expression_free(exp->call.arguments[i]);
            free(exp->call.arguments); break;

        case AST_EX_SP: break;
    }
    
    free(exp);
}

void ast_field_free(AST_Field *fields, size_t count) {
    if (!fields) return;

    for (size_t i = 0; i < count; i++) {
        ast_expression_free(fields[i].offset);
        ast_expression_free(fields[i].size);
    }

    free(fields);
}

void ast_block_free(AST_Block *block) {
    if (!block) return;

    for (size_t i = 0; i < block->count; i++) ast_node_free(block->statements[i]);

    free(block->statements);
    free(block);
}

void ast_node_free(AST_Node *node) {
    if (!node) return;

    switch (node->type) {
        case AST_PROGRAM: ast_program_clear(&node->program); break;

        case AST_FUNCTION_DECLARATION: ast_expression_free(node->functionDeclaration.returnSize); 
            ast_block_free(node->functionDeclaration.body);
            ast_field_free(node->functionDeclaration.parameters, node->functionDeclaration.parameterCount); break;

        case AST_VARIABLE_DECLARATION: ast_expression_free(node->variableDeclaration.initializer); break;

        case AST_STRUCT_DECLARATION: ast_field_free(node->structDeclaration.fields, node->structDeclaration.fieldCount); break;

        case AST_ASSIGNMENT: ast_expression_free(node->assignment.target); ast_expression_free(node->assignment.value); break;

        case AST_EXPRESSION_STATEMENT: ast_expression_free(node->expressionStatement.expression); break;

        case AST_IF: ast_block_free(node->conditional.thenBody); ast_block_free(node->conditional.elseBody); 
            ast_expression_free(node->conditional.condition); break;

        case AST_WHILE: ast_block_free(node->loop.body); ast_expression_free(node->loop.condition); break;

        case AST_RETURN: ast_expression_free(node->returnStatement.value); break;

        case AST_BREAK: break;

        case AST_CONTINUE: break;

        case AST_SP_LOCATION: ast_expression_free(node->spLocation.initializer); break;

        case AST_END: break;
    }

    free(node);
}

int block_add_statement(AST_Block *block, AST_Node *statement) {
    if (!block) return 0;

    if (block->count >= block->capacity) {
        int newCapacity = block->capacity == 0 ? 8 : block->capacity * 2;

        AST_Node **newStatements = realloc(block->statements, sizeof(AST_Node *) * newCapacity);
        if (!newStatements) return 0;

        block->statements = newStatements;
        block->capacity = newCapacity;
    }

    block->statements[block->count++] = statement;
    return 1;
}

int program_add_statement(AST_Program *program, AST_Node *statement) {
    if (!program) return 0;
    
    if (program->count >= program->capacity) {
        int newCapacity = program->capacity == 0 ? 8 : program->capacity * 2;

        AST_Node **newStatements = realloc(program->statements, sizeof(AST_Node *) * newCapacity);
        if (!newStatements) return 0;

        program->statements = newStatements;
        program->capacity = newCapacity;
    }

    program->statements[program->count++] = statement;
    return 1;
}