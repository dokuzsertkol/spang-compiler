#pragma once
#include <stdio.h>

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
    TOKEN_APOST,

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

    TOKEN_INT,
    TOKEN_CHAR,
    TOKEN_FLOAT,
    TOKEN_BOOL,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_BREAK,
    
    TOKEN_LITERAL,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_IDENTIFIER,
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    int length;
} Token;

typedef struct {
    const char* source;
    const char *current;
} Lexer;

Lexer lexer_init(const char* source);
Token lexer_next_token(Lexer* lexer);
int token_to_int(Token* token);
char token_to_char(Token* token);

// debug
void lexer_print(const Lexer *lexer);
