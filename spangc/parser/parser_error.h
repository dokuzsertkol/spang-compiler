#pragma once
#include "../token/token.h"

typedef struct Parser Parser;

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
    PARSER_ERROR_INVALID_INCLUDE,
    PARSER_ERROR_CIRCULAR_INCLUDE,
} ParserErrorType;

typedef struct {
    ParserErrorType type;
    const char *tokenStr;
    const char *message;

    bool showToken; 

    char *path;
    size_t line;
    size_t column;
} ParserError;

void parser_error(Parser *parser, ParserErrorType type);
void parser_print_error(const Parser *parser);