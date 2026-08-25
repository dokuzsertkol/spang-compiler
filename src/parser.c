#include <stdlib.h> 
#include "parser.h"
#include "ast.h"
#include "lexer.h"

static int program_add_statement(ASTNode *program, ASTNode *statement) {
    if (program->data.program.count >= program->data.program.capacity) {
        int newCapacity = program->data.program.capacity == 0 ? 8 :program->data.program.capacity * 2;

        ASTNode **newStatements = realloc(program->data.program.statements, sizeof(ASTNode *) * newCapacity);

        if (!newStatements) return 0;

        program->data.program.statements = newStatements;
        program->data.program.capacity = newCapacity;
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

static int parse_assignment(ASTNode *program, Lexer *lexer, Token *token, Token *identifier) {
    Token tokens[128];
    int count = 0;

    Token variable = *identifier;

    *token = lexer_next_token(lexer);

    while (token->type != TOKEN_ENDLINE && token->type != TOKEN_EOF) {
        if (count >= 128) return 0;

        tokens[count++] = *token;

        *token = lexer_next_token(lexer);
    }

    if (token->type == TOKEN_EOF) return 0;

    int index = 0;
    ASTNode *expression = parse_expression(tokens, count, &index);
    if (!expression || index != count) return 0;
    
    ASTNode *left = malloc(sizeof(ASTNode));
    if (!left) return 0;
    *left = (ASTNode){
        .type = AST_VARIABLE,
        .data.variable = {
            .start = variable.start,
            .length = variable.length,
        },
    };

    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return 0;
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
    if (token->type != TOKEN_IDENTIFIER) return 0;
    
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) return 0;

    *node = (ASTNode){
        .type = AST_VARIABLE_DECLARATION,
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
        .data.program = {0},
    };

    Token token;
    while (1) {
        token = lexer_next_token(lexer);

        if (token.type == TOKEN_EOF) break;

        if (token.type == TOKEN_LET) {
            if (!parse_variable_declaration(&program, lexer, &token)) break;
        }
        else if (token.type == TOKEN_IDENTIFIER) {
            Token identifier = token;
            token = lexer_next_token(lexer);

            if (token.type == TOKEN_EQUAL) {
                if (!parse_assignment(&program, lexer, &token, &identifier)) break;
            }
            else break;
        }
        else break;
    }

    return program;
}