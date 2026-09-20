#pragma once
#include "../lexer/lexer.h"
#include "../ast/ast.h"
#include "parser_error.h"
#include "include/include_resolver.h"

typedef struct Parser {
    Lexer *lexer;
    Token current;

    IncludeResolver *resolver;

    ParserError error;
    int hasError;

    SourceManager *manager;
} Parser;

Parser *parser_init(SourceManager *manager, Lexer *lexer);
void parser_free(Parser *parser);
AST_Program *parse_program(Parser *parser);