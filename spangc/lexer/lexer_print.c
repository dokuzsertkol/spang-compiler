#include <stdio.h>
#include "lexer_print.h"

static const char *token_type_to_string(const TokenType tokenType)  {
    switch (tokenType) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";

        case TOKEN_LEFT_BRACKET: return "LEFT_BRACKET";
        case TOKEN_RIGHT_BRACKET: return "RIGHT_BRACKET";
        case TOKEN_LEFT_BRACE: return "LEFT_BRACE";
        case TOKEN_RIGHT_BRACE: return "RIGHT_BRACE";
        case TOKEN_LEFT_PAREN: return "LEFT_PAREN";
        case TOKEN_RIGHT_PAREN: return "RIGHT_PAREN";

        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_DOT: return "DOT";
        case TOKEN_COMMA: return "COMMA";

        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_ASTER: return "ASTERISK";
        case TOKEN_SLASH: return "SLASH";

        case TOKEN_EQUAL: return "EQUAL";
        case TOKEN_EQUAL_EQUAL: return "EQUAL_EQUAL";
        case TOKEN_NOT_EQUAL: return "NOT_EQUAL";
        case TOKEN_LESS: return "LESS";
        case TOKEN_GREATER: return "GREATER";
        case TOKEN_LESS_EQUAL: return "LESS_EQUAL";
        case TOKEN_GREATER_EQUAL: return "GREATER_EQUAL";
        case TOKEN_AMPERS_AMPERS: return "AMPERS_AMPERS";
        case TOKEN_BAR_BAR: return "BAR_BAR";
        case TOKEN_EXCLAM: return "EXCLAM";

        case TOKEN_RETURN: return "RETURN";
        case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_BREAK: return "BREAK";
        case TOKEN_CONTINUE: return "CONTINUE";
        case TOKEN_STRUCT: return "STRUCT";

        case TOKEN_I1: return "I1";
        case TOKEN_I2: return "I2";
        case TOKEN_I4: return "I4";
        case TOKEN_I8: return "I8";
        case TOKEN_U1: return "U1";
        case TOKEN_U2: return "U2";
        case TOKEN_U4: return "U4";
        case TOKEN_U8: return "U8";
        case TOKEN_C1: return "C1";
        case TOKEN_C2: return "C2";
        case TOKEN_C4: return "C4";
        case TOKEN_F4: return "F4";
        case TOKEN_F8: return "F8";
        case TOKEN_B1: return "B1";
        case TOKEN_V0: return "V0";

        case TOKEN_INT_LITERAL: return "INT_LITERAL";
        case TOKEN_FLOAT_LITERAL: return "FLOAT_LITERAL";
        case TOKEN_C1_LITERAL: return "C1_LITERAL";
        case TOKEN_C2_LITERAL: return "C2_LITERAL";
        case TOKEN_C4_LITERAL: return "C4_LITERAL";
        case TOKEN_S1_LITERAL: return "S1_LITERAL";
        case TOKEN_S2_LITERAL: return "S2_LITERAL";
        case TOKEN_S4_LITERAL: return "S4_LITERAL";
        case TOKEN_TRUE: return "TRUE";
        case TOKEN_FALSE: return "FALSE";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_SP: return "SP";
        case TOKEN_FP: return "FP";
        case TOKEN_BP: return "BP";
        case TOKEN_HP: return "HP";
    }
}

void lexer_print(const Lexer *lexer) {
    printf("=== LEXER PRINT ===\n");

    Lexer copy = *lexer;
    Token token;

    do {
        token = lexer_next_token(&copy);

        printf("TOKEN: %s:%zu:%zu \t", lexer->path, token.line, token.column);

        printf("%s", token_type_to_string(token.type));

        if (token.type != TOKEN_EOF) printf(" \"%.*s\"", (int)token.length, token.start);

        putchar('\n');

    } while (token.type != TOKEN_EOF);
    printf("=== END ===\n\n");
}