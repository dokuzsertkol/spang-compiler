#include "lexer.h"
#include <string.h>

Lexer lexer_init(const char *source) {
    Lexer lexer;

    lexer.source = source;
    lexer.current = source;

    return lexer;
}

Token lexer_next_token(Lexer *lexer) {
    Token token;

    token.start = lexer->current;
    token.length = 0;
    
    char c = *token.start;

    // whitespace skipper
    while (c == ' ' || c == '\n' || c == '\t') {
        lexer->current++;
        c = *(++token.start);
    }

    // line end lexer
    if (c == '.') {
        token.type = TOKEN_ENDLINE;
        token.length = 1;
        lexer->current++;
        return token;
    }

    // string lexer
    if (c >= 'a' && c <= 'z') {
        while ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z')) {
            token.length++;
            c = *(token.start + token.length);
        }

        if (strncmp(token.start, "let", token.length) == 0) {
            token.type = TOKEN_LET;
        }
        else {
            token.type = TOKEN_IDENTIFIER;
        }

        lexer->current += token.length;
        return token;
    }

    // number lexer
    if (c >= '0' && c <= '9') {
        token.type = TOKEN_NUMBER;
        while (c >= '0' && c <= '9') {
            token.length++;
            c = *(token.start + token.length);
        }
        lexer->current += token.length;
        return token;
    }

    // operator lexer
    switch (c) {
        case '+':
            token.type = TOKEN_PLUS;
            token.length = 1;
            break;
        case '=':
            token.type = TOKEN_EQUAL;
            token.length = 1;
            break;
        case '\0':
            token.type = TOKEN_EOF;
            token.length = 1;
            break;
    }
    lexer->current += token.length;
    
    return token;
}