cmake_minimum_required(VERSION 3.20)

project(spangc C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(spangc
    spangc/main.c
    spangc/token/token.c
    spangc/lexer/lexer.c
    spangc/ast/ast.c
    spangc/parser/parser.c
)

target_include_directories(spangc PRIVATE ${PROJECT_SOURCE_DIR}/spangc)