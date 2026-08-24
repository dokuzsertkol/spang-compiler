#include <stdlib.h>
#include <string.h>
#include "semantic_analyser.h"
#include "ast.h"

static int symbol_exists(const SymbolTable *table, const char* name) {
    for (int i = 0; i < table->count; i++) {
        if (strcmp(name, table->symbols[i].name) == 0) return 1;
    }
    return 0;
}

static int symbol_add(SymbolTable *table, const char* name) {
    if (symbol_exists(table, name)) return 0;

    Symbol symbol = {name};

    if (table->count >= table->capacity) {
        int newCapacity = table->capacity == 0 ? 8 : table->capacity * 2;

        Symbol *newSymbols = realloc(table->symbols, sizeof(*table->symbols) * newCapacity);
        if (!newSymbols) return 0;

        table->symbols = newSymbols;
        table->capacity = newCapacity;
    }

    table->symbols[table->count++] = symbol;
    return 1;
}

static int analyse_expression(SymbolTable *table, const ASTNode *expression) {
    switch (expression->type) {
        case AST_LITERAL: return 1;

        case AST_VARIABLE: return symbol_exists(table, expression->data.variable.start);

        case AST_OPERATOR:
            return analyse_expression(table, expression->data.operation.left) 
                && analyse_expression(table, expression->data.operation.right);

        default: return 0;
    }
}

static int analyse_assignment(SymbolTable *table, const ASTNode* assignment) {
    if (!symbol_exists(table, assignment->data.binary.left->data.variable.start)) return 0;

    ASTNode expression = *assignment->data.binary.right;

    while (expression.type == AST_OPERATOR) {
        if (expression.data.operation.left->type != AST_LITERAL) return 0;
        if (expression.data.operation.right->type == AST_OPERATOR) {
            expression = *expression.data.operation.right;
        }
        else if (expression.data.operation.right->type == AST_LITERAL) break;
        else return 0;
    }
    return 1;
}

int semantic_analyse(const ASTNode *program) {
    SymbolTable table = {0};

    for (int i = 0; i < program->data.program.count; i++) {
        const ASTNode statement = *program->data.program.statements[i];
        
        if (statement.type == AST_VARIABLE_DECLARATION) {
            if (!symbol_add(&table, statement.data.variable.start)) return 0;
        }
        if (statement.type == AST_ASSIGNMENT) {
            if (! analyse_assignment(&table, &statement)) return 0;
        }
    }
    return 1;
}