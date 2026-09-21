#include <stdlib.h>
#include <string.h>
#include "semantic.h"
#include "semantic_error.h"
#include <stdio.h>

SemanticAnalyser *semantic_analyser_init() {
    SemanticAnalyser *analyser = malloc(sizeof(*analyser));
    *analyser = (SemanticAnalyser){0};
    return analyser;
}

static int symbol_exists(const SymbolTable *table, const char* name, int length) {
    for (int i = 0; i < table->count; i++) {
        if (table->symbols[i].length != length) continue;
        if (strncmp(name, table->symbols[i].name, length) == 0) return 1;
    }
    return 0;
}

static int symbol_add(SemanticAnalyser *analyser, char* name, size_t length, SymbolType type, const SourceLocation *source) {
    printf("%.*s", (int)length, name);
    if (symbol_exists(&analyser->table, name, length)) {
        semantic_error(analyser, SEMANTIC_ERROR_REDECLARATION, source, name, length);
        return 0;
    }

    Symbol symbol = {type, name, length};

    if (analyser->table.count >= analyser->table.capacity) {
        size_t newCapacity = analyser->table.capacity == 0 ? 8 : analyser->table.capacity * 2;

        Symbol *newSymbols = realloc(analyser->table.symbols, sizeof(*analyser->table.symbols) * newCapacity);
        if (!newSymbols) return 0;

        analyser->table.symbols = newSymbols;
        analyser->table.capacity = newCapacity;
    }
    analyser->table.symbols[analyser->table.count++] = symbol;
    return 1;
}

static int semantic_analyse_node(SemanticAnalyser *analyser, const AST_Node *node) {
    switch (node->type) {
        case AST_PROGRAM:
            return semantic_analyse(analyser, &node->program);

        case AST_VARIABLE_DECLARATION:
            // return semantic_analyse_variable_declaration(analyser, node);

        case AST_FUNCTION_DECLARATION:
            // return semantic_analyse_function_declaration(analyser, node);

        case AST_STRUCT_DECLARATION:
            // return semantic_analyse_struct_declaration(analyser, node);

        case AST_ASSIGNMENT:
            // return semantic_analyse_assignment(analyser, node);

        case AST_EXPRESSION_STATEMENT:
            // return semantic_analyse_expression(analyser, node->expressionStatement.expression);

        case AST_IF:
            // return semantic_analyse_if(analyser, node);

        case AST_WHILE:
            // return semantic_analyse_while(analyser, node);

        case AST_RETURN:
            // return semantic_analyse_return(analyser, node);

        case AST_BREAK:
        case AST_CONTINUE:
        case AST_END:
        case AST_INCLUDE:
        case AST_SP_LOCATION:
            return 1;
    }

    return 1;
}

int semantic_analyse(SemanticAnalyser *analyser, const AST_Program *program) {
    for (int i = 0; i < program->count && !analyser->hasError; i++) {
        semantic_analyse_node(analyser, program->statements[i]);
    }
    return !analyser->hasError;
}