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

Parser parser_init(Lexer *lexer);
AST_Program *parse_program(Parser *parser);