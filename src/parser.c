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
        free(loc->offset);
        return 0;
    }

    loc->size = parse_expression(parser);
    if (!loc->size) {
        free(loc->offset);
        return 0;
    }

    if (!parser_match(parser, TOKEN_RIGHT_BRACKET)) {
        free(loc->offset);
        free(loc->size);
        return 0;
    }

    return 1;
}

static int block_add_statement(AST_Block *block, AST_Node *statement) {
    if (block->count >= block->capacity) {
        int newCapacity = block->capacity == 0 ? 8 : block->capacity * 2;

        AST_Node **newStatements = realloc(block->statements, sizeof(AST_Node *) * newCapacity);
        if (!newStatements) return 0;

        block->statements = newStatements;
        block->capacity = newCapacity;
    }

    block->statements[block->count++] = statement;
    return 1;
}

static AST_Expression *parse_literal(Parser *parser) {
    AST_Expression *exp = malloc(sizeof(*exp));
    if (!exp) return NULL;

    exp->type = AST_EX_LITERAL;

    switch (parser->current.type) {
        case TOKEN_INT_LITERAL:
            exp->literal.type = AST_DATA_INT;
            exp->literal.intValue = token_to_int(&parser->current);
            break;

        case TOKEN_CHAR_LITERAL:
            exp->literal.type = AST_DATA_CHAR;
            exp->literal.charValue = token_to_char(&parser->current);
            break;

        case TOKEN_FLOAT_LITERAL:
            exp->literal.type = AST_DATA_FLOAT;
            exp->literal.floatValue = token_to_float(&parser->current);
            break;

        case TOKEN_TRUE:
            exp->literal.type = AST_DATA_BOOL;
            exp->literal.boolValue = 1;
            break;
        
        case TOKEN_FALSE:
            exp->literal.type = AST_DATA_BOOL;
            exp->literal.boolValue = 0;
            break;

        default:
            free(exp);
            return NULL;
    }
    if (!parser_next(parser)) {
        free(exp);
        return NULL;
    }

    return exp;
}

static AST_Expression *parse_identifier(Parser *parser) {
    AST_Expression *exp = malloc(sizeof(*exp));
    if (!exp) return NULL;

    exp->type = AST_EX_VARIABLE;
    exp->variable.name = parser->current.start;
    exp->variable.length = parser->current.length;

    if (!parser_next(parser)) {
        free(exp);
        return NULL;
    }
    return exp;
}

static AST_Expression *parse_parenthesized(Parser *parser) {
    if (!parser_next(parser)) return NULL; // skip (

    AST_Expression *exp = parse_expression(parser);
    if (!exp) return NULL;

    if (!parser_match(parser, TOKEN_RIGHT_PAREN)) {
        free(exp);
        return NULL;
    }

    return exp;
}

static AST_Expression *parse_primary(Parser *parser) {
    switch (parser->current.type) {
        case TOKEN_INT_LITERAL: case TOKEN_FLOAT_LITERAL: case TOKEN_CHAR_LITERAL: case TOKEN_TRUE: case TOKEN_FALSE:
            return parse_literal(parser);

        case TOKEN_IDENTIFIER: return parse_identifier(parser);

        case TOKEN_LEFT_PAREN: return parse_parenthesized(parser);

        default: return NULL;
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
            free(operand);
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
    return parse_primary(parser);
}

static AST_Expression *parse_multiplicative(Parser *parser) {
    AST_Expression *left = parse_unary(parser);
    if (!left) return NULL;

    while (parser->current.type == TOKEN_ASTER || parser->current.type == TOKEN_SLASH) {
        TokenType op = parser->current.type;

        if (!parser_next(parser)) {
            free(left);
            return NULL;
        }

        AST_Expression *right = parse_unary(parser);
        if (!right) {
            free(left);
            return NULL;
        }

        AST_Expression *binary = malloc(sizeof(*binary));
        if (!binary) {
            free(left);
            free(right);
            return NULL;
        }

        binary->type = AST_EX_BINARY;
        binary->binary.left = left;
        binary->binary.right = right;

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
            free(left);
            return NULL;
        }

        AST_Expression *right = parse_multiplicative(parser);
        if (!right) {
            free(left);
            return NULL;
        }

        AST_Expression *binary = malloc(sizeof(*binary));
        if (!binary) {
            free(left);
            free(right);
            return NULL;
        }

        binary->type = AST_EX_BINARY;
        binary->binary.left = left;
        binary->binary.right = right;

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
            free(left);
            return NULL;
        }

        AST_Expression *right = parse_additive(parser);
        if (!right) {
            free(left);
            return NULL;
        }

        AST_Expression *binary = malloc(sizeof(*binary));
        if (!binary) {
            free(left);
            free(right);
            return NULL;
        }

        binary->type = AST_EX_BINARY;
        binary->binary.left = left;
        binary->binary.right = right;

        switch (op) {
            case TOKEN_LESS: binary->binary.op = AST_OP_LESS; break;

            case TOKEN_GREATER: binary->binary.op = AST_OP_GREATER; break;

            case TOKEN_LESS_EQUAL: binary->binary.op = AST_OP_LESS_EQUAL; break;

            case TOKEN_GREATER_EQUAL: binary->binary.op = AST_OP_GREATER_EQUAL; break;

            default:
                free(binary);
                free(left);
                free(right);
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
            free(left);
            return NULL;
        }

        AST_Expression *right = parse_comparison(parser);
        if (!right) {
            free(left);
            return NULL;
        }

        AST_Expression *binary = malloc(sizeof(*binary));
        if (!binary) {
            free(left);
            free(right);
            return NULL;
        }

        binary->type = AST_EX_BINARY;
        binary->binary.left = left;
        binary->binary.right = right;

        switch (op) {
            case TOKEN_EQUAL_EQUAL: binary->binary.op = AST_OP_EQUAL; break;

            case TOKEN_NOT_EQUAL: binary->binary.op = AST_OP_NOT_EQUAL; break;

            default:
                free(binary);
                free(left);
                free(right);
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
            free(left);
            return NULL;
        }

        AST_Expression *right = parse_equality(parser);
        if (!right) {
            free(left);
            return NULL;
        }

        AST_Expression *binary = malloc(sizeof(*binary));
        if (!binary) {
            free(left);
            free(right);
            return NULL;
        }

        binary->type = AST_EX_BINARY;
        binary->binary.left = left;
        binary->binary.right = right;
        binary->binary.op = AST_OP_AND;

        left = binary;
    }

    return left;
}

static AST_Expression *parse_logical_or(Parser *parser) {
    AST_Expression *left = parse_logical_and(parser);
    if (!left) return NULL;

    while (parser->current.type == TOKEN_BAR_BAR) {
        if (!parser_next(parser)) {
            free(left);
            return NULL;
        }

        AST_Expression *right = parse_logical_and(parser);
        if (!right) {
            free(left);
            return NULL;
        }

        AST_Expression *binary = malloc(sizeof(*binary));
        if (!binary) {
            free(left);
            free(right);
            return NULL;
        }

        binary->type = AST_EX_BINARY;
        binary->binary.left = left;
        binary->binary.right = right;
        binary->binary.op = AST_OP_OR;

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
        parser_next(parser);

        while (parser->current.type != TOKEN_RIGHT_BRACE && parser->current.type != TOKEN_EOF) {

            AST_Node *statement = parse_statement(parser);
            if (!statement) {
                free(block);
                return NULL;
            }

            if (!block_add_statement(block, statement)) {
                free(statement);
                free(block);
                return NULL;
            }
        }

        if (!parser_match(parser, TOKEN_RIGHT_BRACE)) {
            free(block);
            return NULL;
        }
    }
    else {
        AST_Node *statement = parse_statement(parser);
        if (!statement) {
            free(block);
            return NULL;
        }

        if (!block_add_statement(block, statement)) {
            free(statement);
            free(block);
            return NULL;
        }
    }

    return block;
}

static AST_Node *parse_variable_assignment(Parser *parser, Token identifier) {
    if(!parser_next(parser)) return NULL; // skip =

    AST_Expression *exp = parse_expression(parser);
    if (!exp) return NULL;

    AST_Node *node = malloc(sizeof(*node));
    if (!node) {
        free(exp);
        return NULL;
    }

    AST_Expression *var = malloc(sizeof(*var));
    if (!var) {
        free(exp);
        return NULL;
    }
    *var = (AST_Expression){
        .type = AST_EX_VARIABLE,
        .variable = (AST_Variable){
            .name = identifier.start,
            .length = identifier.length,
        },
    };

    *node = (AST_Node){
        .type = AST_ASSIGNMENT,
        .assignment = (AST_Assignment){
            .target = var,
            .value = exp,
        }
    };

    return node;
}

static AST_Node *parse_variable_declaration(Parser *parser, Token identifier) {
    
    AST_Location loc = {0};
    if(!parse_location(parser, &loc)) return NULL;

    AST_Expression *initializer = NULL;
    if (parser->current.type == TOKEN_EQUAL) {
        if (!parser_next(parser)) {
            free(loc.offset);
            free(loc.size);
            return NULL;
        }

        initializer = parse_expression(parser);
        if (!initializer) {
            free(loc.offset);
            free(loc.size);
            return NULL;
        }
    }

    AST_Node *node = malloc(sizeof(*node));
    if (!node) {
        free(loc.offset);
        free(loc.size);
        free(initializer);
        return NULL; 
    }
    node->type = AST_VARIABLE_DECLARATION;
    node->variableDeclaration.location = loc;
    node->variableDeclaration.name = identifier.start;
    node->variableDeclaration.length = identifier.length;
    node->variableDeclaration.initializer = initializer;

    return node;
}

static AST_Node *parse_identifier_statement(Parser *parser) {
    Token identifier = parser->current;

    if (!parser_next(parser)) return NULL; // skip identifier

    switch (parser->current.type) {
        case TOKEN_LEFT_BRACKET: return parse_variable_declaration(parser, identifier);

        case TOKEN_EQUAL: return parse_variable_assignment(parser, identifier);

        default: return NULL;
    }
}

static AST_Node *parse_location_assignment(Parser *parser) {
    AST_Location loc = {0};
    if(!parse_location(parser, &loc)) return NULL;

    if(!parser_match(parser, TOKEN_EQUAL)) {
        free(loc.offset);
        free(loc.size);
        return NULL;
    }

    AST_Expression *exp = parse_expression(parser);
    if (!exp) {
        free(loc.offset);
        free(loc.size);
        return NULL;
    }

    AST_Expression *location = malloc(sizeof(*location));
    if (!location) {
        free(loc.offset);
        free(loc.size);
        free(exp);
        return NULL;
    }
    *location = (AST_Expression) {
        .type = AST_EX_LOCATION,
        .location = loc,
    };

    AST_Node *node = malloc(sizeof(*node));
    if (!node) {
        free(location);
        free(loc.offset);
        free(loc.size);
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

static AST_Node *parse_if_statement(Parser *parser) {
    if (!parser_next(parser)) return NULL; // skip if

    if (!parser_match(parser, TOKEN_LEFT_PAREN)) return NULL;

    AST_Expression *condition = parse_expression(parser);
    if (!condition) return NULL;

    if (!parser_match(parser, TOKEN_RIGHT_PAREN)) {
        free(condition);
        return NULL;
    }

    AST_Block *thenBody = parse_statement_or_block(parser);
    if (!thenBody) {
        free(condition);
        return NULL;
    }

    AST_Block *elseBody = NULL;
    if (parser_match(parser, TOKEN_ELSE)) {
        elseBody = parse_statement_or_block(parser);
        if (!elseBody) {
            free(condition);
            free(thenBody);
            return NULL;
        }
    }

    AST_Node *node = malloc(sizeof(*node));
    if (!node) {
        free(condition);
        free(thenBody);
        free(elseBody);
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
        free(condition);
        return NULL;
    }

    AST_Block *body = parse_statement_or_block(parser);
    if (!body) {
        free(condition);
        return NULL;
    }

    AST_Node *node = malloc(sizeof(*node));
    if (!node) {
        free(condition);
        free(body);
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

static AST_Node *parse_statement(Parser *parser) {
    AST_Node *node = NULL;

    switch (parser->current.type) {
        case TOKEN_IDENTIFIER:
            node = parse_identifier_statement(parser);

            if (!parser_match(parser, TOKEN_SEMICOLON)) {
                free(node);
                return NULL;
            }
            break;
        
        case TOKEN_LEFT_BRACKET:
            node = parse_location_assignment(parser);

            if (!parser_match(parser, TOKEN_SEMICOLON)) {
                free(node);
                return NULL;
            }

        case TOKEN_IF:
            node = parse_if_statement(parser);
            break;

        case TOKEN_WHILE:
            node = parse_while_statement(parser);
            break;

        default: return NULL;
    }

    if (!node) return NULL;

    return node;
}

AST_Program *parse_program(Parser *parser) {
    AST_Program *program = malloc(sizeof(*program));
    if (!program) return NULL;
    *program = (AST_Program){0};

    while (parser->current.type != TOKEN_EOF) {

        if (parser->current.type == TOKEN_ERROR) {
            free(program);
            return NULL;
        }

        AST_Node *statement = parse_statement(parser);

        if (!statement) {
            free(program);
            return NULL;
        }

        if (!block_add_statement(&program->block, statement)) {
            free(statement);
            free(program);
            return NULL;
        }
    }

    return program;
}