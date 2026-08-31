#include <stdlib.h>
#include <stdio.h>
#include "src/lexer.h"
#include "src/parser.h"
#include "src/semantic_analyser.h"
#include "src/ir.h"
#include "src/codegen.h"


static const char* get_source() {
    FILE *input = fopen("./input/main.dk", "r");

    if (!input) {
        perror("fopen");
        return NULL;
    }

    fseek(input, 0, SEEK_END);
    long size = ftell(input);
    rewind(input);

    char *source = malloc(size + 1);
    if (!source) {
        fclose(input);
        return NULL;
    }

    fread(source, 1, size, input);
    source[size] = '\0';

    fclose(input);

    return source;
}

int main() {
    const char* source = get_source();
    if (source == NULL) return 1;

    Lexer lexer = lexer_init(source);
    lexer_print(&lexer);

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