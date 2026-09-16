#include <string.h>
#include <stdio.h>
#include "lexer/lexer_print.h"
#include "ast/ast_print.h"
#include "parser/parser.h"
#include "parser/parser_error.h"

typedef struct {
    bool printTokens;
    bool printProgram;
} CompilerOptions;

static void print_help(const char *programName) {
    printf("Usage: %s <file.spg> [options]\n\n"
            "Options:\n"
            "  -h, --help          Show this help message\n"
            "  -t, --tokens        Print lexer tokens\n"
            "  -p, --program       Print parsed AST program\n"
            "  -a, --all           Enable all debug output\n\n",
        programName
    );
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_help(argv[0]);
        return 1;
    }

    CompilerOptions options = {0};

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]);
            return 0;
        }

        if (i == 1) continue;

        if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--tokens") == 0) {
            options.printTokens = true;
        }
        else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--program") == 0) {
            options.printProgram = true;
        }
        else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0) {
            options.printTokens = true;
            options.printProgram = true;
        }
        else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 1;
        }
    }

    Lexer *lexer = lexer_init(argv[1]);
    if (!lexer) return 1;
    if (options.printTokens) lexer_print(lexer);

    Parser *parser = parser_init(lexer);
    if (!parser) {
        lexer_free(lexer);
        return 0;
    }

    AST_Program *program = parse_program(parser);
    if (options.printProgram && program) program_print(program);
    parser_print_error(parser);

    ast_program_free(program);
    parser_free(parser);
    lexer_free(lexer);

    return parser->hasError ? 1 : 0;

    /*if (!semantic_analyse(&program)) {
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