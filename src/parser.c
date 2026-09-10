#include "parser.h"
#include "ast.h"
#include "token.h"
#include <stdlib.h>

Parser parser_init(Lexer *lexer) {
    return (Parser) {
        .lexer = lexer,
        .current = lexer_next_token(lexer)
    };
}

static int parser_next(Parser *parser) {
    parser->current = lexer_next_token(parser->lexer);
    return parser->current.type != TOKEN_ERROR; 
}

static int parser_match(Parser *parser, TokenType type) {
    if (parser->current.type != type) return 0;
    return parser_next(parser);
}

static int parse_location(Parser *parser, AST_Location *loc) {
    *loc = (AST_Location){0};

    if (!parser_match(parser, TOKEN_LEFT_BRACKET)) return 0;

    switch (parser->current.type) {
        case TOKEN_SP: loc->base = AST_LOCATION_SP; break;
        case TOKEN_FP: loc->base = AST_LOCATION_FP; break;
        case TOKEN_BP: loc->base = AST_LOCATION_BP; break;
        case TOKEN_HP: loc->base = AST_LOCATION_HP; break;
        default: return 0;
    }

    if (!parser_next(parser)) return 0;

    if (parser->current.type != TOKEN_COMMA) {
        loc->offset = parse_expression(parser);
        if (!loc->offset) return 0;
    }

    if (!parser_match(parser, TOKEN_COMMA)) {
        ast_expression_free(loc->offset);
        return 0;
    }

    loc->size = parse_expression(parser);
    if (!loc->size) {
        ast_expression_free(loc->offset);
        return 0;
    }

    if (!parser_match(parser, TOKEN_RIGHT_BRACKET)) {
        ast_expression_free(loc->offset);
        ast_expression_free(loc->size);
        return 0;
    }

    return 1;
}

static int parse_field(Parser *parser, AST_Field *field) {
    Token identifier = parser->current;
    if (!parser_match(parser, TOKEN_IDENTIFIER)) return 0;

    if (!parser_match(parser, TOKEN_LEFT_BRACKET)) return 0;

    field->offset = parse_expression(parser);
    if (!field->offset) return 0;

    if (!parser_match(parser, TOKEN_COMMA)) {
        ast_expression_free(field->offset);
        return 0;
    }

    field->size = parse_expression(parser);
    if (!field->size) {
        ast_expression_free(field->offset);
        return 0;
    }

    if (!parser_match(parser, TOKEN_RIGHT_BRACKET)) {
        ast_expression_free(field->offset);
        ast_expression_free(field->size);
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
    if (!parser_match(parser, TOKEN_LEFT_PAREN)) return NULL;

    AST_Expression *exp = parse_expression(parser);
    if (!exp) return NULL;

    if (!parser_match(parser, TOKEN_RIGHT_PAREN)) {
        ast_expression_free(exp);
        return NULL;
    }

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
    
        default: return NULL;
    }
}

static AST_Expression *parse_member_access(Parser *parser, AST_Expression *exp) {
    if (!parser_match(parser, TOKEN_DOT)) {
        ast_expression_free(exp);
        return NULL;
    }

    Token member = parser->current;

    if (!parser_match(parser, TOKEN_IDENTIFIER)) {
        ast_expression_free(exp);
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
            if (!parser_next(parser) || parser->current.type == TOKEN_RIGHT_PAREN) {
                ast_expression_free(func);
                return NULL;
            }
        }
        else break;
    }

    if (!parser_match(parser, TOKEN_RIGHT_PAREN)) {
        ast_expression_free(func);
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
                exp = parse_member_access(parser, exp);
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

        default: ast_expression_free(target); return NULL;
    }
}

static AST_Node *parse_location_assignment(Parser *parser) {
    AST_Location loc = {0};
    if(!parse_location(parser, &loc)) return NULL;

    if(!parser_match(parser, TOKEN_EQUAL)) {
        ast_expression_free(loc.offset);
        ast_expression_free(loc.size);
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
    if (!parser_match(parser, TOKEN_STRUCT)) return NULL;

    Token identifier = parser->current;
    if (!parser_match(parser, TOKEN_IDENTIFIER)) return NULL;

    if (!parser_match(parser, TOKEN_LEFT_BRACE)) return NULL;

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
        return NULL;
    }

    return node;
}

static AST_Node *parse_function_declaration(Parser *parser) {
    if(!parser_match(parser, TOKEN_FP)) return NULL;

    Token identifier = parser->current;
    if(!parser_match(parser, TOKEN_IDENTIFIER)) return NULL;

    if (!parser_match(parser, TOKEN_LEFT_PAREN)) return NULL;

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
            if (!parser_next(parser) || parser->current.type == TOKEN_RIGHT_PAREN) {
                ast_node_free(node);
                return NULL;
            }
        } else break;
    }

    if (!parser_match(parser, TOKEN_RIGHT_PAREN)) {
        ast_node_free(node);
        return NULL;
    }

    // parse size
    if (!parser_match(parser, TOKEN_LEFT_BRACKET)) {
        ast_node_free(node);
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

static AST_Node *parse_return(Parser *parser) {
    if(!parser_match(parser, TOKEN_RETURN)) return NULL;

    AST_Expression *value = NULL;

    if (parser->current.type != TOKEN_SEMICOLON) {
        value = parse_expression(parser);
        if (!value) return NULL;
    }

    if (!parser_match(parser, TOKEN_SEMICOLON)) {
        ast_expression_free(value);
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
    if (!parser_next(parser)) return NULL; // skip if

    if (!parser_match(parser, TOKEN_LEFT_PAREN)) return NULL;

    AST_Expression *condition = parse_expression(parser);
    if (!condition) return NULL;

    if (!parser_match(parser, TOKEN_RIGHT_PAREN)) {
        ast_expression_free(condition);
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
    if (!parser_next(parser)) return NULL; // skip while

    if (!parser_match(parser, TOKEN_LEFT_PAREN)) return NULL;

    AST_Expression *condition = parse_expression(parser);
    if (!condition) return NULL;

    if (!parser_match(parser, TOKEN_RIGHT_PAREN)) {
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

static AST_Node *parse_break(Parser *parser) {
    if (!parser_match(parser, TOKEN_BREAK)) return NULL;

    if (!parser_match(parser, TOKEN_SEMICOLON)) return NULL;

    AST_Node *node = malloc(sizeof(*node));
    if (!node) return NULL;

    *node = (AST_Node){ .type = AST_BREAK };

    return node;
}

static AST_Node *parse_continue(Parser *parser) {
    if (!parser_match(parser, TOKEN_CONTINUE)) return NULL;

    if (!parser_match(parser, TOKEN_SEMICOLON)) return NULL;

    AST_Node *node = malloc(sizeof(*node));
    if (!node) return NULL;

    *node = (AST_Node){ .type = AST_CONTINUE };
    return node;
}

static AST_Node *parse_statement(Parser *parser) {
    switch (parser->current.type) {
        case TOKEN_IDENTIFIER: return parse_identifier_statement(parser);
        
        case TOKEN_LEFT_BRACKET: return parse_location_assignment(parser);

        case TOKEN_IF: return parse_if_statement(parser);

        case TOKEN_WHILE: return parse_while_statement(parser);

        case TOKEN_STRUCT: return parse_struct_declaration(parser);
        
        case TOKEN_FP: return parse_function_declaration(parser);

        case TOKEN_RETURN: return parse_return(parser);
        
        case TOKEN_BREAK: return parse_break(parser);

        case TOKEN_CONTINUE: return parse_continue(parser);

        default: return NULL;
    }
}

AST_Program *parse_program(Parser *parser) {
    AST_Program *program = malloc(sizeof(*program));
    if (!program) return NULL;
    *program = (AST_Program){0};

    while (parser->current.type != TOKEN_EOF) {

        if (parser->current.type == TOKEN_ERROR) {
            ast_program_free(program);
            return NULL;
        }

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

    return program;
}