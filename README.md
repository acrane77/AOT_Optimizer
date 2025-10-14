# S-C AOT Optimizer
## An ahead-of-time optimizer for a simplified subset of C, S-C (Simple C).

## Overview
The S-C AOT Optimizer reads S-C source code, lexes and parses it into an abstract syntax tree (AST), and then applies a series of classical compiler optimizations before execution. 

Performs optimizations such as:
- Constant folding
- Dead code elimination
- Loop Optimizations (invariant motion, unrolling, strength reduction)
- Function inlining
- Induction Variable Elimination

## S-C Language
The S-C lexer currently supports:
- Primitive types: `int`, `float`, `char`, `str`, `bool`
- Operators:
  - Bitwise: `&`, `|`, `^`, `<<`, `>>` and more
  - Arithmetic: `+`, `-`, `*`, `/`, `%`
  - Boolean: `&&`, `||`, `!`, `==`, `!=` and more
  - Control Flow: `if`, `else`, `while`, `for`, `break`, `continue`
  - Functions: Simple (non-recursive) declarations, definitions and calls
  - Arrays: Multi-dimensional, with limited pointer semantics
  - Literals: integer, float, char, str and bool literals

These features are lexed into tokens by the **S-C Lexer** (`sc_lexer.c`) and represented as a buffer of Token structures.

## Lexer (Tokenization)

The lexer (`sc_lexer.c`) reads the entire source file and tokenizes it into a dynamic buffer.

- Recognizes keywords, operators, identifiers, delimiters and literals.
- Handles nested brackets in arrays and functions.
- Ignores comments
- Allocates and owns the source and token buffer.

Each token is represented by:

```
struct Token {
  int val;
  enum TokenType type;
  char* lexeme;
  int line, col;
  int length;
}
```

## Parser

The parser will consume tokens from the lexer, build the AST, and apply multiple compile-time optimizations.

### Function Inlining
Replaces function calls with the function's body:
```
int add(int a, int b) { return a + b; }
int main() { int x = add(5, 6); }
// becomes:
int main() { int x = 5 + 6; }
```

### Constand Folding
Evaluates and replaces constant expressions:
```
int x = 5 + 4 * 5; /* becomes */ int x = 25;
```

### Dead Code Elimination
Removes unused assignments or statements
```
int y = 5;
int z = 6;
int x = y + 2; // z is never used, remove it.
```

### Loop Optimizations
- Loop-Invariant Code: move static (unchanging) code outside of the loop.
- Strength Reduction: Replace expensive operations (`*`, `/`) with cheaper ones (`+`, `-`)
- Induction Variable Elimination: Simplify dependant loop variables.
- Loop Unrolling: Expand loop bodies to reduce iteration overhead.

### Optimization Order
1. Function Inlining
2. Constant Folding
3. Dead Code Elimination
4. Loop Invariant Code Motion
5. Loop Strength Reduction
6. Induction Variable Elimination
7. Loop Unrolling

## Building & Running
### Requirements
- C99 or later compiler
- Standard C Library

### Build
`gcc -o sc_opt sc_lexer.c sc_parser.c`
### Run
`./sc_opt`

You will then be prompted for the path to your C source file, the lexer will then tokenize the file and prepare it for parsing and optimization.

## Structure
```
.
├── sc_lexer.c      Tokenizer for S-C source
├── sc_parser.c     Parser and optimizer (WIP)
├── sc_token.h      Shared token data structures
└── README.md       This file
```

## Status
Lexer: Functional                                                                    
Parser: In progress                                                                                            
Optimizations: Not yet implemented
