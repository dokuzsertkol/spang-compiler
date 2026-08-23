#include <stdio.h>

#include "src/compiler.h"
#include "src/lexer.h"
#include "src/parser.h"
#include "src/vm.h"


int main(void) {
    const char *source = "let elma. let armut. armut = 1. elma = armut. armut = 5.";

    Lexer lexer = lexer_init(source);

    ASTProgram program = parse_program(&lexer);

    Bytecode bytecode = compile(&program);

    VM vm = {0};

    vm_run(&vm, &bytecode);

    printf("elma = %d\n", vm.variables[0]);
    printf("armut = %d\n", vm.variables[1]);

    return 0;
}