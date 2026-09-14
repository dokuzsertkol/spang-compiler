#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "token.h"

static const char *get_source(const char *path) {
    FILE *input = fopen(path, "r");

    if (!input) {
        perror("fopen");
        return NULL;
    }

    fseek(input, 0, SEEK_END);
    long size = ftell(input);
    rewind(input);

    char *source = malloc(size + 1);
    if (!source) {
        fclose(input);
        return NULL;
    }

    fread(source, 1, size, input);
    source[size] = '\0';

    fclose(input);

    return source;
}

Lexer lexer_init(const char *path) {
    return (Lexer) {
        .path = path,
        .source = get_source(path),
        .current = get_source(path),
        .line = 1,
        .column = 1,
    };
}

static int is_identifier_start(const char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

static int is_digit(const char c) {
    return (c >= '0' && c <= '9');
}

static int is_identifier_char(const char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

static void lexer_advance(Lexer *lexer, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (*lexer->current == '\0') return;

        if (*lexer->current == '\n') {
            lexer->line++;
            lexer->column = 1;
        }
        else lexer->column++;

        lexer->current++;
    }
}

static int lexer_skip_whitespace_and_comments(Lexer *lexer) {
    while (1) {
        // whitespace
        while (*lexer->current == ' ' || *lexer->current == '\n' || *lexer->current == '\t') lexer_advance(lexer, 1);
        
        // comment
        if (lexer->current[0] == '/' && lexer->current[1] == '/') {
            while (*lexer->current != '\n' && *lexer->current != '\0')lexer_advance(lexer, 1);
            continue;
        }
        if (lexer->current[0] == '/' && lexer->current[1] == '*') {
            lexer_advance(lexer, 2);

            while (*lexer->current != '\0' && !(lexer->current[0] == '*' && lexer->current[1] == '/')) {
                lexer_advance(lexer, 1);
            }

            if (*lexer->current == '\0') return 0;
            else lexer_advance(lexer, 2);
            continue;
        }
        break;
    }
    return 1;
}

static TokenType lexer_keyword_type(const char *start, const size_t length) {
    if (length == 2 && strncmp(start, "if", 2) == 0) {
        return TOKEN_IF;
    }
    else if (length == 2 && strncmp(start, "i1", 2) == 0) {
        return TOKEN_I1;
    }
    else if (length == 2 && strncmp(start, "i2", 2) == 0) {
        return TOKEN_I2;
    }
    else if (length == 2 && strncmp(start, "i4", 2) == 0) {
        return TOKEN_I4;
    }
    else if (length == 2 && strncmp(start, "i8", 2) == 0) {
        return TOKEN_I8;
    }
    else if (length == 2 && strncmp(start, "u1", 2) == 0) {
        return TOKEN_U1;
    }
    else if (length == 2 && strncmp(start, "u2", 2) == 0) {
        return TOKEN_U2;
    }
    else if (length == 2 && strncmp(start, "u4", 2) == 0) {
        return TOKEN_U4;
    }
    else if (length == 2 && strncmp(start, "u8", 2) == 0) {
        return TOKEN_U8;
    }
    else if (length == 2 && strncmp(start, "f4", 2) == 0) {
        return TOKEN_F4;
    }
    else if (length == 2 && strncmp(start, "f8", 2) == 0) {
        return TOKEN_F8;
    }
    else if (length == 2 && strncmp(start, "c1", 2) == 0) {
        return TOKEN_C1;
    }
    else if (length == 2 && strncmp(start, "c2", 2) == 0) {
        return TOKEN_C2;
    }
    else if (length == 2 && strncmp(start, "c4", 2) == 0) {
        return TOKEN_C4;
    }
    else if (length == 2 && strncmp(start, "b1", 2) == 0) {
        return TOKEN_B1;
    }
    else if (length == 2 && strncmp(start, "v0", 2) == 0) {
        return TOKEN_V0;
    }
    else if (length == 2 && strncmp(start, "sp", 2) == 0) {
        return TOKEN_SP;
    }
    else if (length == 2 && strncmp(start, "fp", 2) == 0) {
        return TOKEN_FP;
    }
    else if (length == 2 && strncmp(start, "bp", 2) == 0) {
        return TOKEN_BP;
    }
    else if (length == 2 && strncmp(start, "hp", 2) == 0) {
        return TOKEN_HP;
    }
    else if (length == 4 && strncmp(start, "else", 4) == 0) {
        return TOKEN_ELSE;
    }
    else if (length == 4 && strncmp(start, "true", 4) == 0) {
        return TOKEN_TRUE;
    }
    else if (length == 5 && strncmp(start, "false", 5) == 0) {
        return TOKEN_FALSE;
    }
    else if (length == 5 && strncmp(start, "while", 5) == 0) {
        return TOKEN_WHILE;
    }
    else if (length == 5 && strncmp(start, "break", 5) == 0) {
        return TOKEN_BREAK;
    }
    else if (length == 6 && strncmp(start, "return", 6) == 0) {
        return TOKEN_RETURN;
    }
    else if (length == 6 && strncmp(start, "struct", 6) == 0) {
        return TOKEN_STRUCT;
    }
    else if (length == 8 && strncmp(start, "continue", 8) == 0) {
        return TOKEN_CONTINUE;
    }
    else {
        return TOKEN_IDENTIFIER;
    }
}

static Token lexer_prefixed_char_and_string(Lexer *lexer) {
    Token token = {
        .start = lexer->current,
        .length = 0,
        .line = lexer->line,
        .column = lexer->column,
    };

    char prefix = lexer->current[0];
    char quote = lexer->current[1];

    if (quote == '\'') {
        token.type = prefix == 'u' ? TOKEN_C2_LITERAL : TOKEN_C4_LITERAL;
    } else {
        token.type = prefix == 'u' ? TOKEN_S2_LITERAL : TOKEN_S4_LITERAL;
    }

    lexer_advance(lexer, 2);

    while (*lexer->current != quote && *lexer->current != '\0') {
        if (*lexer->current == '\\' && lexer->current[1] != '\0') lexer_advance(lexer, 2);
        else lexer_advance(lexer, 1);
    }

    if (*lexer->current == '\0') {
        token.type = TOKEN_ERROR;
        return token;
    }

    lexer_advance(lexer, 1);

    token.length = lexer->current - token.start;
    return token;
}

static Token lexer_char_and_string(Lexer *lexer) {
    Token token = {
        .start = lexer->current,
        .length = 0,
        .line = lexer->line,
        .column = lexer->column,
    };

    char quote = *lexer->current;

    token.type = quote == '\'' ? TOKEN_C1_LITERAL : TOKEN_S1_LITERAL;

    lexer_advance(lexer, 1);

    while (*lexer->current != quote && *lexer->current != '\0') {
        if (*lexer->current == '\\' && lexer->current[1] != '\0') lexer_advance(lexer, 2);
        else lexer_advance(lexer, 1);
    }

    if (*lexer->current == '\0') {
        token.type = TOKEN_ERROR;
        return token;
    }

    lexer_advance(lexer, 1);

    token.length = lexer->current - token.start;
    return token;
}

static Token lexer_identifier(Lexer *lexer) {
    Token token = {
        .start = lexer->current,
        .length = 0,
        .line = lexer->line,
        .column = lexer->column,
    };

    while (is_identifier_char(lexer->current[token.length])) token.length++;

    token.type = lexer_keyword_type(token.start, token.length);

    lexer_advance(lexer, token.length);

    return token;
}

static Token lexer_number(Lexer *lexer) {
    Token token = {
        .start = lexer->current,
        .length = 0,
        .type = TOKEN_INT_LITERAL,
        .line = lexer->line,
        .column = lexer->column,
    };

    int dot_count = 0;

    while (1) {
        char c = lexer->current[token.length];

        if (c >= '0' && c <= '9') {
            token.length++;
            continue;
        }

        if (c == '.') {
            if (dot_count++) {
                token.type = TOKEN_ERROR;
                token.length++;
                break;
            }

            token.type = TOKEN_FLOAT_LITERAL;
            token.length++;
            continue;
        }

        break;
    }

    lexer_advance(lexer, token.length);

    return token;
}

static Token lexer_punctuation(Lexer *lexer) {
    Token token = {
        .start = lexer->current,
        .length = 1,
        .line = lexer->line,
        .column = lexer->column,
    };

    switch (*lexer->current) {
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
        case '/': token.type = TOKEN_SLASH; break;

        case '=':
            if (lexer->current[1] == '=') {
                token.type = TOKEN_EQUAL_EQUAL;
                token.length = 2;
            } else {
                token.type = TOKEN_EQUAL;
            }
            break;

        case '>':
            if (lexer->current[1] == '=') {
                token.type = TOKEN_GREATER_EQUAL;
                token.length = 2;
            } else {
                token.type = TOKEN_GREATER;
            }
            break;

        case '<':
            if (lexer->current[1] == '=') {
                token.type = TOKEN_LESS_EQUAL;
                token.length = 2;
            } else {
                token.type = TOKEN_LESS;
            }
            break;

        case '!':
            if (lexer->current[1] == '=') {
                token.type = TOKEN_NOT_EQUAL;
                token.length = 2;
            } else {
                token.type = TOKEN_EXCLAM;
            }
            break;

        case '&':
            if (lexer->current[1] == '&') {
                token.type = TOKEN_AMPERS_AMPERS;
                token.length = 2;
            } else {
                token.type = TOKEN_ERROR;
            }
            break;

        case '|':
            if (lexer->current[1] == '|') {
                token.type = TOKEN_BAR_BAR;
                token.length = 2;
            } else {
                token.type = TOKEN_ERROR;
            }
            break;

        default: token.type = TOKEN_ERROR; break;
    }

    lexer_advance(lexer, token.length);

    return token;
}

Token lexer_next_token(Lexer *lexer) {

    if (!lexer_skip_whitespace_and_comments(lexer)) {
        return (Token) {
            .type = TOKEN_ERROR,
            .start = lexer->current,
            .length = 0,
            .line = lexer->line,
            .column = lexer->column,
        };
    }

    char c = *lexer->current;
    if (c == '\0') {
        return (Token) {
            .type = TOKEN_EOF,
            .start = lexer->current,
            .length = 0,
            .line = lexer->line,
            .column = lexer->column,
        };
    }

    if (c == '\'' || c == '"') return lexer_char_and_string(lexer);

    if ((c == 'u' || c == 'U') && (lexer->current[1] == '\'' || lexer->current[1] == '"')) return lexer_prefixed_char_and_string(lexer);

    if (is_identifier_start(c)) return lexer_identifier(lexer);

    if (is_digit(c)) return lexer_number(lexer);

    return lexer_punctuation(lexer);
}

// debug
void lexer_print(const Lexer *lexer) {
    printf("=== LEXER PRINT ===\n");

    Lexer copy = *lexer;
    Token token;

    do {
        token = lexer_next_token(&copy);

        printf("TOKEN: %zu:%zu ", token.line, token.column);

        printf("%s", token_type_to_string(token.type));

        if (token.type != TOKEN_EOF) {
            printf(" \"");
            printf("%.*s", (int)token.length, token.start);
            printf("\"");
        }

        putchar('\n');

    } while (token.type != TOKEN_EOF);
    printf("=== END ===\n\n");
}