#pragma once
#include "lexer.h"
#include "ast.h"

ASTProgram parse_program(Lexer *lexer);