#include "src/lexer.h"
#include "src/parser.h"
#include "src/ir.h"


int main(void) {
    const char *source = "let elma. elma = 1 * 2 + 3 * 4.";

    Lexer lexer = lexer_init(source);
    lexer_print(&lexer);

    ASTNode program = parse_program(&lexer);
    ast_print(&program);

    IR ir = ir_generate(&program);

    ir_print(&ir);
    return 0;
}