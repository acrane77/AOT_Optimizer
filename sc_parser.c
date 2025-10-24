/* Ownership Rules:
 * Lexer allocates source buffer and the token array
 * Parser consumes both and frees them when done.
 * Individual token .lexeme pointers point into the source buffer.
*/

/* Constant Folding: 
 * Form of optimization where an expression can be evaluated by the compiler at compile time, instead of generating code to evaluate it at runtime.
 * ex: 5 + 4 * 5; is the same as x=25; so we can let the compiler evaluate it and just output the ASM for x = 25;
 * To implement, look for sub-trees in an AST tree where the leaves are integer literals. If there is a binary operation which has two integer literals leaves,
 * The compiler can evaluate the expression and replace the sub tree with a single integer literal node.
 * If there is a unary operation with an integer literal leaf child, then the compiler can also evaluate the expression and replace the sub tree with a single integer literal node.
 * Once we can do this for sub-trees, we can write a function to traverse the AST which looks for sub-trees to fold, the algorithm follows as:
 * 1. Try to fold and replace the left child, recursively
 * 2. Try to fold and replace the right child, recursively
 * If its a binary operation with two literals child leaves, fold that.
 * If its a unary operation with one literal child leaf, fold that.
*/

/* Dead-Code:
 * Code that is never executed or a variable that is never used.
 * Other optimizations may create dead code, so this should run last.
 * ex: y = 5; z = 6; x = y + 2; (z is never used, so it can be removed)
*/

/* Loop Optimizations
 * Loop-Invariant Code Motion: If result of a statement or expression does not change inside of a loop, and it has no externally visible side effects (!), you can hoist its 
 * computation outside of the loop.
 * ex: for (i=0; i<100; i++) { x = y; } can be optimized to x = y; for (i=0; i<100; i++) { }
 * Loop Strength Reduction: Replaces an expensive operation (multiply, divide) by cheap ones (add, subtract)
 * ex: for (i=0; i<100; i++) { x = i * 8; } can be optimized to: x = 0; for (i=0; i<100; i++) { x += 8; }
 * Induction Variable Elimination:
 * Loop Unrolling: Execute loop body multiple times per iteration, reducing overhead. Space tradeoff, program size increases.
 * ex: for (i=0;i<n;i++) { S } unrolled 4 times: for (i=0;i<n-3;i+=4) { S; S; S; S; } for (;i<n;i++) { S; }
*/

/* Function Inlining:
 * Replace a function call with the body of the function:
 * ex: int add(int a, int b) { return a + b; } int main() { int x = add(5, 6); } can be optimized to: int main() { int x = 5 + 6; }
 * Reduces function call overhead, enables further optimizations (constant folding, dead code elimination)
*/

/* Optimization Order:
 * 1. Function Inlining
 * 2. Constant Folding
 * 3. Dead Code Elimination
 * 4. Loop Invariant Code Motion
 * 5. Loop Strength Reduction
 * 6. Induction Variable Elimination
 * 7. Loop Unrolling
*/

#include "sc_token.h"
#include <stdlib.h>

// ChatGPT functions to print tokens (for testing)
const char* token_type_name(enum TokenType type) {
    switch (type) {
        case INT_LITERAL:    return "INT_LITERAL";
        case FLOAT_LITERAL:  return "FLOAT_LITERAL";
        case CHAR_LITERAL:   return "CHAR_LITERAL";
        case STR_LITERAL:    return "STR_LITERAL";
        case BOOL_LITERAL:   return "BOOL_LITERAL";
        case IDENTIFIER:     return "IDENTIFIER";
        case FUNCTION:       return "FUNCTION";
        case ARRAY:          return "ARRAY";
        case KEYWORD:        return "KEYWORD";
        case OPERATOR:       return "OPERATOR";
        case DELIMITER:      return "DELIMITER";
        case EMPTY:          return "EMPTY";
        case END_OF_FILE:    return "END_OF_FILE";
        default:             return "UNKNOWN";
    }
}

void print_token(const struct Token* t) {
    printf("Token {\n");
    printf("  type: %s\n", token_type_name(t->type));
    printf("  lexeme: \"%.*s\"\n", t->length, t->lexeme);
    printf("  val: %f\n", t->val);
    printf("  line: %d, col: %d\n", t->line, t->col);
    printf("  length: %d\n", t->length);
    printf("}\n");
}

// Check if currently at end of token array
int isAtEnd(struct Parser* parser) {
    if (parser->pos >= parser->count) return 1;
    else if (parser->tokens[parser->pos].type == END_OF_FILE) return 1;
    return 0;
}

// Peek at current token
struct Token* current(struct Parser* parser) {
    return &parser->tokens[parser->pos];
}

// Consumes, advances and returns token
struct Token* advance(struct Parser* parser) {
    parser->pos++;
    return &parser->tokens[parser->pos - 1];
}

// Peeks at next token in line
struct Token* peekNext(struct Parser* parser) {
    if (parser->pos + 1 >= parser->count) 
        return &parser->tokens[parser->count - 1];
    return &parser->tokens[parser->pos + 1];
}

// Main parser function
int main() {
    char fileName[1024];
    printf("Entire path to input file: \n");
    scanf("%1024s", fileName); // Take file path

    struct TokenBuffer tb = lexFile(fileName); // Call lexer and tokenize file
    struct Parser ps;

    if (tb.buf == NULL || tb.src == NULL) { // Ensure no memory errors
        printf("Memory error detected, Exiting...");
        if (tb.src) free(tb.src); // src allocates before buf, so if src succeeds but buf fails, we must free src.
        return -1;
    }

    ps.errCount = 0; ps.pos = 0;
    ps.src = tb.src;
    ps.tokens = tb.buf;
    ps.count = tb.count;

    while (!isAtEnd(&ps)) {
        print_token(current(&ps));
        advance(&ps);
    }

    free(tb.buf); // Free tokenbuffer buf and regular buf allocated in lexer (stored in .src and .buf)
    free(tb.src);
}