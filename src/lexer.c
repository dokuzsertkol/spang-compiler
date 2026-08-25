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

    // literal lexer
    if (c >= '0' && c <= '9') {
        token.type = TOKEN_LITERAL;
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

        case '-':
            token.type = TOKEN_MINUS;
            token.length = 1;
            break;

        case '*':
            token.type = TOKEN_MULTIPLY;
            token.length = 1;
            break;

        case '/':
            token.type = TOKEN_DIVIDE;
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

int token_to_int(Token* token) {
    int result = 0;

    for (int i = 0; i < token->length; i++) {
        result = result * 10 + (*(token->start + i) - '0');
    }

    return result;
}

char token_to_char(Token* token) {
    return *token->start;
}

// debug
void lexer_print(const Lexer *lexer) {
    printf("=== LEXER PRINT ===\n");

    Lexer copy = *lexer;
    Token token;

    do {
        token = lexer_next_token(&copy);

        printf("TOKEN: ");

        switch (token.type) {
            case TOKEN_LET: printf("LET"); break;

            case TOKEN_IDENTIFIER: printf("IDENTIFIER"); break;

            case TOKEN_LITERAL: printf("LITERAL"); break;

            case TOKEN_PLUS: printf("PLUS"); break;

            case TOKEN_MINUS: printf("MINUS"); break;

            case TOKEN_MULTIPLY: printf("MULTIPLY"); break;

            case TOKEN_DIVIDE: printf("DIVIDE"); break;

            case TOKEN_EQUAL: printf("EQUAL"); break;

            case TOKEN_ENDLINE: printf("ENDLINE"); break;

            case TOKEN_EOF: printf("EOF"); break;

            default: printf("UNKNOWN"); break;
        }

        if (token.type != TOKEN_EOF) {
            printf(" \"");
            printf("%.*s", token.length, token.start);
            printf("\"");
        }

        putchar('\n');

    } while (token.type != TOKEN_EOF);
    printf("=== END ===\n\n");
}