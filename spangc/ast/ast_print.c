#include <stdio.h>
#include "ast_print.h"

static AST_PrintContext tree_child(AST_PrintContext *ctx, bool parentLast) {
    AST_PrintContext child = *ctx;

    if (ctx->depth < AST_PRINT_MAX_DEPTH) child.branch[child.depth++] = parentLast;

    return child;
}

static void tree_prefix(AST_PrintContext *ctx) {
    for (size_t i = 0; i < ctx->depth; i++) printf("%s", ctx->branch[i] ? "    " : "│   ");
}

static void tree_line(AST_PrintContext *ctx, bool last, const char *text) {
    tree_prefix(ctx);
    printf("%s%s\n", last ? "└── " : "├── ", text);
}

static const char *operator_name(AST_OperatorType op) {
    switch (op) {
        case AST_OP_PLUS: return "+";
        case AST_OP_MINUS: return "-";
        case AST_OP_MULTIPLY: return "*";
        case AST_OP_DIVIDE: return "/";
        case AST_OP_EQUAL: return "==";
        case AST_OP_NOT: return "!";
        case AST_OP_NOT_EQUAL: return "!=";
        case AST_OP_LESS: return "<";
        case AST_OP_LESS_EQUAL: return "<=";
        case AST_OP_GREATER: return ">";
        case AST_OP_GREATER_EQUAL: return ">=";
        case AST_OP_AND: return "&&";
        case AST_OP_OR: return "||";
    }
}

static const char *data_type_name(AST_DataType type) {
    switch (type) {
        case AST_DATA_I1: return "i1";
        case AST_DATA_I2: return "i2";
        case AST_DATA_I4: return "i4";
        case AST_DATA_I8: return "i8";

        case AST_DATA_U1: return "u1";
        case AST_DATA_U2: return "u2";
        case AST_DATA_U4: return "u4";
        case AST_DATA_U8: return "u8";

        case AST_DATA_C1: return "c1";
        case AST_DATA_C2: return "c2";
        case AST_DATA_C4: return "c4";

        case AST_DATA_F4: return "f4";
        case AST_DATA_F8: return "f8";

        case AST_DATA_B1: return "b1";
        case AST_DATA_V0: return "v0";
    }
}

static const char *base_type_name(AST_BaseType base) {
    switch (base) {
        case AST_LOCATION_SP: return "sp";
        case AST_LOCATION_FP: return "fp";
        case AST_LOCATION_HP: return "hp";
        case AST_LOCATION_BP: return "bp";
    }
}

static void literal_print(AST_Literal *literal, AST_PrintContext *ctx, bool last) {
    char buffer[AST_PRINT_MAX_DEPTH];

    switch (literal->type) {
        case AST_LITERAL_INT:
            snprintf(buffer, sizeof(buffer), "INT_LITERAL %llu", (unsigned long long)literal->intValue);
            break;

        case AST_LITERAL_FLOAT:
            snprintf(buffer, sizeof(buffer), "FLOAT_LITERAL %f", literal->floatValue);
            break;

        case AST_LITERAL_BOOL:
            snprintf(buffer, sizeof(buffer), "BOOL_LITERAL %s", literal->boolValue ? "true" : "false");
            break;

        case AST_LITERAL_C1:
            snprintf(buffer, sizeof(buffer), "C1_LITERAL %u", literal->c1Value);
            break;

        case AST_LITERAL_C2:
            snprintf(buffer, sizeof(buffer), "C2_LITERAL %u", literal->c2Value);
            break;

        case AST_LITERAL_C4:
            snprintf(buffer, sizeof(buffer), "C4_LITERAL %u", literal->c4Value);
            break;

        case AST_LITERAL_S1:
            snprintf(buffer, sizeof(buffer), "S1_LITERAL length=%zu", literal->length);
            break;

        case AST_LITERAL_S2:
            snprintf(buffer, sizeof(buffer), "S2_LITERAL length=%zu", literal->length);
            break;

        case AST_LITERAL_S4:
            snprintf(buffer, sizeof(buffer), "S4_LITERAL length=%zu", literal->length);
            break;
    }

    tree_line(ctx, last, buffer);
}

static void location_print(AST_Location *location, AST_PrintContext *ctx, bool last) {
    tree_line(ctx, last, "LOCATION");

    AST_PrintContext child = tree_child(ctx, last);
    tree_line(&child, false, "base");

    AST_PrintContext baseCtx = tree_child(&child, false);
    tree_line(&baseCtx, true, base_type_name(location->base));

    tree_line(&child,false, "offset:");

    AST_PrintContext offsetCtx = tree_child(&child, false);
    expression_print(location->offset, &offsetCtx, true);

    bool hasReadAs = location->readAs != NULL;

    tree_line(&child, !hasReadAs, "size:");

    AST_PrintContext sizeCtx = tree_child(&child, !hasReadAs);
    expression_print(location->size, &sizeCtx, true);

    if (hasReadAs) {
        tree_line(&child, true, "readAs:");

        AST_PrintContext readAsCtx = tree_child(&child, true);
        expression_print(location->readAs, &readAsCtx, true);
    }
}

static void field_print(AST_Field *field, AST_PrintContext *ctx, bool last) {
    char buffer[AST_PRINT_MAX_DEPTH];

    snprintf(buffer, sizeof(buffer), "FIELD %.*s", (int)field->length, field->name);

    tree_line(ctx, last, buffer);

    AST_PrintContext child = tree_child(ctx, last);

    bool hasReadAs = field->readAs != NULL;

    tree_line(&child, false, "offset:");

    AST_PrintContext offsetCtx = tree_child(&child, false);
    expression_print(field->offset, &offsetCtx, true);

    tree_line(&child, !hasReadAs, "size:");

    AST_PrintContext sizeCtx = tree_child(&child, !hasReadAs);
    expression_print(field->size, &sizeCtx, true);

    if (hasReadAs) {
        tree_line(&child, true, "readAs:");

        AST_PrintContext readAsCtx = tree_child(&child, true);

        expression_print(field->readAs, &readAsCtx, true);
    }
}

static void expression_print(AST_Expression *expression,AST_PrintContext *ctx, bool last) {
    if (!expression) {
        tree_line(ctx, last, "<null>");
        return;
    }

    char buffer[AST_PRINT_MAX_DEPTH];

    switch (expression->type) {
        case AST_EX_LITERAL:
            literal_print(&expression->literal, ctx, last );
            break;

        case AST_EX_LOCATION:
            location_print(&expression->location, ctx, last);
            break;

        case AST_EX_VARIABLE:
            snprintf(buffer, sizeof(buffer), "VARIABLE %.*s", (int)expression->variable.length, expression->variable.name);

            tree_line(ctx, last, buffer);
            break;

        case AST_EX_DATA_TYPE:
            snprintf(buffer, sizeof(buffer), "DATA_TYPE %s", data_type_name(expression->dataType));

            tree_line(ctx, last, buffer);
            break;

        case AST_EX_SP:
            tree_line(ctx, last, "SP");
            break;

        case AST_EX_UNARY: {
            snprintf(buffer, sizeof(buffer), "UNARY %s", operator_name(expression->unary.op));

            tree_line(ctx, last, buffer);

            AST_PrintContext child = tree_child(ctx, last);
            tree_line(&child, true, "operand:");

            AST_PrintContext operandCtx = tree_child(&child, true);
            expression_print(expression->unary.operand, &operandCtx, true);
        } break;

        case AST_EX_BINARY: {
            snprintf(buffer, sizeof(buffer), "BINARY %s", operator_name(expression->binary.op));

            tree_line(ctx, last, buffer);

            AST_PrintContext child = tree_child(ctx, last);
            tree_line(&child, false, "left:");

            AST_PrintContext leftCtx = tree_child(&child, false);
            expression_print(expression->binary.left, &leftCtx, true);

            tree_line(&child, true, "right:");

            AST_PrintContext rightCtx = tree_child(&child, true);
            expression_print(expression->binary.right, &rightCtx, true);
        } break;

        case AST_EX_CALL: {
            tree_line(ctx, last, "CALL");

            AST_PrintContext child = tree_child(ctx, last);
            tree_line(&child, expression->call.argumentCount == 0, "function:");

            AST_PrintContext functionCtx = tree_child(&child, expression->call.argumentCount == 0);
            expression_print(expression->call.function, &functionCtx, true);

            for (size_t i = 0; i < expression->call.argumentCount; i++) {
                bool arg_last = i == expression->call.argumentCount - 1;

                snprintf(buffer, sizeof(buffer), "argument[%zu]:", i);

                tree_line(&child, arg_last, buffer);

                AST_PrintContext argCtx = tree_child(&child, arg_last);
                expression_print(expression->call.arguments[i], &argCtx, true);
            }
        } break;

        case AST_EX_MEMBER_ACCESS: {
            snprintf(buffer, sizeof(buffer), "MEMBER_ACCESS %.*s", (int)expression->memberAccess.length, 
                expression->memberAccess.name);

            tree_line(ctx, last, buffer);

            AST_PrintContext child = tree_child(ctx, last);
            tree_line(&child, true, "parent:");

            AST_PrintContext parentCtx = tree_child(&child, true);
            expression_print(expression->memberAccess.parent, &parentCtx, true);
        } break;

        case AST_EX_LOCATION_ACCESS: {
            tree_line(ctx, last, "LOCATION_ACCESS");

            AST_PrintContext child = tree_child(ctx, last);

            bool hasReadAs = expression->locationAccess.readAs != NULL;

            tree_line(&child, false, "parent:");

            AST_PrintContext parentCtx = tree_child(&child, false);
            expression_print(expression->locationAccess.parent, &parentCtx, true);

            tree_line(&child, false, "offset:");

            AST_PrintContext offsetCtx = tree_child(&child, false);
            expression_print(expression->locationAccess.offset, &offsetCtx, true);

            tree_line(&child, !hasReadAs, "size:");

            AST_PrintContext sizeCtx = tree_child(&child, !hasReadAs);
            expression_print(expression->locationAccess.size, &sizeCtx, true);

            if (hasReadAs) {
                tree_line(&child, true, "readAs:");

                AST_PrintContext readAsCtx = tree_child(&child, true);
                expression_print(expression->locationAccess.readAs, &readAsCtx, true);
            }
        } break;
    }
}

static void block_print(AST_Block *block, AST_PrintContext *ctx, bool last) {
    tree_line(ctx, last, "BLOCK");

    if (!block) return;

    AST_PrintContext child = tree_child(ctx, last);

    for (size_t i = 0; i < block->count; i++) {
        bool statement_last = i == block->count - 1;

        node_print(block->statements[i], &child, statement_last);
    }
}

static void node_print(AST_Node *node, AST_PrintContext *ctx, bool last) {
    if (!node) {
        tree_line(ctx, last, "<null node>");
        return;
    }

    char buffer[AST_PRINT_MAX_DEPTH];

    switch (node->type) {
        case AST_PROGRAM: {
            tree_line(ctx, last, "PROGRAM");

            AST_PrintContext child = tree_child(ctx, last);

            for (size_t i = 0; i < node->program.count; i++) {
                bool statement_last = i == node->program.count - 1;

                node_print(node->program.statements[i], &child, statement_last);
            }
        } break;

        case AST_FUNCTION_DECLARATION: {
            AST_FunctionDeclaration *fn = &node->functionDeclaration;
            snprintf(buffer, sizeof(buffer), "FUNCTION %.*s", (int)fn->length, fn->name);

            tree_line(ctx, last, buffer);

            AST_PrintContext child = tree_child(ctx, last);

            bool hasBody = fn->body != NULL;

            tree_line(&child, false, "return_size:");

            AST_PrintContext returnCtx = tree_child(&child, false);
            expression_print(fn->returnSize, &returnCtx, true);

            tree_line(&child, !hasBody, "parameters");

            AST_PrintContext parameterCtx = tree_child(&child, !hasBody);

            for (size_t i = 0; i < fn->parameterCount; i++) {
                bool parameter_last = i == fn->parameterCount - 1;

                field_print(&fn->parameters[i], &parameterCtx, parameter_last);
            }

            if (hasBody) block_print(fn->body, &child, true);
        } break;

        case AST_VARIABLE_DECLARATION: {
            AST_VariableDeclaration *decl = &node->variableDeclaration;
            snprintf(buffer, sizeof(buffer), "VARIABLE_DECLARATION %.*s", (int)decl->var.length, decl->var.name);

            tree_line(ctx, last, buffer);

            AST_PrintContext child = tree_child(ctx, last);

            bool hasInitializer = decl->initializer != NULL;

            location_print(&decl->location, &child, !hasInitializer);

            if (hasInitializer) {
                tree_line(&child, true, "initializer:");

                AST_PrintContext initializerCtx = tree_child(&child, true);
                expression_print(decl->initializer, &initializerCtx, true);
            }
        } break;

        case AST_STRUCT_DECLARATION: {
            AST_StructDeclaration *structure = &node->structDeclaration;
            snprintf(buffer, sizeof(buffer), "STRUCT %.*s", (int)structure->length, structure->name);

            tree_line(ctx, last, buffer);

            AST_PrintContext child = tree_child(ctx, last);

            for (size_t i = 0; i < structure->fieldCount; i++) {
                bool field_last = i == structure->fieldCount - 1;

                field_print(&structure->fields[i], &child, field_last);
            }
        } break;

        case AST_ASSIGNMENT: {
            tree_line(ctx, last, "ASSIGNMENT");

            AST_PrintContext child = tree_child(ctx, last);
            tree_line(&child, false, "target:");

            AST_PrintContext targetCtx = tree_child(&child, false);
            expression_print(node->assignment.target, &targetCtx, true );

            tree_line(&child, true, "value:");

            AST_PrintContext valueCtx = tree_child(&child, true);
            expression_print(node->assignment.value, &valueCtx, true);
        } break;

        case AST_EXPRESSION_STATEMENT: {
            tree_line(ctx, last, "EXPRESSION_STATEMENT");

            AST_PrintContext child = tree_child(ctx, last);
            tree_line(&child, true, "expression:");

            AST_PrintContext expressionCtx = tree_child(&child, true);
            expression_print(node->expressionStatement.expression, &expressionCtx, true);
        } break;

        case AST_SP_LOCATION: {
            AST_SPLocation *sp = &node->spLocation;

            bool hasInitializer = sp->initializer != NULL;

            tree_line(ctx, last, "SP_LOCATION");

            AST_PrintContext child = tree_child(ctx, last);
            tree_line(&child, !hasInitializer, "location:");

            AST_PrintContext locationCtx = tree_child(&child, !hasInitializer);
            location_print(&sp->location, &locationCtx, true);

            if (hasInitializer) {
                tree_line(&child, true, "initializer:");

                AST_PrintContext initializerCtx = tree_child(&child, true);
                expression_print(sp->initializer, &initializerCtx, true);
            }
        } break;

        case AST_IF: {
            AST_Conditional *conditional = &node->conditional;

            bool hasElse = conditional->elseBody != NULL;

            tree_line(ctx, last, "IF");

            AST_PrintContext child = tree_child(ctx, last);
            tree_line(&child, false, "condition:");

            AST_PrintContext conditionCtx = tree_child(&child, false);
            expression_print(conditional->condition, &conditionCtx, true);

            tree_line(&child, !hasElse, "then:");

            AST_PrintContext thenCtx = tree_child(&child, !hasElse);

            if (conditional->thenBody) {
                for (size_t i = 0; i < conditional->thenBody->count; i++) {
                    bool statement_last = i == conditional->thenBody->count - 1;

                    node_print( conditional->thenBody->statements[i], &thenCtx, statement_last);
                }
            }

            if (hasElse) {
                tree_line(&child, true, "else:");

                AST_PrintContext elseCtx = tree_child(&child, true);

                for (size_t i = 0; i < conditional->elseBody->count; i++) {
                    bool statement_last = i == conditional->elseBody->count - 1;

                    node_print(conditional->elseBody->statements[i], &elseCtx, statement_last);
                }
            }
        } break;

        case AST_WHILE: {
            AST_Loop *loop = &node->loop;

            tree_line(ctx, last, "WHILE");

            AST_PrintContext child = tree_child(ctx, last);
            tree_line(&child, false, "condition:");

            AST_PrintContext conditionCtx = tree_child(&child, false);
            expression_print(loop->condition, &conditionCtx, true);

            tree_line(&child, true, "body:");

            AST_PrintContext bodyCtx = tree_child(&child, true);

            if (loop->body) {
                for (size_t i = 0; i < loop->body->count; i++) {
                    bool statement_last = i == loop->body->count - 1;

                    node_print(loop->body->statements[i], &bodyCtx, statement_last);
                }
            }
        } break;

        case AST_RETURN: {
            bool hasValue = node->returnStatement.value != NULL;

            tree_line(ctx, last, "RETURN");

            if (hasValue) {
                AST_PrintContext child = tree_child(ctx, last);
                tree_line(&child, true, "value:");

                AST_PrintContext valueCtx = tree_child(&child, true);
                expression_print(node->returnStatement.value, &valueCtx, true);
            }
        } break;

        case AST_BREAK:
            tree_line(ctx, last, "BREAK");
            break;

        case AST_CONTINUE:
            tree_line(ctx, last, "CONTINUE");
            break;

        case AST_END:
            tree_line(ctx, last, "END");
            break;
    }
}

void program_print(AST_Program *program) {
    if (!program) return;

    printf("PROGRAM\n");

    AST_PrintContext ctx = {
        .branch = {0},
        .depth = 0
    };

    for (size_t i = 0; i < program->count; i++) {
        bool last = i == program->count - 1;

        node_print(program->statements[i], &ctx, last);
    }
}