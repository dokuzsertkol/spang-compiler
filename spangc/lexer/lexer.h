#pragma once
#include "../token/token.h"

typedef struct {
    const char *path;
    
    char *source;
    const char *current;

    size_t line;
    size_t column;
} Lexer;

static char *get_source(const char *path);
static int is_identifier_start(const char c);
static int is_digit(const char c);
static int is_identifier_char(const char c);
static void lexer_advance(Lexer *lexer, size_t count);
static int lexer_skip_whitespace_and_comments(Lexer *lexer);
static TokenType lexer_keyword_type(const char *start, const size_t length);
static Token lexer_prefixed_char_and_string(Lexer *lexer);
static Token lexer_char_and_string(Lexer *lexer);
static Token lexer_identifier(Lexer *lexer);
static Token lexer_number(Lexer *lexer);
static Token lexer_punctuation(Lexer *lexer);
int lexer_init(Lexer *lexer, const char *path);
void lexer_free(Lexer *lexer);
Token lexer_next_token(Lexer* lexer);