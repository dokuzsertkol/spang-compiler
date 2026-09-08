#include <string.h>
#include "lexer.h"
#include "token.h"

Lexer lexer_init(const char *source) {
    Lexer lexer;

    lexer.source = source;
    lexer.current = source;

    return lexer;
}

Token lexer_next_token(Lexer *lexer) {
    Token token;

    while (1) {
        // whitespace
        while (*lexer->current == ' ' || *lexer->current == '\n' || *lexer->current == '\t') lexer->current++;
        
        // comment
        if (lexer->current[0] == '/' && lexer->current[1] == '/') {
            while (*lexer->current != '\n' && *lexer->current != '\0') lexer->current++;
            continue;
        }
        if (lexer->current[0] == '/' && lexer->current[1] == '*') {
            lexer->current += 2;

            while (*lexer->current != '\0' &&
                !(lexer->current[0] == '*' &&
                    lexer->current[1] == '/')) {
                lexer->current++;
            }

            if (*lexer->current == '\0') {
                token.type = TOKEN_ERROR;
                return token;
            } else lexer->current += 2;
            continue;
        }
        break;
    }

    token.start = lexer->current;
    token.length = 0;

    char c = *token.start;

    // utf-16 or utf-32 char and string lexer
    if (c == 'u' || c == 'U') {
        if (lexer->current[1] == '\'' || lexer->current[1] == '"') {
            char quote = lexer->current[1];

            token.length = 2;

            if (quote == '\'') {
                token.type = (c == 'u') ? TOKEN_C2_LITERAL : TOKEN_C4_LITERAL;
            } else {
                token.type = (c == 'u') ? TOKEN_S2_LITERAL : TOKEN_S4_LITERAL;
            }

            lexer->current += 2;

            while (*lexer->current != quote &&
                *lexer->current != '\0') {

                if (*lexer->current == '\\' &&
                    lexer->current[1] != '\0') {
                    lexer->current += 2;
                } else {
                    lexer->current++;
                }
            }

            if (*lexer->current == '\0') {
                token.type = TOKEN_ERROR;
                return token;
            }

            lexer->current++;

            token.length = lexer->current - token.start;

            return token;
        }
    }

    // UTF-8 char and string lexer
    if (c == '"' || c == '\'') {
        char quote = c;

        token.type = (quote == '"') ? TOKEN_S1_LITERAL : TOKEN_C1_LITERAL;

        lexer->current++;

        while (*lexer->current != quote &&
            *lexer->current != '\0') {

            if (*lexer->current == '\\' && lexer->current[1] != '\0') {
                lexer->current += 2;
            } else {
                lexer->current++;
            }
        }

        if (*lexer->current == '\0') {
            token.type = TOKEN_ERROR;
            return token;
        }

        lexer->current++;

        token.length = lexer->current - token.start;

        return token;
    }
    // identifier and keyword lexer
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
        while ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_')) {
            c = *(token.start + ++token.length);
        }

        if (token.length == 2 && strncmp(token.start, "if", 2) == 0) {
            token.type = TOKEN_IF;
        }
        else if (token.length == 2 && strncmp(token.start, "i1", 2) == 0) {
            token.type = TOKEN_I1;
        }
        else if (token.length == 2 && strncmp(token.start, "i2", 2) == 0) {
            token.type = TOKEN_I2;
        }
        else if (token.length == 2 && strncmp(token.start, "i4", 2) == 0) {
            token.type = TOKEN_I4;
        }
        else if (token.length == 2 && strncmp(token.start, "i8", 2) == 0) {
            token.type = TOKEN_I8;
        }
        else if (token.length == 2 && strncmp(token.start, "u1", 2) == 0) {
            token.type = TOKEN_U1;
        }
        else if (token.length == 2 && strncmp(token.start, "u2", 2) == 0) {
            token.type = TOKEN_U2;
        }
        else if (token.length == 2 && strncmp(token.start, "u4", 2) == 0) {
            token.type = TOKEN_U4;
        }
        else if (token.length == 2 && strncmp(token.start, "u8", 2) == 0) {
            token.type = TOKEN_U8;
        }
        else if (token.length == 2 && strncmp(token.start, "f4", 2) == 0) {
            token.type = TOKEN_F4;
        }
        else if (token.length == 2 && strncmp(token.start, "f8", 2) == 0) {
            token.type = TOKEN_F8;
        }
        else if (token.length == 2 && strncmp(token.start, "c1", 2) == 0) {
            token.type = TOKEN_C1;
        }
        else if (token.length == 2 && strncmp(token.start, "c2", 2) == 0) {
            token.type = TOKEN_C2;
        }
        else if (token.length == 2 && strncmp(token.start, "c4", 2) == 0) {
            token.type = TOKEN_C4;
        }
        else if (token.length == 2 && strncmp(token.start, "b1", 2) == 0) {
            token.type = TOKEN_B1;
        }
        else if (token.length == 2 && strncmp(token.start, "v0", 2) == 0) {
            token.type = TOKEN_V0;
        }
        else if (token.length == 2 && strncmp(token.start, "sp", 2) == 0) {
            token.type = TOKEN_SP;
        }
        else if (token.length == 2 && strncmp(token.start, "fp", 2) == 0) {
            token.type = TOKEN_FP;
        }
        else if (token.length == 2 && strncmp(token.start, "bp", 2) == 0) {
            token.type = TOKEN_BP;
        }
        else if (token.length == 2 && strncmp(token.start, "hp", 2) == 0) {
            token.type = TOKEN_HP;
        }
        else if (token.length == 4 && strncmp(token.start, "else", 4) == 0) {
            token.type = TOKEN_ELSE;
        }
        else if (token.length == 4 && strncmp(token.start, "true", 4) == 0) {
            token.type = TOKEN_TRUE;
        }
        else if (token.length == 5 && strncmp(token.start, "false", 5) == 0) {
            token.type = TOKEN_FALSE;
        }
        else if (token.length == 5 && strncmp(token.start, "while", 5) == 0) {
            token.type = TOKEN_WHILE;
        }
        else if (token.length == 5 && strncmp(token.start, "break", 5) == 0) {
            token.type = TOKEN_BREAK;
        }
        else if (token.length == 6 && strncmp(token.start, "return", 6) == 0) {
            token.type = TOKEN_RETURN;
        }
        else if (token.length == 6 && strncmp(token.start, "struct", 6) == 0) {
            token.type = TOKEN_STRUCT;
        }
        else if (token.length == 8 && strncmp(token.start, "continue", 8) == 0) {
            token.type = TOKEN_CONTINUE;
        }
        else {
            token.type = TOKEN_IDENTIFIER;
        }

        lexer->current += token.length;
        return token;
    }

    // number lexer
    if (c >= '0' && c <= '9') {
        token.type = TOKEN_INT_LITERAL;
        int dotCount = 0;
        while ((c >= '0' && c <= '9') || (c == '.')) {
            if (c == '.') {
                if (!dotCount++) { token.type = TOKEN_ERROR; break; }
                token.type = TOKEN_FLOAT_LITERAL;
            }
            token.length++;
            c = *(token.start + token.length);
        }
        lexer->current += token.length;
        return token;
    }

    // punctuation lexer
    token.length = 1;
    switch (c) {
        case ';': token.type = TOKEN_SEMICOLON; break;

        case '[': token.type = TOKEN_LEFT_BRACKET; break;

        case ']': token.type = TOKEN_RIGHT_BRACKET; break;

        case '{': token.type = TOKEN_LEFT_BRACE; break;

        case '}': token.type = TOKEN_RIGHT_BRACE; break;

        case '(': token.type = TOKEN_LEFT_PAREN; break;

        case ')': token.type = TOKEN_RIGHT_PAREN; break;

        case '.': token.type = TOKEN_DOT; break;

        case ',': token.type = TOKEN_COMMA; break;

        case '+': token.type = TOKEN_PLUS; break;

        case '-': token.type = TOKEN_MINUS; break;

        case '*': token.type = TOKEN_ASTER; break;

        case '/': {
            token.type = TOKEN_SLASH; break;
        }
        case '=': {
            if (lexer->current[1] == '=') {
                token.type = TOKEN_EQUAL_EQUAL;
                token.length = 2;
            } else token.type = TOKEN_EQUAL;
            break;
        }
        case '>': {
            if (lexer->current[1] == '=') {
                token.type = TOKEN_GREATER_EQUAL;
                token.length = 2;
            } else token.type = TOKEN_GREATER;
            break;
        }
        case '<': {
            if (lexer->current[1] == '=') {
                token.type = TOKEN_LESS_EQUAL;
                token.length = 2;
            } else token.type = TOKEN_LESS;
            break;
        }
        case '!': {
            if (lexer->current[1] == '=') {
                token.type = TOKEN_NOT_EQUAL;
                token.length = 2;
            } else token.type = TOKEN_EXCLAM;
            break;
        }
        case '&': {
            if (lexer->current[1] == '&') {
                token.type = TOKEN_AMPERS_AMPERS;
                token.length = 2;
            } else token.type = TOKEN_ERROR;
            break;
        }
        case '|': {
            if (lexer->current[1] == '|') {
                token.type = TOKEN_BAR_BAR;
                token.length = 2;
            } else token.type = TOKEN_ERROR;
            break;
        }
        case '\0': token.type = TOKEN_EOF; break;
        
        default: token.type = TOKEN_ERROR; break;
    }
    lexer->current += token.length;
    return token;
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
            case TOKEN_EOF: printf("EOF"); break;
            case TOKEN_ERROR: printf("ERROR"); break;

            case TOKEN_LEFT_BRACKET: printf("LEFT_BRACKET"); break;
            case TOKEN_RIGHT_BRACKET: printf("RIGHT_BRACKET"); break;
            case TOKEN_LEFT_BRACE: printf("LEFT_BRACE"); break;
            case TOKEN_RIGHT_BRACE: printf("RIGHT_BRACE"); break;
            case TOKEN_LEFT_PAREN: printf("LEFT_PAREN"); break;
            case TOKEN_RIGHT_PAREN: printf("RIGHT_PAREN"); break;

            case TOKEN_SEMICOLON: printf("SEMICOLON"); break;
            case TOKEN_DOT: printf("DOT"); break;
            case TOKEN_COMMA: printf("COMMA"); break;

            case TOKEN_PLUS: printf("PLUS"); break;
            case TOKEN_MINUS: printf("MINUS"); break;
            case TOKEN_ASTER: printf("ASTERIX"); break;
            case TOKEN_SLASH: printf("SLASH"); break;

            case TOKEN_EQUAL: printf("EQUAL"); break;
            case TOKEN_EQUAL_EQUAL: printf("EQUAL_EQUAL"); break;
            case TOKEN_NOT_EQUAL: printf("NOT_EQUAL"); break;
            case TOKEN_LESS: printf("LESS"); break;
            case TOKEN_GREATER: printf("GREATER"); break;
            case TOKEN_LESS_EQUAL: printf("LESS_EQUAL"); break;
            case TOKEN_GREATER_EQUAL: printf("GREATER_EQUAL"); break;
            case TOKEN_AMPERS_AMPERS: printf("AMPERS_AMPERS"); break;
            case TOKEN_BAR_BAR: printf("BAR_BAR"); break;
            case TOKEN_EXCLAM: printf("EXCLAM"); break;

            case TOKEN_RETURN: printf("RETURN"); break;
            case TOKEN_IF: printf("IF"); break;
            case TOKEN_ELSE: printf("ELSE"); break;
            case TOKEN_WHILE: printf("WHILE"); break;
            case TOKEN_BREAK: printf("BREAK"); break;
            case TOKEN_CONTINUE: printf("CONTINUE"); break;
            case TOKEN_STRUCT: printf("STRUCT"); break;


            case TOKEN_I1: printf("I1"); break;
            case TOKEN_I2: printf("I2"); break;
            case TOKEN_I4: printf("I4"); break;
            case TOKEN_I8: printf("I8"); break;
            case TOKEN_U1: printf("U1"); break;
            case TOKEN_U2: printf("U2"); break;
            case TOKEN_U4: printf("U4"); break;
            case TOKEN_U8: printf("U8"); break;
            case TOKEN_C1: printf("C1"); break;
            case TOKEN_C2: printf("C2"); break;
            case TOKEN_C4: printf("C4"); break;
            case TOKEN_F4: printf("F4"); break;
            case TOKEN_F8: printf("F8"); break;
            case TOKEN_B1: printf("B1"); break;
            case TOKEN_V0: printf("V0"); break;

            case TOKEN_INT_LITERAL: printf("INT_LITERAL"); break;
            case TOKEN_FLOAT_LITERAL: printf("FLOAT_LITERAL"); break;
            case TOKEN_C1_LITERAL: printf("C1_LITERAL"); break;
            case TOKEN_C2_LITERAL: printf("C2_LITERAL"); break;
            case TOKEN_C4_LITERAL: printf("C4_LITERAL"); break;
            case TOKEN_S1_LITERAL: printf("S1_LITERAL"); break;
            case TOKEN_S2_LITERAL: printf("S2_LITERAL"); break;
            case TOKEN_S4_LITERAL: printf("S4_LITERAL"); break;
            case TOKEN_TRUE: printf("TRUE"); break;
            case TOKEN_FALSE: printf("FALSE"); break;
            case TOKEN_IDENTIFIER: printf("IDENTIFIER"); break;
            case TOKEN_SP: printf("SP"); break;
            case TOKEN_FP: printf("FP"); break;
            case TOKEN_BP: printf("BP"); break;
            case TOKEN_HP: printf("HP"); break;
        }

        if (token.type != TOKEN_EOF) {
            printf(" \"");
            printf("%.*s", (int)token.length, token.start);
            printf("\"");
        }

        putchar('\n');

    } while (token.type != TOKEN_EOF);
    printf("=== END ===\n\n");
}