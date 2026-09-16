#include "parser.h"
#include <stdlib.h>

static AST_Expression *parse_expression(Parser *parser);
static AST_Node *parse_statement(Parser *parser);

static int parser_next(Parser *parser) {
    parser->current = lexer_next_token(parser->lexer);
    if (parser->current.type == TOKEN_ERROR) {
        parser_error(parser, PARSER_ERROR_INVALID_TOKEN);
        return 0;
    }
    return 1;
}

Parser parser_init(Lexer *lexer) {
    Parser parser = {
        .lexer = lexer,
        .current = {0},
        .hasError = 0,
    };
    parser_next(&parser);
    return parser;
}

static int parser_match(Parser *parser, TokenType type) {
    if (parser->current.type != type) return 0;
    return parser_next(parser);
}

static int parse_location(Parser *parser, AST_Location *loc) {
    *loc = (AST_Location){0};

    if (!parser_match(parser, TOKEN_LEFT_BRACKET)) {
        parser_error(parser, PARSER_ERROR_EXPECTED_LEFT_BRACKET);
        return 0;
    }

    switch (parser->current.type) {
        case TOKEN_SP: loc->base = AST_LOCATION_SP; break;
        case TOKEN_FP: loc->base = AST_LOCATION_FP; break;
        case TOKEN_BP: loc->base = AST_LOCATION_BP; break;
        case TOKEN_HP: loc->base = AST_LOCATION_HP; break;
        default: parser_error(parser, PARSER_ERROR_EXPECTED_BASE); return 0;
    }

    if (!parser_next(parser)) return 0;

    if (parser->current.type != TOKEN_COMMA) {
        loc->offset = parse_expression(parser);
        if (!loc->offset) return 0;
    }

    if (!parser_match(parser, TOKEN_COMMA)) {
        ast_expression_free(loc->offset);
        parser_error(parser, PARSER_ERROR_EXPECTED_COMMA);
        return 0;
    }

    loc->size = parse_expression(parser);
    if (!loc->size) {
        ast_expression_free(loc->offset);
        return 0;
    }

    if (loc->size->type == AST_EX_DATA_TYPE) loc->readAs = loc->size;

    if (!parser_match(parser, TOKEN_RIGHT_BRACKET)) {
        ast_expression_free(loc->offset);
        ast_expression_free(loc->size);
        parser_error(parser, PARSER_ERROR_EXPECTED_RIGHT_BRACKET);
        return 0;
    }

    return 1;
}

static int parse_field(Parser *parser, AST_Field *field) {
    *field = (AST_Field){0};
    
    Token identifier = parser->current;
    if (!parser_match(parser, TOKEN_IDENTIFIER)) {
        parser_error(parser, PARSER_ERROR_EXPECTED_IDENTIFIER);
        return 0;
    }

    if (!parser_match(parser, TOKEN_LEFT_BRACKET)) {
        parser_error(parser, PARSER_ERROR_EXPECTED_LEFT_BRACKET);
        return 0;
    }

    field->offset = parse_expression(parser);
    if (!field->offset) return 0;

    if (!parser_match(parser, TOKEN_COMMA)) {
        ast_expression_free(field->offset);
        parser_error(parser, PARSER_ERROR_EXPECTED_COMMA);
        return 0;
    }

    field->size = parse_expression(parser);
    if (!field->size) {
        ast_expression_free(field->offset);
        return 0;
    }

    if (field->size->type == AST_EX_DATA_TYPE) field->readAs = field->size;

    if (!parser_match(parser, TOKEN_RIGHT_BRACKET)) {
        ast_expression_free(field->offset);
        ast_expression_free(field->size);
        parser_error(parser, PARSER_ERROR_EXPECTED_RIGHT_BRACKET);
        return 0;
    }

    field->name = identifier.start;
    field->length = identifier.length;
    return 1;
}

static AST_Expression *parse_literal(Parser *parser) {
    AST_Expression *exp = malloc(sizeof(*exp));
    if (!exp) return NULL;

    exp->type = AST_EX_LITERAL;
    exp->literal.length = 0;

    switch (parser->current.type) {
        case TOKEN_INT_LITERAL:
            exp->literal.type = AST_LITERAL_INT;
            exp->literal.intValue = token_to_int(&parser->current);
            break;

        case TOKEN_FLOAT_LITERAL:
            exp->literal.type = AST_LITERAL_FLOAT;
            exp->literal.floatValue = token_to_float(&parser->current);
            break;

        case TOKEN_TRUE: case TOKEN_FALSE:
            exp->literal.type = AST_LITERAL_BOOL;
            exp->literal.boolValue = parser->current.type == TOKEN_TRUE;
            break;

        case TOKEN_C1_LITERAL:
            exp->literal.type = AST_LITERAL_C1;
            if(!token_to_c1(&parser->current, &exp->literal.c1Value)) {
                free(exp);
                return NULL;
            }
            break;
        case TOKEN_C2_LITERAL:
            exp->literal.type = AST_LITERAL_C2;
            if(!token_to_c2(&parser->current, &exp->literal.c2Value)) {
                free(exp);
                return NULL;
            }
            break;
        case TOKEN_C4_LITERAL:
            exp->literal.type = AST_LITERAL_C4;
            if(!token_to_c4(&parser->current, &exp->literal.c4Value)) {
                free(exp);
                return NULL;
            }
            break;

        case TOKEN_S1_LITERAL:
            exp->literal.type = AST_LITERAL_S1;
            exp->literal.s1Value = token_to_s1(&parser->current, &exp->literal.length);
            if (!exp->literal.s1Value) {
                free(exp);
                return NULL;
            }
            break;
        case TOKEN_S2_LITERAL:
            exp->literal.type = AST_LITERAL_S2;
            exp->literal.s2Value = token_to_s2(&parser->current, &exp->literal.length);
            if (!exp->literal.s2Value) {
                free(exp);
                return NULL;
            }
            break;
        case TOKEN_S4_LITERAL:
            exp->literal.type = AST_LITERAL_S4;
            exp->literal.s4Value = token_to_s4(&parser->current, &exp->literal.length);
            if (!exp->literal.s4Value) {
                free(exp);
                return NULL;
            }
            break;

        default:
            parser_error(parser, PARSER_ERROR_EXPECTED_EXPRESSION);
            free(exp);
            return NULL;
    }
    if (!parser_next(parser)) {
        ast_expression_free(exp);
        return NULL;
    }

    return exp;
}

static AST_Expression *parse_identifier(Parser *parser) {
    AST_Expression *exp = malloc(sizeof(*exp));
    if (!exp) return NULL;

    *exp = (AST_Expression){
        .type = AST_EX_VARIABLE,
        .variable = (AST_Variable) {
            .name = parser->current.start,
            .length = parser->current.length,
        },
    };

    if (!parser_next(parser)) {
        free(exp);
        return NULL;
    }
    return exp;
}

static AST_Expression *parse_parenthesized(Parser *parser) {
    if (!parser_match(parser, TOKEN_LEFT_PAREN)) {
        parser_error(parser, PARSER_ERROR_EXPECTED_LEFT_PAREN);
        return NULL;
    }

    AST_Expression *exp = parse_expression(parser);
    if (!exp) return NULL;

    if (!parser_match(parser, TOKEN_RIGHT_PAREN)) {
        ast_expression_free(exp);
        parser_error(parser, PARSER_ERROR_EXPECTED_RIGHT_PAREN);
        return NULL;
    }

    return exp;
}

static AST_Expression *parse_data_type(Parser *parser) {
    AST_Expression *exp = malloc(sizeof(*exp));
    if (!exp) return NULL;

    exp->type = AST_EX_DATA_TYPE;

    switch (parser->current.type) {
        case TOKEN_I1: exp->dataType = AST_DATA_I1; break;
        case TOKEN_I2: exp->dataType = AST_DATA_I2; break;
        case TOKEN_I4: exp->dataType = AST_DATA_I4; break;
        case TOKEN_I8: exp->dataType = AST_DATA_I8; break;
        case TOKEN_U1: exp->dataType = AST_DATA_U1; break;
        case TOKEN_U2: exp->dataType = AST_DATA_U2; break;
        case TOKEN_U4: exp->dataType = AST_DATA_U4; break;
        case TOKEN_U8: exp->dataType = AST_DATA_U8; break;
        case TOKEN_C1: exp->dataType = AST_DATA_C1; break;
        case TOKEN_C2: exp->dataType = AST_DATA_C2; break;
        case TOKEN_C4: exp->dataType = AST_DATA_C4; break;
        case TOKEN_F4: exp->dataType = AST_DATA_F4; break;
        case TOKEN_F8: exp->dataType = AST_DATA_F8; break;
        case TOKEN_B1: exp->dataType = AST_DATA_B1; break;
        case TOKEN_V0: exp->dataType = AST_DATA_V0; break;
        default: parser_error(parser, PARSER_ERROR_EXPECTED_EXPRESSION); ast_expression_free(exp); return NULL;
    }
    if (!parser_next(parser)) {
        ast_expression_free(exp);
        return NULL;
    }

    return exp;
}

static AST_Expression *parse_sp(Parser *parser) {
    if(!parser_match(parser, TOKEN_SP)) {
        parser_error(parser, PARSER_ERROR_UNEXPECTED_TOKEN);
        return NULL;
    }

    AST_Expression *exp = malloc(sizeof(*exp));
    if (!exp) return NULL;

    *exp = (AST_Expression) {
        .type = AST_EX_SP,
    };
    return exp;
} 

static AST_Expression *parse_primary(Parser *parser) {
    switch (parser->current.type) {
        case TOKEN_INT_LITERAL: case TOKEN_FLOAT_LITERAL: case TOKEN_TRUE: case TOKEN_FALSE:
        case TOKEN_C1_LITERAL: case TOKEN_C2_LITERAL: case TOKEN_C4_LITERAL: 
        case TOKEN_S1_LITERAL: case TOKEN_S2_LITERAL: case TOKEN_S4_LITERAL:
            return parse_literal(parser);

        case TOKEN_IDENTIFIER: return parse_identifier(parser);

        case TOKEN_LEFT_PAREN: return parse_parenthesized(parser);

        case TOKEN_I1: case TOKEN_I2: case TOKEN_I4: case TOKEN_I8: case TOKEN_U1: case TOKEN_U2: case TOKEN_U4: case TOKEN_U8:
        case TOKEN_C1: case TOKEN_C2: case TOKEN_C4: case TOKEN_F4: case TOKEN_F8: case TOKEN_B1: case TOKEN_V0:
            return parse_data_type(parser);
    
        case TOKEN_SP: return parse_sp(parser);

        default: 
        parser_error(parser, PARSER_ERROR_EXPECTED_EXPRESSION); return NULL;
    }
}

static AST_Expression *parse_location_access(Parser *parser, AST_Expression *exp) {
    if (!parser_match(parser, TOKEN_LEFT_BRACKET)) {
        ast_expression_free(exp);
        parser_error(parser, PARSER_ERROR_EXPECTED_LEFT_BRACKET);
        return NULL;
    }

    AST_Expression *offset = parse_expression(parser);
    if (!offset) {
        ast_expression_free(exp);
        return NULL;
    }

    if (!parser_match(parser, TOKEN_COMMA)) {
        ast_expression_free(exp);
        ast_expression_free(offset);
        parser_error(parser, PARSER_ERROR_EXPECTED_COMMA);
        return NULL;
    }

    AST_Expression *size = parse_expression(parser);
    if (!size) {
        ast_expression_free(exp);
        ast_expression_free(offset);
        return NULL;
    }

    AST_Expression *readAs = (size->type == AST_EX_DATA_TYPE) ? size : NULL;

    if (!parser_match(parser, TOKEN_RIGHT_BRACKET)) {
        ast_expression_free(exp);
        ast_expression_free(offset);
        ast_expression_free(size);
        parser_error(parser, PARSER_ERROR_EXPECTED_RIGHT_BRACKET);
        return NULL;
    }

    AST_Expression *access = malloc(sizeof(*access));
    if (!access) {
        ast_expression_free(exp);
        ast_expression_free(offset);
        ast_expression_free(size);
        return NULL;
    }

    *access = (AST_Expression){
        .type = AST_EX_LOCATION_ACCESS,
        .locationAccess = {
            .parent = exp,
            .offset = offset,
            .size = size,
            .readAs = readAs,
        },
    };

    return access;
}

static AST_Expression *parse_member_access(Parser *parser, AST_Expression *exp) {
    Token member = parser->current;

    if (!parser_match(parser, TOKEN_IDENTIFIER)) {
        ast_expression_free(exp);
        parser_error(parser, PARSER_ERROR_EXPECTED_IDENTIFIER);
        return NULL;
    }

    AST_Expression *access = malloc(sizeof(*access));
    if (!access) {
        ast_expression_free(exp);
        return NULL;
    }

    *access = (AST_Expression){
        .type = AST_EX_MEMBER_ACCESS,
        .memberAccess = {
            .parent = exp,
            .name = member.start,
            .length = member.length,
        },
    };

    return access;
}

static AST_Expression *parse_call(Parser *parser, AST_Expression *exp) {
    if (!parser_match(parser, TOKEN_LEFT_PAREN)) {
        ast_expression_free(exp);
        parser_error(parser, PARSER_ERROR_EXPECTED_LEFT_PAREN);
        return NULL;
    }

    AST_Expression *func = malloc(sizeof(*func));
    if (!func) {
        ast_expression_free(exp);
        return NULL;
    }

    *func = (AST_Expression){
        .type = AST_EX_CALL,
        .call = {
            .function = exp,
            .arguments = NULL,
            .argumentCount = 0,
        },
    };

    size_t capacity = 8;

    func->call.arguments = malloc(sizeof(*func->call.arguments) * capacity);
    if (!func->call.arguments) {
        ast_expression_free(func);
        return NULL;
    }

    while (parser->current.type != TOKEN_RIGHT_PAREN && parser->current.type != TOKEN_EOF) {
        AST_Expression *arg = parse_expression(parser);
        if (!arg) {
            ast_expression_free(func);
            return NULL;
        }

        if (func->call.argumentCount >= capacity) {
            capacity *= 2;

            AST_Expression **args = realloc(func->call.arguments, sizeof(*args) * capacity);

            if (!args) {
                ast_expression_free(arg);
                ast_expression_free(func);
                return NULL;
            }

            func->call.arguments = args;
        }

        func->call.arguments[func->call.argumentCount++] = arg;

        if (parser->current.type == TOKEN_COMMA) {
            if (!parser_next(parser)) {
                ast_expression_free(func);
                return NULL;
            }
            if (parser->current.type == TOKEN_RIGHT_PAREN) {
                ast_expression_free(func);
                parser_error(parser, PARSER_ERROR_UNEXPECTED_TOKEN);
                return NULL;
            }
        }
        else break;
    }

    if (!parser_match(parser, TOKEN_RIGHT_PAREN)) {
        ast_expression_free(func);
        parser_error(parser, PARSER_ERROR_EXPECTED_RIGHT_PAREN);
        return NULL;
    }

    return func;
}

static AST_Expression *parse_postfix(Parser *parser) {
    AST_Expression *exp = parse_primary(parser);
    if (!exp) return NULL;

    while (1) {
        switch (parser->current.type) {
            case TOKEN_LEFT_PAREN:
                exp = parse_call(parser, exp);
                if (!exp) return NULL;
                break;

            case TOKEN_DOT:
                if (!parser_match(parser, TOKEN_DOT)) {
                    ast_expression_free(exp);
                    parser_error(parser, PARSER_ERROR_EXPECTED_DOT);
                    return NULL;
                }

                if (parser->current.type == TOKEN_LEFT_BRACKET) {
                    exp = parse_location_access(parser, exp);
                } 
                else exp = parse_member_access(parser, exp);

                if (!exp) return NULL;
                break;

            default: return exp;
        }
    }
}

static AST_Expression *parse_unary(Parser *parser) {
    if (parser->current.type == TOKEN_PLUS || parser->current.type == TOKEN_MINUS || parser->current.type == TOKEN_EXCLAM) {
        TokenType op = parser->current.type;
        if (!parser_next(parser)) return NULL;

        AST_Expression *operand = parse_unary(parser);
        if (!operand) return NULL;

        AST_Expression *exp = malloc(sizeof(*exp));
        if (!exp) {
            ast_expression_free(operand);
            return NULL;
        }

        exp->type = AST_EX_UNARY;
        exp->unary.operand = operand;

        switch (op) {
            case TOKEN_PLUS: exp->unary.op = AST_OP_PLUS; break;

            case TOKEN_MINUS: exp->unary.op = AST_OP_MINUS; break;

            case TOKEN_EXCLAM: exp->unary.op = AST_OP_NOT; break;

            default: break;
        }
        return exp;
    }
    return parse_postfix(parser);
}

static AST_Expression *parse_multiplicative(Parser *parser) {
    AST_Expression *left = parse_unary(parser);
    if (!left) return NULL;

    while (parser->current.type == TOKEN_ASTER || parser->current.type == TOKEN_SLASH) {
        TokenType op = parser->current.type;

        if (!parser_next(parser)) {
            ast_expression_free(left);
            return NULL;
        }

        AST_Expression *right = parse_unary(parser);
        if (!right) {
            ast_expression_free(left);
            return NULL;
        }

        AST_Expression *binary = malloc(sizeof(*binary));
        if (!binary) {
            ast_expression_free(left);
            ast_expression_free(right);
            return NULL;
        }

        *binary = (AST_Expression) {
            .type = AST_EX_BINARY,
            .binary = {
                .left = left,
                .right = right,
            },
        };

        switch (op) {
            case TOKEN_ASTER: binary->binary.op = AST_OP_MULTIPLY; break;

            case TOKEN_SLASH: binary->binary.op = AST_OP_DIVIDE; break;

            default: break;
        }

        left = binary;
    }

    return left;
}

static AST_Expression *parse_additive(Parser *parser) {
    AST_Expression *left = parse_multiplicative(parser);
    if (!left) return NULL;

    while (parser->current.type == TOKEN_PLUS || parser->current.type == TOKEN_MINUS) {
        TokenType op = parser->current.type;

        if (!parser_next(parser)) {
            ast_expression_free(left);
            return NULL;
        }

        AST_Expression *right = parse_multiplicative(parser);
        if (!right) {
            ast_expression_free(left);
            return NULL;
        }

        AST_Expression *binary = malloc(sizeof(*binary));
        if (!binary) {
            ast_expression_free(left);
            ast_expression_free(right);
            return NULL;
        }

        *binary = (AST_Expression) {
            .type = AST_EX_BINARY,
            .binary = {
                .left = left,
                .right = right,
            },
        };

        switch (op) {
            case TOKEN_PLUS: binary->binary.op = AST_OP_PLUS; break;

            case TOKEN_MINUS: binary->binary.op = AST_OP_MINUS; break;

            default: break;
        }

        left = binary;
    }
    return left;
}

static AST_Expression *parse_comparison(Parser *parser) {
    AST_Expression *left = parse_additive(parser);
    if (!left) return NULL;

    while (parser->current.type == TOKEN_LESS || parser->current.type == TOKEN_GREATER 
        || parser->current.type == TOKEN_LESS_EQUAL || parser->current.type == TOKEN_GREATER_EQUAL) {

        TokenType op = parser->current.type;

        if (!parser_next(parser)) {
            ast_expression_free(left);
            return NULL;
        }

        AST_Expression *right = parse_additive(parser);
        if (!right) {
            ast_expression_free(left);
            return NULL;
        }

        AST_Expression *binary = malloc(sizeof(*binary));
        if (!binary) {
            ast_expression_free(left);
            ast_expression_free(right);
            return NULL;
        }

        *binary = (AST_Expression) {
            .type = AST_EX_BINARY,
            .binary = {
                .left = left,
                .right = right,
            },
        };

        switch (op) {
            case TOKEN_LESS: binary->binary.op = AST_OP_LESS; break;

            case TOKEN_GREATER: binary->binary.op = AST_OP_GREATER; break;

            case TOKEN_LESS_EQUAL: binary->binary.op = AST_OP_LESS_EQUAL; break;

            case TOKEN_GREATER_EQUAL: binary->binary.op = AST_OP_GREATER_EQUAL; break;

            default:
                ast_expression_free(binary);
                return NULL;
        }

        left = binary;
    }

    return left;
}

static AST_Expression *parse_equality(Parser *parser) {
    AST_Expression *left = parse_comparison(parser);
    if (!left) return NULL;

    while (parser->current.type == TOKEN_EQUAL_EQUAL || parser->current.type == TOKEN_NOT_EQUAL) {

        TokenType op = parser->current.type;

        if (!parser_next(parser)) {
            ast_expression_free(left);
            return NULL;
        }

        AST_Expression *right = parse_comparison(parser);
        if (!right) {
            ast_expression_free(left);
            return NULL;
        }

        AST_Expression *binary = malloc(sizeof(*binary));
        if (!binary) {
            ast_expression_free(left);
            ast_expression_free(right);
            return NULL;
        }

        *binary = (AST_Expression) {
            .type = AST_EX_BINARY,
            .binary = {
                .left = left,
                .right = right,
            },
        };

        switch (op) {
            case TOKEN_EQUAL_EQUAL: binary->binary.op = AST_OP_EQUAL; break;

            case TOKEN_NOT_EQUAL: binary->binary.op = AST_OP_NOT_EQUAL; break;

            default:
                ast_expression_free(binary);
                return NULL;
        }

        left = binary;
    }

    return left;
}

static AST_Expression *parse_logical_and(Parser *parser) {
    AST_Expression *left = parse_equality(parser);
    if (!left) return NULL;

    while (parser->current.type == TOKEN_AMPERS_AMPERS) {
        if (!parser_next(parser)) {
            ast_expression_free(left);
            return NULL;
        }

        AST_Expression *right = parse_equality(parser);
        if (!right) {
            ast_expression_free(left);
            return NULL;
        }

        AST_Expression *binary = malloc(sizeof(*binary));
        if (!binary) {
            ast_expression_free(left);
            ast_expression_free(right);
            return NULL;
        }

        *binary = (AST_Expression) {
            .type = AST_EX_BINARY,
            .binary = {
                .left = left,
                .right = right,
                .op = AST_OP_AND,
            },
        };

        left = binary;
    }

    return left;
}

static AST_Expression *parse_logical_or(Parser *parser) {
    AST_Expression *left = parse_logical_and(parser);
    if (!left) return NULL;

    while (parser->current.type == TOKEN_BAR_BAR) {
        if (!parser_next(parser)) {
            ast_expression_free(left);
            return NULL;
        }

        AST_Expression *right = parse_logical_and(parser);
        if (!right) {
            ast_expression_free(left);
            return NULL;
        }

        AST_Expression *binary = malloc(sizeof(*binary));
        if (!binary) {
            ast_expression_free(left);
            ast_expression_free(right);
            return NULL;
        }

        *binary = (AST_Expression) {
            .type = AST_EX_BINARY,
            .binary = {
                .left = left,
                .right = right,
                .op = AST_OP_OR,
            },
        };

        left = binary;
    }

    return left;
}

static AST_Expression *parse_expression(Parser *parser) {
    return parse_logical_or(parser);
}

static AST_Block *parse_statement_or_block(Parser *parser) {
    AST_Block *block = malloc(sizeof(*block));
    if (!block) return NULL;
    *block = (AST_Block){0};

    if (parser->current.type == TOKEN_LEFT_BRACE) {
        if (!parser_next(parser)) {
            ast_block_free(block);
            return NULL;
        }

        while (parser->current.type != TOKEN_RIGHT_BRACE && parser->current.type != TOKEN_EOF) {
            AST_Node *statement = parse_statement(parser);
            if (!statement) {
                ast_block_free(block);
                return NULL;
            }

            if (!block_add_statement(block, statement)) {
                ast_node_free(statement);
                ast_block_free(block);
                return NULL;
            }
        }

        if (!parser_match(parser, TOKEN_RIGHT_BRACE)) {
            ast_block_free(block);
            parser_error(parser, PARSER_ERROR_EXPECTED_RIGHT_BRACE);
            return NULL;
        }
    }
    else {
        AST_Node *statement = parse_statement(parser);
        if (!statement) {
            ast_block_free(block);
            return NULL;
        }

        if (!block_add_statement(block, statement)) {
            ast_node_free(statement);
            ast_block_free(block);
            return NULL;
        }
    }

    return block;
}

static AST_Node *parse_variable_assignment(Parser *parser, AST_Expression *target) {
    if(!parser_next(parser)) { // skip =
        ast_expression_free(target);
        return NULL;
    }

    AST_Expression *exp = parse_expression(parser);
    if (!exp) {
        ast_expression_free(target);
        return NULL;
    }

    if (!parser_match(parser, TOKEN_SEMICOLON)) {
        ast_expression_free(target);
        ast_expression_free(exp);
            parser_error(parser, PARSER_ERROR_EXPECTED_SEMICOLON);
        return NULL;
    }

    AST_Node *node = malloc(sizeof(*node));
    if (!node) {
        ast_expression_free(target);
        ast_expression_free(exp);
        return NULL;
    }

    *node = (AST_Node){
        .type = AST_ASSIGNMENT,
        .assignment = (AST_Assignment){
            .target = target,
            .value = exp,
        }
    };

    return node;
}

static AST_Node *parse_variable_declaration(Parser *parser, AST_Expression *target) {
    if (target->type != AST_EX_VARIABLE) {
        ast_expression_free(target);
        parser_error(parser, PARSER_ERROR_UNEXPECTED_TOKEN);
        return NULL; 
    }

    AST_Location loc = {0};
    if(!parse_location(parser, &loc)) {
        ast_expression_free(target);
        return NULL;
    }

    AST_Expression *initializer = NULL;
    if (parser->current.type == TOKEN_EQUAL) {
        if (!parser_next(parser)) {
            ast_expression_free(target);
            ast_expression_free(loc.offset);
            ast_expression_free(loc.size);
            return NULL;
        }

        initializer = parse_expression(parser);
        if (!initializer) {
            ast_expression_free(target);
            ast_expression_free(loc.offset);
            ast_expression_free(loc.size);
            return NULL;
        }
    }

    if (!parser_match(parser, TOKEN_SEMICOLON)) {
        ast_expression_free(target);
        ast_expression_free(loc.offset);
        ast_expression_free(loc.size);
        ast_expression_free(initializer);
        parser_error(parser, PARSER_ERROR_EXPECTED_SEMICOLON);
        return NULL;
    }

    AST_Node *node = malloc(sizeof(*node));
    if (!node) {
        ast_expression_free(target);
        ast_expression_free(loc.offset);
        ast_expression_free(loc.size);
        ast_expression_free(initializer);
        return NULL; 
    }

    *node = (AST_Node) {
        .type = AST_VARIABLE_DECLARATION,
        .variableDeclaration = (AST_VariableDeclaration) {
            .location = loc,
            .var = target->variable,
            .initializer = initializer,
        }
    };

    ast_expression_free(target);
    return node;
}

static AST_Node *parse_expression_statement(Parser *parser, AST_Expression *target) {
    if (!parser_match(parser, TOKEN_SEMICOLON)) {
        ast_expression_free(target);
        parser_error(parser, PARSER_ERROR_EXPECTED_SEMICOLON);
        return NULL;
    }

    AST_Node *node = malloc(sizeof(*node));
    if (!node) {
        ast_expression_free(target);
        return NULL;
    }

    *node = (AST_Node){
        .type = AST_EXPRESSION_STATEMENT,
        .expressionStatement = {
            .expression = target,
        },
    };
    return node;
}

static AST_Node *parse_identifier_statement(Parser *parser) {
    AST_Expression *target = parse_postfix(parser);
    if (!target) return NULL;

    switch (parser->current.type) {
        case TOKEN_LEFT_BRACKET: return parse_variable_declaration(parser, target);

        case TOKEN_EQUAL: return parse_variable_assignment(parser, target);

        case TOKEN_SEMICOLON: return parse_expression_statement(parser, target);

        default: ast_expression_free(target); parser_error(parser, PARSER_ERROR_UNEXPECTED_TOKEN); return NULL;
    }
}

static AST_Node *parse_location_assignment(Parser *parser) {
    AST_Location loc = {0};
    if(!parse_location(parser, &loc)) return NULL;

    if(!parser_match(parser, TOKEN_EQUAL)) {
        ast_expression_free(loc.offset);
        ast_expression_free(loc.size);
        parser_error(parser, PARSER_ERROR_EXPECTED_ASSIGNMENT);
        return NULL;
    }

    AST_Expression *exp = parse_expression(parser);
    if (!exp) {
        ast_expression_free(loc.offset);
        ast_expression_free(loc.size);
        return NULL;
    }

    if (!parser_match(parser, TOKEN_SEMICOLON)) {
        ast_expression_free(loc.offset);
        ast_expression_free(loc.size);
        ast_expression_free(exp);
        parser_error(parser, PARSER_ERROR_EXPECTED_SEMICOLON);
        return NULL;
    }

    AST_Expression *location = malloc(sizeof(*location));
    if (!location) {
        ast_expression_free(loc.offset);
        ast_expression_free(loc.size);
        ast_expression_free(exp);
        return NULL;
    }
    *location = (AST_Expression) {
        .type = AST_EX_LOCATION,
        .location = loc,
    };

    AST_Node *node = malloc(sizeof(*node));
    if (!node) {
        ast_expression_free(exp);
        ast_expression_free(location);
        return NULL;
    }

    *node = (AST_Node) {
        .type = AST_ASSIGNMENT,
        .assignment = (AST_Assignment) {
            .target = location,
            .value = exp,
        },
    };
    return node;
}

static AST_Node *parse_struct_declaration(Parser *parser) {
    if (!parser_match(parser, TOKEN_STRUCT)) {
        parser_error(parser, PARSER_ERROR_UNEXPECTED_TOKEN);
        return NULL;
    }

    Token identifier = parser->current;
    if (!parser_match(parser, TOKEN_IDENTIFIER)) {
        parser_error(parser, PARSER_ERROR_EXPECTED_IDENTIFIER);
        return NULL;
    }

    if (!parser_match(parser, TOKEN_LEFT_BRACE)) {
        parser_error(parser, PARSER_ERROR_EXPECTED_LEFT_BRACE);
        return NULL;
    }

    AST_Node *node = malloc(sizeof(*node));
    if (!node) return NULL;

    *node = (AST_Node) {
        .type = AST_STRUCT_DECLARATION,
        .structDeclaration = {
            .name = identifier.start,
            .length = identifier.length,
            .fields = NULL,
            .fieldCount = 0,
        },
    };

    size_t capacity = 8;
    node->structDeclaration.fields = malloc(sizeof(AST_Field) * capacity);
    if (!node->structDeclaration.fields) {
        ast_node_free(node);
        return NULL;
    }

    while (parser->current.type != TOKEN_EOF && parser->current.type != TOKEN_RIGHT_BRACE) {
        AST_Field field = {0};
        if (!parse_field(parser, &field)) {
            ast_node_free(node);
            return NULL;
        }

        if (node->structDeclaration.fieldCount >= capacity) {
            capacity *= 2;

            AST_Field *fields = realloc(node->structDeclaration.fields, sizeof(AST_Field) * capacity);
            if (!fields) {
                ast_expression_free(field.offset);
                ast_expression_free(field.size);
                ast_node_free(node);
                return NULL;
            }

            node->structDeclaration.fields = fields;
        }

        node->structDeclaration.fields[node->structDeclaration.fieldCount++] = field;

        if (parser->current.type == TOKEN_COMMA) {
            if (!parser_next(parser)) {
                ast_node_free(node);
                return NULL;
            }
            if (parser->current.type == TOKEN_RIGHT_BRACE) break;
        } else break;
    }

    if (!parser_match(parser, TOKEN_RIGHT_BRACE)) {
        ast_node_free(node);
        parser_error(parser, PARSER_ERROR_EXPECTED_RIGHT_BRACE);
        return NULL;
    }

    return node;
}

static AST_Node *parse_function_declaration(Parser *parser) {
    if(!parser_match(parser, TOKEN_FP)) {
        parser_error(parser, PARSER_ERROR_UNEXPECTED_TOKEN);
        return NULL;
    }

    Token identifier = parser->current;
    if (!parser_match(parser, TOKEN_IDENTIFIER)) {
        parser_error(parser, PARSER_ERROR_EXPECTED_IDENTIFIER);
        return NULL;
    }

    if (!parser_match(parser, TOKEN_LEFT_PAREN)) {
        parser_error(parser, PARSER_ERROR_EXPECTED_LEFT_PAREN);
        return NULL;
    }

    AST_Node *node = malloc(sizeof(*node));
    if (!node) return NULL;

    *node = (AST_Node) {
        .type = AST_FUNCTION_DECLARATION,
        .functionDeclaration = {
            .name = identifier.start,
            .length = identifier.length,
            .parameters = NULL,
            .parameterCount = 0,
            .body = NULL,
        },
    };

    // parameters
    size_t capacity = 8;
    node->functionDeclaration.parameters = malloc(sizeof(AST_Field) * capacity);
    if (!node->functionDeclaration.parameters) {
        ast_node_free(node);
        return NULL;
    }

    while (parser->current.type != TOKEN_RIGHT_PAREN && parser->current.type != TOKEN_EOF) {
        AST_Field field = {0};
        if (!parse_field(parser, &field)) {
            ast_node_free(node);
            return NULL;
        }

        if (node->functionDeclaration.parameterCount >= capacity) {
            capacity *= 2;

            AST_Field *fields = realloc(node->functionDeclaration.parameters, sizeof(AST_Field) * capacity);
            if (!fields) {
                ast_expression_free(field.offset);
                ast_expression_free(field.size);
                ast_node_free(node);
                return NULL;
            }

            node->functionDeclaration.parameters = fields;
        }

        node->functionDeclaration.parameters[node->functionDeclaration.parameterCount++] = field;

        if (parser->current.type == TOKEN_COMMA) {
            if (!parser_next(parser)) {
                ast_node_free(node);
                return NULL;
            }
            if (parser->current.type == TOKEN_RIGHT_PAREN) {
                ast_node_free(node);
                parser_error(parser, PARSER_ERROR_UNEXPECTED_TOKEN);
                return NULL;
            }
        } else break;
    }

    if (!parser_match(parser, TOKEN_RIGHT_PAREN)) {
        ast_node_free(node);
        parser_error(parser, PARSER_ERROR_EXPECTED_RIGHT_PAREN);
        return NULL;
    }

    // parse size
    if (!parser_match(parser, TOKEN_LEFT_BRACKET)) {
        ast_node_free(node);
        parser_error(parser, PARSER_ERROR_EXPECTED_LEFT_BRACKET);
        return NULL;
    }
    AST_Expression *size = parse_expression(parser);
    if (!size) {
        ast_node_free(node);
        return NULL;
    }
    node->functionDeclaration.returnSize = size;

    if (!parser_match(parser, TOKEN_RIGHT_BRACKET)) {
        ast_node_free(node);
        parser_error(parser, PARSER_ERROR_EXPECTED_RIGHT_BRACKET);
        return NULL;
    }

    // parse body
    node->functionDeclaration.body = parse_statement_or_block(parser);
    if (!node->functionDeclaration.body) {
        ast_node_free(node);
        return NULL;
    }

    return node;
}

static AST_Node *parse_return_statement(Parser *parser) {
    if(!parser_match(parser, TOKEN_RETURN)) {
        parser_error(parser, PARSER_ERROR_UNEXPECTED_TOKEN);
        return NULL;
    }

    AST_Expression *value = NULL;

    if (parser->current.type != TOKEN_SEMICOLON) {
        value = parse_expression(parser);
        if (!value) return NULL;
    }

    if (!parser_match(parser, TOKEN_SEMICOLON)) {
        ast_expression_free(value);
        parser_error(parser, PARSER_ERROR_EXPECTED_SEMICOLON);
        return NULL;
    }

    AST_Node *node = malloc(sizeof(*node));
    if (!node) {
        ast_expression_free(value);
        return NULL;
    }

    *node = (AST_Node){
        .type = AST_RETURN,
        .returnStatement = {
            .value = value,
        },
    };
    return node;
}

static AST_Node *parse_if_statement(Parser *parser) {
    if (!parser_match(parser, TOKEN_IF)) {
        parser_error(parser, PARSER_ERROR_UNEXPECTED_TOKEN);
        return NULL;
    }

    if (!parser_match(parser, TOKEN_LEFT_PAREN)) {
        parser_error(parser, PARSER_ERROR_EXPECTED_LEFT_PAREN);
        return NULL;
    }

    AST_Expression *condition = parse_expression(parser);
    if (!condition) return NULL;

    if (!parser_match(parser, TOKEN_RIGHT_PAREN)) {
        ast_expression_free(condition);
        parser_error(parser, PARSER_ERROR_EXPECTED_RIGHT_PAREN);
        return NULL;
    }

    AST_Block *thenBody = parse_statement_or_block(parser);
    if (!thenBody) {
        ast_expression_free(condition);
        return NULL;
    }

    AST_Block *elseBody = NULL;
    if (parser_match(parser, TOKEN_ELSE)) {
        elseBody = parse_statement_or_block(parser);
        if (!elseBody) {
            ast_expression_free(condition);
            ast_block_free(thenBody);
            return NULL;
        }
    }

    AST_Node *node = malloc(sizeof(*node));
    if (!node) {
        ast_expression_free(condition);
        ast_block_free(thenBody);
        ast_block_free(elseBody);
        return NULL;
    }

    *node = (AST_Node) {
        .type = AST_IF,
        .conditional = (AST_Conditional) {
            .condition = condition,
            .thenBody = thenBody,
            .elseBody = elseBody,
        },
    };

    return node;
}

static AST_Node *parse_while_statement(Parser *parser) {
    if (!parser_match(parser, TOKEN_WHILE)) {
        parser_error(parser, PARSER_ERROR_UNEXPECTED_TOKEN);
        return NULL;
    }

    if (!parser_match(parser, TOKEN_LEFT_PAREN)) {
        parser_error(parser, PARSER_ERROR_EXPECTED_LEFT_PAREN);
        return NULL;
    }

    AST_Expression *condition = parse_expression(parser);
    if (!condition) return NULL;

    if (!parser_match(parser, TOKEN_RIGHT_PAREN)) {
        parser_error(parser, PARSER_ERROR_EXPECTED_RIGHT_PAREN);
        ast_expression_free(condition);
        return NULL;
    }

    AST_Block *body = parse_statement_or_block(parser);
    if (!body) {
        ast_expression_free(condition);
        return NULL;
    }

    AST_Node *node = malloc(sizeof(*node));
    if (!node) {
        ast_expression_free(condition);
        ast_block_free(body);
        return NULL;
    }

    *node = (AST_Node) {
        .type = AST_WHILE,
        .loop = (AST_Loop) {
            .condition = condition,
            .body = body,
        },
    };

    return node;
}

static AST_Node *parse_break_statement(Parser *parser) {
    if (!parser_match(parser, TOKEN_BREAK)) {
        parser_error(parser, PARSER_ERROR_UNEXPECTED_TOKEN);
        return NULL;
    }

    if (!parser_match(parser, TOKEN_SEMICOLON)) {
        parser_error(parser, PARSER_ERROR_EXPECTED_SEMICOLON);
        return NULL;
    }

    AST_Node *node = malloc(sizeof(*node));
    if (!node) return NULL;

    *node = (AST_Node){ .type = AST_BREAK };

    return node;
}

static AST_Node *parse_continue_statement(Parser *parser) {
    if (!parser_match(parser, TOKEN_CONTINUE)) {
        parser_error(parser, PARSER_ERROR_UNEXPECTED_TOKEN);
        return NULL;
    }

    if (!parser_match(parser, TOKEN_SEMICOLON)) {
        parser_error(parser, PARSER_ERROR_EXPECTED_SEMICOLON);
        return NULL;
    }

    AST_Node *node = malloc(sizeof(*node));
    if (!node) return NULL;

    *node = (AST_Node){ .type = AST_CONTINUE };
    return node;
}

static AST_Node *parse_sp_assignment(Parser *parser, AST_Expression *target) {
    if(!parser_match(parser, TOKEN_EQUAL)) {
        ast_expression_free(target);
        parser_error(parser, PARSER_ERROR_EXPECTED_ASSIGNMENT);
        return NULL;
    }

    AST_Expression *value = parse_expression(parser);
    if (!value){
        ast_expression_free(target);
        return NULL;
    }

    if (!parser_match(parser, TOKEN_SEMICOLON)) {
        ast_expression_free(value);
        ast_expression_free(target);
        parser_error(parser, PARSER_ERROR_EXPECTED_SEMICOLON);
        return NULL;
    }

    AST_Node *node = malloc(sizeof(*node));
    if (!node) {
        ast_expression_free(value);
        ast_expression_free(target);
        return NULL;
    }

    *node = (AST_Node){
        .type = AST_ASSIGNMENT,
        .assignment = {
            .target = target,
            .value = value,
        },
    };
    return node;
}

static AST_Node *parse_sp_location(Parser *parser, AST_Expression *target) {
    if (target->type != AST_EX_SP) {
        ast_expression_free(target);
        parser_error(parser, PARSER_ERROR_UNEXPECTED_TOKEN);
        return NULL;
    }
    ast_expression_free(target);

    AST_Location loc = {0};
    if (!parse_location(parser, &loc)) {
        return NULL;
    }

    AST_Expression *initializer = NULL;
    if (parser->current.type == TOKEN_EQUAL) {
        if(!parser_next(parser)) {
            ast_expression_free(loc.size);
            ast_expression_free(loc.offset);
            return NULL;
        }

        initializer = parse_expression(parser);
        if (!initializer) {
            ast_expression_free(loc.size);
            ast_expression_free(loc.offset);
            return NULL;
        }
    }

    if (!parser_match(parser, TOKEN_SEMICOLON)) {
        ast_expression_free(initializer);
        ast_expression_free(loc.size);
        ast_expression_free(loc.offset);
        parser_error(parser, PARSER_ERROR_EXPECTED_SEMICOLON);
        return NULL;
    }

    AST_Node *node = malloc(sizeof(*node));
    if (!node) {
        ast_expression_free(initializer);
        ast_expression_free(loc.size);
        ast_expression_free(loc.offset);
        return NULL;
    }

    *node = (AST_Node){
        .type = AST_SP_LOCATION,
        .spLocation = {
            .location = loc,
            .initializer = initializer,
        },
    };
    return node;
}

static AST_Node *parse_sp_statement(Parser *parser) {
    AST_Expression *target = parse_postfix(parser);
    if (!target) return NULL;

    switch (parser->current.type) {
        case TOKEN_LEFT_BRACKET: return parse_sp_location(parser, target);

        case TOKEN_EQUAL: return parse_sp_assignment(parser, target);

        default: ast_expression_free(target); parser_error(parser, PARSER_ERROR_UNEXPECTED_TOKEN); return NULL;
    }
}

static AST_Node *parse_statement(Parser *parser) {
    switch (parser->current.type) {
        case TOKEN_IDENTIFIER: return parse_identifier_statement(parser);
        
        case TOKEN_LEFT_BRACKET: return parse_location_assignment(parser);

        case TOKEN_IF: return parse_if_statement(parser);

        case TOKEN_WHILE: return parse_while_statement(parser);

        case TOKEN_STRUCT: return parse_struct_declaration(parser);
        
        case TOKEN_FP: return parse_function_declaration(parser);

        case TOKEN_RETURN: return parse_return_statement(parser);
        
        case TOKEN_BREAK: return parse_break_statement(parser);

        case TOKEN_CONTINUE: return parse_continue_statement(parser);

        case TOKEN_SP: return parse_sp_statement(parser); 

        default: parser_error(parser, PARSER_ERROR_INVALID_STATEMENT); return NULL;
    }
}

AST_Program *parse_program(Parser *parser) {
    if (parser->hasError) return NULL;
    
    AST_Program *program = malloc(sizeof(*program));
    if (!program) return NULL;
    *program = (AST_Program){0};

    while (parser->current.type != TOKEN_EOF && !parser->hasError) {
        AST_Node *statement = parse_statement(parser);
        if (!statement) {
            ast_program_free(program);
            return NULL;
        }

        if (!program_add_statement(program, statement)) {
            ast_program_free(program);
            ast_node_free(statement);
            return NULL;
        }
    }

    if (parser->hasError) {
        ast_program_free(program);
        return NULL;
    }

    return program;
}