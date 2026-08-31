cmake_minimum_required(VERSION 3.20)

project(spangc C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(spangc
    main.c
    src/lexer.c
)

target_include_directories(spangc PRIVATE src)