#pragma once
#include "lexer.h"
#include "ast.h"

typedef enum {
    PARSER_ERROR_UNEXPECTED_TOKEN,
    PARSER_ERROR_EXPECTED_EXPRESSION,
    PARSER_ERROR_EXPECTED_IDENTIFIER,
    PARSER_ERROR_EXPECTED_SEMICOLON,
    PARSER_ERROR_EXPECTED_COMMA,
    PARSER_ERROR_EXPECTED_DOT,
    PARSER_ERROR_EXPECTED_LEFT_BRACKET,
    PARSER_ERROR_EXPECTED_RIGHT_BRACKET,
    PARSER_ERROR_EXPECTED_LEFT_PAREN,
    PARSER_ERROR_EXPECTED_RIGHT_PAREN,
    PARSER_ERROR_EXPECTED_LEFT_BRACE,
    PARSER_ERROR_EXPECTED_RIGHT_BRACE,
    PARSER_ERROR_EXPECTED_ASSIGNMENT,
    PARSER_ERROR_EXPECTED_BASE,
    PARSER_ERROR_INVALID_STATEMENT,
    PARSER_ERROR_INVALID_TOKEN,
} ParserErrorType;

typedef struct {
    ParserErrorType type;
    Token token;
    const char *message;
} ParserError;

typedef struct {
    Lexer *lexer;
    Token current;
    ParserError error;
    int hasError;
} Parser;

static AST_Expression *parse_expression(Parser *parser);
static AST_Node *parse_statement(Parser *parser);
Parser parser_init(Lexer *lexer);
AST_Program *parse_program(Parser *parser);

// error
static int parser_next(Parser *parser);
static void parser_error(Parser *parser, ParserErrorType type);
void parser_print_error(const Parser *parser);