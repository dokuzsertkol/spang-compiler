#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include <stdio.h>

static int token_to_int(Token* token) {
    int result = 0;

    for (int i = 0; i < token->length; i++) {
        result = result * 10 + (*(token->start + i) - '0');
    }

    return result;
}

static char token_to_char(Token* token) {
    return *token->start;
}

static int check_identifier(ASTProgram *program, Token* token) {
    int result = 0;
    for (int i = 0; i < program->variableCount; i++) {
        ASTVariable *var = &program->variables[i];

        if (var->length != token->length) continue;

        int equal = 1;

        for (int j = 0; j < var->length; j++) {
            if (var->start[j] != token->start[j]) {
                equal = 0;
                break;
            }
        }

        if (equal) return 1;
    }
    if (!result) printf("%s", "IDENTIFIER NOT FOUND");
    return result;
}

static int parse_line(ASTProgram *program, Lexer *lexer) {
    Token token = lexer_next_token(lexer);

    // parse new variable
    if (token.type == TOKEN_LET) {
        token = lexer_next_token(lexer);
        if (token.type == TOKEN_IDENTIFIER) {
            ASTVariable var = {AST_VARIABLE, token.start, token.length};
            token = lexer_next_token(lexer);
            if (token.type == TOKEN_ENDLINE) {
                program->variables[program->variableCount++] = var;
                return 1;
            }
        }
        printf("%s", "LET PARSE ERROR");
    }

    // parse variable assign
    if (token.type == TOKEN_IDENTIFIER) {
        ASTNode node;
        if (!check_identifier(program, &token)) return -1;
        node = (ASTNode){AST_VARIABLE, 0, token.start, token.length, NULL, NULL};
        program->nodes[program->nodeCount] = node;

        token = lexer_next_token(lexer);
        if (token.type == TOKEN_EQUAL) {
            ASTNode top = {AST_OPERATOR, token_to_char(&token), 0, 0, &program->nodes[program->nodeCount++], NULL};

            token = lexer_next_token(lexer);
            // if equal number
            if (token.type == TOKEN_NUMBER) {
                node = (ASTNode){AST_NUMBER, token_to_int(&token), 0, 0, NULL, NULL};
                program->nodes[program->nodeCount] = node;
                top.right = &program->nodes[program->nodeCount++];
                
                token = lexer_next_token(lexer);
                if (token.type == TOKEN_ENDLINE) {
                    program->statements[program->statementCount++] = top;
                    return 1;
                }
            }
            // if equal identifier
            if (token.type == TOKEN_IDENTIFIER) {
                if (!check_identifier(program, &token)) return -1;
                node = (ASTNode){AST_VARIABLE, 0, token.start, token.length, NULL, NULL};
                program->nodes[program->nodeCount] = node;
                top.right = &program->nodes[program->nodeCount++];

                token = lexer_next_token(lexer);
                if (token.type == TOKEN_ENDLINE) {
                    program->statements[program->statementCount++] = top;
                    return 1;
                }
            }
        }
        printf("%s", "ASSIGN PARSE ERROR");
    }

    // parse eof
    if (token.type == TOKEN_EOF) {
        program->statements[program->statementCount++] = (ASTNode){AST_END};
    }
    return -1;
}

ASTProgram parse_program(Lexer *lexer) {
    ASTProgram program = {0};
    program.type = AST_PROGRAM;

    while (parse_line(&program, lexer) != -1);
    
    return program;
}