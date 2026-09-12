# Spang

Spang is an experimental systems-oriented language built around one idea: memory locations should be explicit. Stack frames, offsets, widths, and interpretations are part of the language model instead of hidden implementation details.

The project motto is simple:

Spang, spank the stack!

## Philosophy

Spang favors control and explicit layout over protective abstractions. Its guiding ideas are:

- no hidden memory magic
- variables are named views over memory locations
- stack frames are visible and addressable
- layout costs should be visible to the programmer
- the programmer is trusted to understand the machine model

The language uses familiar punctuation for compactness, but its central abstraction is not a conventional object or value. It is a location.

## Language model

### Locations are the fundamental abstraction

Spang does not treat arrays, structs, pointers, and variables as fundamentally different memory concepts. They are different ways of describing or deriving locations.

A variable names a location:

```spang
value [fp + 0, i4];
```

A struct field derives a location from a named offset:

```spang
value.x;
```

An indexed region derives a location from an explicit offset:

```spang
value.[i * 4, i4];
```

The common primitive is a location described by a base, an offset, and a size.

### Locations use base, offset, and size

The canonical location form is:

```spang
var_name [base + offset, size];
```

The available bases are:

- `sp`: developer-controlled writable base
- `fp`: current function frame pointer
- `hp`: heap pointer
- `bp`: base pointer

The `offset` is an expression. The `size` is an expression that resolves to a byte width, or a datatype expression used as a read-as interpretation.

For example:

```spang
elma [fp + 0, 10 * i4];
```

Here, `10 * i4` can describe a region ten elements wide. A datatype can also be used directly:

```spang
elma [fp + 0, i4];
```

In this form, `i4` supplies the width and can also be retained as the location's `readAs` interpretation.

### Variables are aliases

A declaration creates a named alias for a memory region. It does not create a protected abstract value with automatic ownership or bounds checking.

The bytes at that location can be assigned or read through the alias. Their meaning depends on the width and interpretation used by the program.

### Datatypes describe layout and interpretation

Type information in Spang primarily describes memory layout and interpretation.

The scalar datatypes are:

```text
i1 i2 i4 i8
u1 u2 u4 u8
f4 f8
c1 c2 c4
b1
v0
```

Widths and interpretations should use one of the
explicit datatype names above.

A datatype expression can be used as a read-as marker when accessing a location:

```spang
value [fp + 0, i4];
```

The datatype does not imply ownership, bounds checking, or automatic memory safety. It describes how the accessed bytes should be interpreted. The programmer remains responsible for choosing a valid location and interpretation.

### Memory access expressions

Spang does not require arrays to be a built-in language abstraction.

A memory region can be accessed by creating another location relative to an existing one:

```spang
elma [fp + 0, 10 * i4];

elma.[0, i4] = 10;
elma.[i * 4, i4] = 20;
```

The syntax is:

```spang
parent.[offset, size]
```

It creates a new memory access expression. The `offset` is relative to the parent location, and the `size` describes the width of the accessed region. The optional datatype form also supplies a read-as interpretation.

For example, `elma.[i * 4, i4]` can access the `i`th four-byte element of a memory region. The compiler does not need to know that the original region is an array. It only needs to evaluate the parent location, offset, size, and interpretation.

### Named and explicit access

Struct-like data can be accessed through named members:

```spang
point.x = 10;
```

Arbitrary memory regions can be accessed through explicit location access:

```spang
buffer.[offset, size] = value;
```

Both forms operate on a parent expression. Named access uses a declared member, while explicit access uses a runtime offset and width. In the AST these are separate expression forms: member access and location access.

### sp, fp, hp, and bp

`sp` is the one writable base in the language: the developer can set it and use it as a controllable starting point for location expressions. Its name reflects its intended use in stack-oriented code, not a requirement that it always refer to the active runtime stack.

`fp` is the current function frame pointer and provides the frame anchor for function-relative layout. `hp` refers to the heap area, while `bp` is available as another base location. These bases are not interchangeable with `sp`: only `sp` is explicitly developer-controlled and writable.

For example:

```spang
fp foo (param1 [0, i4], param2 [4, i4]) [i4] {
    ...
}
```

These names describe the intended memory model. The exact operations supported by each base are still part of the compiler's ongoing development.

### Functions and control flow

Function declarations, conditionals, loops, calls, and assignments use compact, familiar syntax:

```spang
if (condition) {
    ...
} else {
    ...
}

while (condition) {
    ...
}
```

The syntax is deliberately compact; the language's identity comes from its location model rather than from its control-flow notation.

## Example syntax

### Variable declaration and assignment

```spang
var_name [base + offset, size];
var_name = expression;
```

### Direct location access

```spang
[fp + offset, size] = expression;
[hp + offset, size] = expression;
```

### Struct-like declaration

```spang
struct mystruct {
    x [offset, size],
    y [offset, size],
}
```

Usage can combine named and explicit access:

```spang
point [fp + 0, mystruct];

point.x = 10;
point.[4, i4] = 20;
```

This shows that struct fields and array-like regions are both derived from the same location model.

## Current project status

This project is active and still under development.

At the moment:

- the lexer is implemented
- AST-related parsing work is in progress
- the AST now represents datatype expressions, named member access, and explicit location access
- the rest of the compiler pipeline is older experimental code from previous learning work
- the project is not yet a complete, production-ready compiler

Some examples in this document describe the intended language model and may not yet be implemented by the current compiler. Syntax accepted by the parser and the future semantic rules are still being established.

## Repository structure

- `main.c`: entry point for the compiler
- `src/lexer.c` / `src/lexer.h`: lexical analysis
- `src/parser.c` / `src/parser.h`: parser work in progress
- `src/ast.c` / `src/ast.h`: AST structures and memory management
- `src/token.c` / `src/token.h`: token definitions and literal conversion

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
fp foo (p1 [0, i4], p2 [4, i4]) [i4] {
    [fp-30, i4] = [fp, i4] + p2;
    return [fp-30, i4];
}

fp main() [i4] {
    elma [fp, i4];
    armut [sp, i4];
    [hp-12, i4] = 5;
    kavun [hp-12, i4];
    [fp-48, i4] = foo(karpuz, [fp-40, i4]);
    return [fp-48, i4];
}
```

## Final note

Spang is intentionally not a “safe” language. It is a language for people who want to understand the machine instead of being separated from it.

It is for people who want to see the stack and work with it directly.

It is a language for explicit memory, forceful layout, and direct control.

And if that sounds a little dangerous, that is part of the point.
