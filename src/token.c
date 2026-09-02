#include "token.h"

long token_to_int(const Token *token) {
    long value = 0;

    for (size_t i = 0; i < token->length; i++) value = value * 10 + (token->start[i] - '0');

    return value;
}

char token_to_char(const Token *token)
{
    if (token->start[1] != '\\')
        return token->start[1];

    switch (token->start[2]) {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case '\\': return '\\';
        case '\'': return '\'';
        case '"': return '"';
        case '0': return '\0';

        default: return '\0';
    }
}

double token_to_float(const Token *token)
{
    double value = 0.0;
    double divisor = 10.0;
    int decimal = 0;

    for (size_t i = 0; i < token->length; i++) {
        char c = token->start[i];

        if (c == '.') {
            decimal = 1;
            continue;
        }

        if (decimal) {
            value += (c - '0') / divisor;
            divisor *= 10.0;
        } else {
            value = value * 10.0 + (c - '0');
        }
    }

    return value;
}