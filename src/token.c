#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "token.h"

static int token_escape_value(char c, uint32_t *value) {
    switch (c) {
        case 'n':  *value = '\n'; return 1;
        case 't':  *value = '\t'; return 1;
        case 'r':  *value = '\r'; return 1;
        case '\\': *value = '\\'; return 1;
        case '\'': *value = '\''; return 1;
        case '"':  *value = '"';  return 1;
        case '0':  *value = '\0'; return 1;
        default: return 0;
    }
}

static int token_utf8_decode(const Token *token, size_t *index, uint32_t *value) {
    const uint8_t *s = (const uint8_t *)token->start;
    size_t length = token->length;

    uint8_t c = s[*index];

    // ASCII
    if (c < 0x80) {
        *index += 1;
        *value = c;
        return 1;
    }

    // 2 byte UTF-8
    if ((c & 0xE0) == 0xC0) {
        if (*index + 1 >= length) return 0;

        uint8_t c1 = s[*index + 1];

        if ((c1 & 0xC0) != 0x80) return 0;

        uint32_t c4 = ((uint32_t)(c & 0x1F) << 6) | (uint32_t)(c1 & 0x3F);

        if (c4 < 0x80) return 0;

        *index += 2;
        *value = c4;
        return 1;
    }

    // 3 byte UTF-8
    if ((c & 0xF0) == 0xE0) {
        if (*index + 2 >= length) return 0;

        uint8_t c1 = s[*index + 1];
        uint8_t c2 = s[*index + 2];

        if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80) return 0;

        uint32_t c4 = ((uint32_t)(c & 0x0F) << 12) | ((uint32_t)(c1 & 0x3F) << 6) | (uint32_t)(c2 & 0x3F);

        if (c4 < 0x800) return 0;

        if (c4 >= 0xD800 && c4 <= 0xDFFF) return 0;

        *index += 3;
        *value = c4;
        return 1;
    }

    // 4 byte UTF-8
    if ((c & 0xF8) == 0xF0) {
        if (*index + 3 >= length)
            return 0;

        uint8_t c1 = s[*index + 1];
        uint8_t c2 = s[*index + 2];
        uint8_t c3 = s[*index + 3];

        if ((c1 & 0xC0) != 0x80 || (c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) return 0;

        uint32_t c4 = ((uint32_t)(c & 0x07) << 18) | ((uint32_t)(c1 & 0x3F) << 12) | ((uint32_t)(c2 & 0x3F) << 6) | (uint32_t)(c3 & 0x3F);

        if (c4 < 0x10000) return 0;

        if (c4 > 0x10FFFF) return 0;

        *index += 4;
        *value = c4;
        return 1;
    }

    return 0;
}

uint64_t token_to_int(const Token *token) {
    uint64_t value = 0;

    for (size_t i = 0; i < token->length; i++) value = value * 10 + (uint64_t)(token->start[i] - '0');

    return value;
}

double token_to_float(const Token *token) {
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
            value += (double)(c - '0') / divisor;
            divisor *= 10.0;
        } else {
            value = value * 10.0 + (double)(c - '0');
        }
    }

    return value;
}

int token_to_c1(const Token *token, uint8_t *value) {
    if (token->start[1] == '\\') {
        uint32_t escaped;

        if (!token_escape_value(token->start[2], &escaped)) return 0;

        *value = (uint8_t)escaped;
        return 1;
    }

    if (token->start[2] != '\'') return 0;

    *value = (uint8_t)token->start[1];
    return 1;
}

int token_to_c2(const Token *token, uint16_t *value) {
    if (token->start[2] == '\\') {
        uint32_t escaped;

        if (!token_escape_value(token->start[3], &escaped)) return 0;

        *value = (uint16_t)escaped;
        return 1;
    }

    uint32_t c4;
    size_t index = 2;
    if (!token_utf8_decode(token, &index, &c4)) return 0;

    if (token->start[index] != '\'') return 0;

    if (c4 > 0xFFFF) return 0;

    *value = (uint16_t)c4;
    return 1;
}

int token_to_c4(const Token *token, uint32_t *value) {
    if (token->start[2] == '\\') {
        if (!token_escape_value(token->start[3], value)) return 0;
        return 1;
    }

    size_t index = 2;
    if (!token_utf8_decode(token, &index, value)) return 0;

    if (token->start[index] != '\'') return 0;
    return 1;
}

uint8_t *token_to_s1(const Token *token, size_t *strLength) {
    size_t length = token->length - 2; // skip ""

    uint8_t *value = malloc((length > 0 ? length : 1));
    if (!value) return NULL;

    *strLength = 0;

    for (size_t i = 1; i < token->length - 1;) {
        if (token->start[i] == '\\') {
            uint32_t escaped;

            if (!token_escape_value(token->start[i + 1], &escaped)) {
                free(value);
                return NULL;
            }

            value[(*strLength)++] = (uint8_t)escaped;
            i += 2;
            continue;
        }

        value[(*strLength)++] = (uint8_t)token->start[i];
        i++;
    }

    return value;
}

uint16_t *token_to_s2(const Token *token, size_t *strLength) {
    size_t length = token->length - 2; // skip ""

    uint16_t *value = malloc((length > 0 ? length : 1) * sizeof(uint16_t));
    if (!value) return NULL;

    *strLength = 0;

    for (size_t i = 1; i < token->length - 1;) {
        if (token->start[i] == '\\') {
            uint32_t escaped;

            if (!token_escape_value(token->start[i + 1], &escaped)) {
                free(value);
                return NULL;
            }

            value[(*strLength)++] = (uint16_t)escaped;
            i += 2;
            continue;
        }

        uint32_t c4;

        if (!token_utf8_decode(token, &i, &c4)) {
            free(value);
            return NULL;
        }

        if (c4 <= 0xFFFF) {
            value[(*strLength)++] = (uint16_t)c4;
        }
        else {
            c4 -= 0x10000;

            value[(*strLength)++] = (uint16_t)(0xD800 | (c4 >> 10));
            value[(*strLength)++] = (uint16_t)(0xDC00 | (c4 & 0x03FF));
        }
    }

    return value;
}

uint32_t *token_to_s4(const Token *token, size_t *strLength) {
    size_t length = token->length - 2; // skip ""

    uint32_t *value = malloc((length > 0 ? length : 1) * sizeof(uint32_t));
    if (!value) return NULL;

    *strLength = 0;

    for (size_t i = 1; i < token->length - 1;) {
        if (token->start[i] == '\\') {
            uint32_t escaped;

            if (!token_escape_value(token->start[i + 1], &escaped)) {
                free(value);
                return NULL;
            }

            value[(*strLength)++] = escaped;
            i += 2;
            continue;
        }

        uint32_t c4;

        if (!token_utf8_decode(token, &i, &c4)) {
            free(value);
            return NULL;
        }

        value[(*strLength)++] = c4;
    }

    return value;
}