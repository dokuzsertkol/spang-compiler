#pragma once
#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer *lexer;
    Token current;
} Parser;

static AST_Expression *parse_expression(Parser *parser);
Parser parser_init(Lexer *lexer);
AST_Program *parse_program(Parser *parser);