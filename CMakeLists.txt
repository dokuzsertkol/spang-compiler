cmake_minimum_required(VERSION 3.20)

project(dokuz C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(dokuz
    main.c
    src/lexer.c
    src/parser.c
    src/compiler.c
    src/vm.c
)

target_include_directories(dokuz PRIVATE src)