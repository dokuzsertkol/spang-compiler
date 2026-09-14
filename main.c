#include "src/ast.h"
#include "src/lexer.h"
#include "src/parser.h"

int main() {
    Lexer lexer;
    if(!lexer_init(&lexer, "./input/main.spg")) return 0;
    // lexer_print(&lexer);

    Parser parser = parser_init(&lexer);
    AST_Program *program = parse_program(&parser);
    parser_print_error(&parser);
    program_print(program);

    lexer_free(&lexer);
    ast_program_free(program);

    /* ASTNode program = parse_program(&lexer);
    ast_print(&program);

    if (!semantic_analyse(&program)) {
        printf("SEMANTIC ERROR\n");
        return 1;
    }

    IR ir = ir_generate(&program);
    ir_print(&ir);

    if (!codegen_generate(&ir, "./output/main.s")) {
        printf("CODEGEN ERROR\n");
        return 1;
    }

    system("as ./output/main.s -o ./output/main.o");
    system("ld ./output/main.o -o ./output/main");  
    system("rm -f ./output/main.s");
    system("rm -f ./output/main.o"); */

    return 0;
}