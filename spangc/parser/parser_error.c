#include <stdio.h>
#include "parser.h"
#include "parser_error.h"

static void parser_error_at(Parser *parser, ParserErrorType type, const char *message, Token token) {
    if (parser->hasError) return;

    parser->hasError = true;

    parser->error = (ParserError) {
        .type = type,
        .token = token,
        .message = message,
    };
}

void parser_error(Parser *parser, ParserErrorType type) {
    const char *message = NULL;

    switch (type) {
        case PARSER_ERROR_UNEXPECTED_TOKEN:
            message = "unexpected token";
            break;

        case PARSER_ERROR_EXPECTED_EXPRESSION:
            message = "expected expression";
            break;

        case PARSER_ERROR_EXPECTED_IDENTIFIER:
            message = "expected identifier";
            break;

        case PARSER_ERROR_EXPECTED_SEMICOLON:
            message = "expected ';'";
            break;

        case PARSER_ERROR_EXPECTED_COMMA:
            message = "expected ','";
            break;

        case PARSER_ERROR_EXPECTED_LEFT_BRACKET:
            message = "expected '['";
            break;

        case PARSER_ERROR_EXPECTED_RIGHT_BRACKET:
            message = "expected ']'";
            break;

        case PARSER_ERROR_EXPECTED_LEFT_PAREN:
            message = "expected '('";
            break;

        case PARSER_ERROR_EXPECTED_RIGHT_PAREN:
            message = "expected ')'";
            break;

        case PARSER_ERROR_EXPECTED_LEFT_BRACE:
            message = "expected '{'";
            break;

        case PARSER_ERROR_EXPECTED_RIGHT_BRACE:
            message = "expected '}'";
            break;

        case PARSER_ERROR_EXPECTED_ASSIGNMENT:
            message = "expected '='";
            break;
            
        case PARSER_ERROR_EXPECTED_DOT:
            message = "expected '.'";
            break;

        case PARSER_ERROR_EXPECTED_BASE:
            message = "expected one of 'fp', 'sp', 'hp' or 'bp'";
            break;

        case PARSER_ERROR_INVALID_STATEMENT:
            message = "invalid statement";
            break;
        
        case PARSER_ERROR_INVALID_TOKEN:
            message = "invalid token";
            break;
        }

    parser_error_at(parser, type, message, parser->current);
}

void parser_print_error(const Parser *parser) {
    if (!parser || !parser->hasError) return;

    const ParserError *error = &parser->error;

    fprintf(stderr, "%s:%zu:%zu: error: %s", parser->lexer->path, error->token.line, error->token.column, error->message);

    fprintf(stderr, ", got '%.*s'\n", (int) error->token.length, error->token.start);
}