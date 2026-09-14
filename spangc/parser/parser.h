#pragma once
#include "../lexer/lexer.h"
#include "../ast/ast.h"
#include "parser_error.h"

typedef struct Parser {
    Lexer *lexer;
    Token current;
    ParserError error;
    int hasError;
} Parser;

static AST_Expression *parse_expression(Parser *parser);
static AST_Node *parse_statement(Parser *parser);
Parser parser_init(Lexer *lexer);
AST_Program *parse_program(Parser *parser);