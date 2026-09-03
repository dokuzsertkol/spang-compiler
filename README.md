# Spang

Spang is a experimental systems-oriented programming language built around one idea: the developer should see the stack, the frame, and the memory layout directly instead of hiding it behind abstraction layers.

The language is intentionally blunt. It does not pretend that memory is safe, clean, or friendly. It exposes pointers, offsets, and sizes as first-class concepts and treats them as the natural shape of computation.

The project motto is simple:

Spang, spank the stack!

## Philosophy

Spang is my personal answer to the question: what if a language removed most of the protective fiction of modern high-level languages and forced the developer to work with memory as truth instead of as a hidden implementation detail?

In my view, Spang is not a language for comfort. It is a language for control.

The guiding ideas are:

- no hidden memory magic
- no deep abstraction layers between the programmer and the runtime
- variables are aliases to memory regions, not abstract typed values
- stack frames are visible and addressable
- the compiler should not hide the cost of layout
- the developer is trusted to understand the machine model

This is not a language that tries to make everything safe. It is a language that makes everything explicit.

## Personal definition of Spang

I think of Spang as a stack-first, pointer-first, low-level language with C-like syntax but a much more raw memory model.

It is a language where:

- a variable is a named view over a location in memory
- data is described by base, offset, and size
- type information is mostly about byte layout and interpretation, not about enforced safety
- memory access is driven by expressions that resolve to addresses and widths
- stack and frame boundaries are visible to the programmer
- the runtime remains honest about how memory is allocated and addressed

So Spang is not “C with a different name.” It is more like C stripped down to its raw mechanics, then made more explicit and less forgiving.

## Core language model

### 1. Memory is addressed by base + offset + size

The core rule is that every variable or memory slot is effectively described as:

- a base: sp, fp, hp, or another addressable base
- an offset: an expression
- a size: an expression or type-like size marker

The canonical form is:

```
var_name [base+offset, size];
```

The base can be:

- sp: stack pointer
- fp: function pointer / current frame pointer
- hp: heap pointer
- bp: if used by the project later, it is treated as another base reference

The important point is that every memory location is a view into a byte region, and that region can be assigned to or read from directly.

### 2. Variables are aliases, not magical values

A declaration such as:

```
value [sp+8, int];
```

does not create a protected typed variable in the abstract sense. It creates a named alias for a memory region whose size is described by the declared size.

The value is whatever bytes exist at that address, interpreted according to the size and whatever rules the code chooses to apply.

### 3. No type safety by default

Spang does not aim for strict type safety.

In this language:

- types describe layout, size, and parsing conventions
- they are not a rigid protection barrier
- a value may be treated as an integer, a char, a float, or a raw byte sequence depending on how the program chooses to interpret it
- the developer is responsible for the interpretation of memory

That is a deliberate design choice. The compiler may help with parsing and size handling, but it is not there to protect the programmer from their own assumptions.

### 4. Assignments are memory operations

Assignments can target:

- a named variable alias
- an explicit memory address
- a base + offset + size region

Examples:

```
var_name = expression;
[base+offset, size] = expression;
```

This means memory can be assigned directly and the target is just another addressable region.

### 5. sp is a working pointer into the active stack frame

The stack pointer is a mutable, developer-facing pointer.

It can be used as a pointer to a memory address or as a base for named aliases.

Examples:

```
sp [base+offset, size];
sp var_name;
sp base+offset;
```

The idea is that sp acts like a waypoint into the current execution stack. It can be used to access live stack memory while the frame lives.

This is useful for walking around the frame, aliasing parts of it, and storing data in a known relative address space.

### 6. fp is the current function frame pointer

fp represents the current function frame. It is the frame anchor that gives the language a coherent stack layout inside a function.

It is treated as a read-mostly frame reference, and it is only meaningful within the active function scope. When the scope changes, fp changes with it.

Typical usage:

```
fp foo {
    param1 [fp+4, int];
    param2 [fp+4, 4];
}
```

This is how function parameters and local storage are conceptually mapped to frame-relative memory.

### 7. hp is the heap pointer and remains global across runtime

hp is the heap base pointer and is intentionally global in the sense that it persists throughout the runtime and is accessible from anywhere the language allows access.

This means the heap is treated as a global memory arena rather than a hidden malloc-backed abstraction.

Example:

```
[hp+offset, size] = expression;
```

This is intentionally direct and runtime-visible.

### 8. No inner scoping model

This language intentionally avoids the decorative safety net of nested block scoping. The model is simple: there is no elaborate inner block isolation story.

This keeps scope behavior closer to primitive frame semantics and makes the stack layout easier to reason about.

### 9. Syntax is C-like, but the semantics are lower level

The language borrows C-style syntax because it is familiar and compact, but the semantics are intentionally more memory-oriented than classic C.

Example:

```
if (condition) {
    ...
} else if (other_condition) {
    ...
} else {
    ...
}

while (condition) {
    ...
}
```

The control flow forms are meant to feel C-like, while memory layout is still the central concern.

## Example syntax

### Variable declaration

```
var_name [base+offset, size];
```

The offset and size are expressions. The base is either sp, fp, or hp.

### Assignment

```
var_name = expression;
[base+offset, size] = expression;
```

### Set pointer

```
sp [base+offset, size];
sp var_name;
sp base+offset;
```

### Heap access

```
[hp+offset, size] = expression;
```

### Function declaration

```
fp foo {
    param1 [fp+4, int];
    param2 [fp+4, 4];

    // locals are also frame-relative
    // fp-offset, size for local memory
}
```

### Struct-like declaration

```
struct mystruct {
    x [offset, size];
    y [offset, size];
}
```

Then usage can look like:

```
var_name [base+offset, mystruct];
var_name.x = expression;
```

This is a direct field-style access to a memory layout rather than a traditional high-level object model.

## Current project status

This project is active and still under development.

At the moment:

- the lexer is implemented
- AST-related parsing work is in progress
- the rest of the compiler pipeline is older experimental code from previous learning work
- the project is not yet a complete, production-ready compiler

The current focus is the parser and AST layer, which is the foundation needed before semantic analysis, IR generation, and code generation can be completed.

## Repository structure

- main.c: entry point for the compiler
- src/lexer.c / src/lexer.h: lexical analysis
- src/parser.c / src/parser.h: parser work in progress
- src/ast.h: AST definitions and memory-related language model
- src/semantic_analyser.c / src/semantic_analyser.h: future analysis layer
- src/ir.c / src/ir.h: intermediate representation work
- src/codegen.c / src/codegen.h: code generation work
- input/: sample language input files
- output/: generated output artifacts

## Build and run

This project uses CMake.

From the project root:

```
cmake -S . -B build
cmake --build build
```

Then run:

```
./build/spangc
```

## Example input

A sample language file in the project looks like this:

```
while (elma == 1) {
    elma = 1;
    elma = 5;
}
```

And another sample uses pointer-style local and global memory layout:

```
int foo(int, int elma) {
    [fp-30, int] = [fp, int] + 4 + elma + kavun;
    return [fp-30, int];
}

int main() {
    elma [fp, int];
    armut [sp, int];
    [hp-12, int] = 5;
    kavun [hp-12, int];
    [fp-48, int] = foo(karpuz, [fp-40, int]);
    return [fp-48, char];
}
```

## Final note

Spang is intentionally not a “safe” language. It is a language for people who want to understand the machine instead of being separated from it.

It is for people who want to see the stack and work with it directly.

It is a language for explicit memory, forceful layout, and direct control.

And if that sounds a little dangerous, that is part of the point.
