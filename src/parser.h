#pragma once
#include "lexer.h"
#include "ast.h"

ASTNode parse_program(Lexer *lexer);