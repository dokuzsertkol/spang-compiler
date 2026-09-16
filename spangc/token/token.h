#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    TOKEN_EOF,
    TOKEN_ERROR,

    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,

    TOKEN_SEMICOLON,
    TOKEN_DOT,
    TOKEN_COMMA,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_ASTER,
    TOKEN_SLASH,

    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS,
    TOKEN_GREATER,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER_EQUAL,
    TOKEN_AMPERS_AMPERS,
    TOKEN_BAR_BAR,
    TOKEN_EXCLAM,

    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_STRUCT,
    TOKEN_INCLUDE,

    TOKEN_I1,
    TOKEN_I2,
    TOKEN_I4,
    TOKEN_I8,
    TOKEN_U1,
    TOKEN_U2,
    TOKEN_U4,
    TOKEN_U8,
    TOKEN_C1,
    TOKEN_C2,
    TOKEN_C4,
    TOKEN_F4,
    TOKEN_F8,
    TOKEN_B1,
    TOKEN_V0,
    
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_C1_LITERAL,
    TOKEN_C2_LITERAL,
    TOKEN_C4_LITERAL,
    TOKEN_S1_LITERAL,
    TOKEN_S2_LITERAL,
    TOKEN_S4_LITERAL,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_IDENTIFIER,
    TOKEN_SP,
    TOKEN_FP,
    TOKEN_BP,
    TOKEN_HP,
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    size_t length;
    
    size_t line;
    size_t column;
} Token;

static int token_escape_value(char c, uint32_t *value);
static int token_utf8_decode(const Token *token, size_t *index, uint32_t *value);

uint64_t token_to_int(const Token *token);
double token_to_float(const Token *token);
bool token_to_bool(const Token *token);

int token_to_c1(const Token *token, uint8_t *value);
int token_to_c2(const Token *token, uint16_t *value);
int token_to_c4(const Token *token, uint32_t *value);

uint8_t *token_to_s1(const Token *token, size_t *length);
uint16_t *token_to_s2(const Token *token, size_t *length);
uint32_t *token_to_s4(const Token *token, size_t *length);