#include "src/lexer.h"
#include "src/parser.h"
#include "src/semantic_analyser.h"
#include "src/ir.h"
#include "src/codegen.h"


int main(void) {
    const char *source = "let elma. elma = 1 * 2 + 3 * 4.";

    Lexer lexer = lexer_init(source);
    lexer_print(&lexer);

    ASTNode program = parse_program(&lexer);
    ast_print(&program);

    if (!semantic_analyse(&program)) {
        printf("SEMANTIC ERROR\n");
        return 1;
    }

    IR ir = ir_generate(&program);
    ir_print(&ir);

    if (!codegen_generate(&ir, "./output.s")) {
        printf("CODEGEN ERROR\n");
        return 1;
    }

    return 0;
}