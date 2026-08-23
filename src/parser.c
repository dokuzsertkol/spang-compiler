#include <stdlib.h> 
#include "parser.h"
#include "ast.h"
#include "lexer.h"

static int identifier_exists(const ASTNode *program, const Token* token) {
    for (int i = 0; i < program->data.program.count; i++) {
        ASTNode *variable = program->data.program.statements[i];

        if (variable->type != AST_VARIABLE || variable->data.variable.length != token->length) continue;

        int equal = 1;

        for (int j = 0; j < variable->data.variable.length; j++) {
            if (variable->data.variable.start[j] != token->start[j]) {
                equal = 0;
                break;
            }
        }

        if (equal) return 1;
    }
    return 0;
}

static int program_add_statement(ASTNode *program, ASTNode *statement) {
    if (program->data.program.count >= program->data.program.capacity) {
        int new_capacity = program->data.program.capacity * 2;

        ASTNode **new_statements = realloc(
            program->data.program.statements,
            sizeof(ASTNode *) * new_capacity
        );

        if (!new_statements) return 0;

        program->data.program.statements = new_statements;
        program->data.program.capacity = new_capacity;
    }

    program->data.program.statements[program->data.program.count++] = statement;
    return 1;
}

static ASTNode* parse_factor(Token *tokens, int count, int *index) { // returns literals
    if (*index >= count) return NULL;

    Token *token = &tokens[*index];

    if (token->type != TOKEN_LITERAL) return NULL;

    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return NULL;

    node->type = AST_LITERAL;
    node->data.literal.value = token_to_int(token);

    (*index)++;

    return node;
}

static ASTNode* parse_term(Token *tokens, int count, int *index) {
    ASTNode *left = parse_factor(tokens, count, index);
    if (!left) return NULL; // return if next factor is not literal

    while (*index < count) {
        Token *token = &tokens[*index];

        if (token->type != TOKEN_MULTIPLY && token->type != TOKEN_DIVIDE) break;  // if next token is not divide or multiply, return literal

        (*index)++;

        ASTNode *right = parse_factor(tokens, count, index); // return if next token is not literal 
        if (!right) return NULL;


        ASTNode *node = malloc(sizeof(ASTNode));
        if (!node) return NULL;

        node->type = AST_OPERATOR;
        node->data.operation.left = left;
        node->data.operation.right = right;
        if (token->type == TOKEN_MULTIPLY) {
            node->data.operation.operatorType = OP_MULTIPLY;
        }
        if (token->type == TOKEN_DIVIDE) {
            node->data.operation.operatorType = OP_DIVIDE;
        }

        left = node; // returns x * y or x / y node
    }

    return left;
}

static ASTNode* parse_expression(Token *tokens, int count, int *index) {
    ASTNode* left = parse_term(tokens, count, index);
    if (!left) return NULL;

    while (*index < count) {
        Token *token = &tokens[*index];

        if (token->type != TOKEN_PLUS && token->type != TOKEN_MINUS) break;

        (*index)++;

        ASTNode *right = parse_term(tokens, count, index);
        if (!right) return NULL;

        ASTNode *node = malloc(sizeof(ASTNode));
        if (!node) return NULL;

        node->type = AST_OPERATOR;
        node->data.operation.left = left;
        node->data.operation.right = right;
        if (token->type == TOKEN_PLUS) {
            node->data.operation.operatorType = OP_PLUS;
        }
        if (token->type == TOKEN_MINUS) {
            node->data.operation.operatorType = OP_MINUS;
        }

        left = node;
    }

    return left;
}

static int parse_assignment(ASTNode *program, Lexer *lexer, Token *token) {
    Token tokens[128];
    int count = 0;

    Token variable = *token;

    *token = lexer_next_token(lexer);

    while (token->type != TOKEN_ENDLINE && token->type != TOKEN_EOF) {
        if (count >= 128) return 0;

        tokens[count++] = *token;

        *token = lexer_next_token(lexer);
    }

    if (token->type == TOKEN_EOF) return 0;

    int index = 0;
    ASTNode *expression = parse_expression(tokens, count, &index);
    if (expression == NULL || index != count) return 0;
    
    ASTNode *left = malloc(sizeof(ASTNode));
    if (left == NULL) return 0;
    *left = (ASTNode){
        .type = AST_VARIABLE,
        .data.variable = {
            .start = variable.start,
            .length = variable.length,
        },
    };

    ASTNode *node = malloc(sizeof(ASTNode));
    if (node == NULL) return 0;
    *node = (ASTNode){
        .type = AST_ASSIGNMENT,
        .data.binary = {
            .left = left,
            .right = expression,
        }
    };

    if (!program_add_statement(program, node)) {
        free(node);
        return 0;
    }
    return 1;
}

static int parse_variable_declaration(ASTNode *program, Lexer *lexer, Token *token) {
    *token = lexer_next_token(lexer);
    if (token->type != TOKEN_IDENTIFIER || identifier_exists(program, token)) return 0;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (node == NULL) return 0;

    *node = (ASTNode){
        .type = AST_VARIABLE,
        .data.variable = {
            .start = token->start,
            .length = token->length,
        },
    };

    *token = lexer_next_token(lexer);
    if (token->type != TOKEN_ENDLINE) {
        free(node);
        return 0;
    }
    
    if (!program_add_statement(program, node)) {
        free(node);
        return 0;
    }
    return 1;
}

ASTNode parse_program(Lexer *lexer) {
    ASTNode program = {
        .type = AST_PROGRAM,
        .data.program = {
            .statements = malloc(sizeof(ASTNode *) * 8),
            .count = 0,
            .capacity = 8
        }
    };
    if (!program.data.program.statements) return program;

    Token token;
    do {
        token = lexer_next_token(lexer);

        if (token.type == TOKEN_LET &&  !parse_variable_declaration(&program, lexer, &token)) break;

        if (token.type == TOKEN_IDENTIFIER) {
            if (!identifier_exists(&program, &token)) break;

            token = lexer_next_token(lexer);
            if (token.type == TOKEN_EQUAL) {
                if(!parse_assignment(&program, lexer, &token)) break;
            }
            else break;
        }

        // case TOKEN_EOF: case TOKEN_ENDLINE: case TOKEN_EQUAL: case TOKEN_IDENTIFIER: case TOKEN_PLUS: case TOKEN_NUMBER: break;
    }
    while(token.type != TOKEN_EOF);

    return program;
}