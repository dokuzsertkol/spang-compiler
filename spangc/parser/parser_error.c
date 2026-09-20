#include <string.h>
#include <stdio.h>
#include "parser.h"
#include "parser_error.h"

static void parser_error_at(Parser *parser, ParserErrorType type, const char *message, Token token) {
    if (parser->hasError) return;

    parser->hasError = true;

    parser->error = (ParserError) {
        .type = type,
        .line = token.line,
        .column = token.column,
        .path = strdup(parser->lexer->path),
        .tokenStr = strndup(token.start, token.length),
        .message = message,
    };
}

void parser_error(Parser *parser, ParserErrorType type) {
    const char *message = NULL;
    bool showToken = false;

    switch (type) {
        case PARSER_ERROR_UNEXPECTED_TOKEN:
            message = "unexpected token";
            showToken = true;
            break;

        case PARSER_ERROR_EXPECTED_EXPRESSION:
            message = "expected expression";
            showToken = true;
            break;

        case PARSER_ERROR_EXPECTED_IDENTIFIER:
            message = "expected identifier";
            showToken = true;
            break;

        case PARSER_ERROR_EXPECTED_SEMICOLON:
            message = "expected ';'";
            showToken = true;
            break;

        case PARSER_ERROR_EXPECTED_COMMA:
            message = "expected ','";
            showToken = true;
            break;

        case PARSER_ERROR_EXPECTED_LEFT_BRACKET:
            message = "expected '['";
            showToken = true;
            break;

        case PARSER_ERROR_EXPECTED_RIGHT_BRACKET:
            message = "expected ']'";
            showToken = true;
            break;

        case PARSER_ERROR_EXPECTED_LEFT_PAREN:
            message = "expected '('";
            showToken = true;
            break;

        case PARSER_ERROR_EXPECTED_RIGHT_PAREN:
            message = "expected ')'";
            showToken = true;
            break;

        case PARSER_ERROR_EXPECTED_LEFT_BRACE:
            message = "expected '{'";
            showToken = true;
            break;

        case PARSER_ERROR_EXPECTED_RIGHT_BRACE:
            message = "expected '}'";
            showToken = true;
            break;

        case PARSER_ERROR_EXPECTED_ASSIGNMENT:
            message = "expected '='";
            showToken = true;
            break;

        case PARSER_ERROR_EXPECTED_DOT:
            message = "expected '.'";
            showToken = true;
            break;

        case PARSER_ERROR_EXPECTED_BASE:
            message = "expected one of 'fp', 'sp', 'hp' or 'bp'";
            showToken = true;
            break;

        case PARSER_ERROR_INVALID_STATEMENT:
            message = "invalid statement";
            break;

        case PARSER_ERROR_INVALID_TOKEN:
            message = "invalid token";
            showToken = true;
            break;

        case PARSER_ERROR_INVALID_INCLUDE:
            message = "invalid include";
            break;

        case PARSER_ERROR_CIRCULAR_INCLUDE:
            message = "circular include";
            break;
    }

    parser_error_at(parser, type, message, parser->current);
    parser->error.showToken = showToken;
}

void parser_print_error(const Parser *parser) {
    if (!parser || !parser->hasError) return;

    const ParserError *error = &parser->error;

    fprintf(stderr, "%s:%zu:%zu: error: %s", error->path, error->line, error->column, error->message);

    if (error->showToken) fprintf(stderr, ", got '%s'", error->tokenStr);

    fprintf(stderr, "\n");
}