#pragma once
#include "../lexer/lexer.h"
#include "../ast/ast.h"
#include "parser_error.h"
#include "include/include_resolver.h"

typedef struct Parser {
    Lexer *lexer;
    Token current;
    ParserError error;
    IncludeResolver *resolver;
    int hasError;
} Parser;

Parser *parser_init(Lexer *lexer);
void parser_free(Parser *parser);
AST_Program *parse_program(Parser *parser);