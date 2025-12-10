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
#include <string.h>
void parseBlock(struct Parser* ps);

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
    return 0;
}

// Peek at current token
struct Token* current(struct Parser* parser) {
    return &parser->tokens[parser->pos];
}

// Consumes, advances and returns token
void advance(struct Parser* parser) {
    parser->pos++;
}

// Peeks at next token in line
struct Token* peekNext(struct Parser* parser) {
    if (parser->pos + 1 >= parser->count) 
        return &parser->tokens[parser->count - 1];
    return &parser->tokens[parser->pos + 1];
}

void parseStatement(struct Parser* ps) {
    if (current(ps)->length == 1 && current(ps)->type == DELIMITER && *current(ps)->lexeme == '{') { parseBlock(ps); }
    else advance(ps);
}

void parseBlock(struct Parser* ps) {
    advance(ps);
    while(!isAtEnd(ps) && !(current(ps)->type == DELIMITER && current(ps)->length == 1 && current(ps)->lexeme[0] == '}')) {
        if (current(ps)->type == KEYWORD) { parseKeyword(ps); }
        else { parseStatement(ps); }
    }
    if (isAtEnd(ps)) {
        ps->errCount++; // TODO error, unterminated }
    }
    else advance(ps);
}

void parseExpression(struct Parser* ps) {
    while(!isAtEnd(ps) && !(current(ps)->type == DELIMITER && current(ps)->length == 1 && (current(ps)->lexeme[0] == ';' || current(ps)->lexeme[0] == ')'))) {
        enum TokenType type = current(ps)->type;
        if (type == IDENTIFIER || type == INT_LITERAL || type == STR_LITERAL || type == BOOL_LITERAL || type == CHAR_LITERAL || type == FLOAT_LITERAL) {
            advance(ps); // TODO
        }
        else if (type == OPERATOR) {
            advance(ps); // TODO
        }
        else break;
    }
    advance(ps); // Consume ; TODO should this be done?
}

void parseFunction(struct Parser* ps) {
    advance(ps);
    if (current(ps)->length == 1 && current(ps)->type == DELIMITER && *current(ps)->lexeme == '{') { parseBlock(ps); } // definition
    else if (current(ps)->length == 1 && current(ps)->type == DELIMITER && *current(ps)->lexeme == ';') { advance(ps); } // declaration
    else { ps->errCount++; } // TODO
}

void parseVar(struct Parser* ps) {
    advance(ps);
    if (current(ps)->length == 1 && current(ps)->type == DELIMITER && *current(ps)->lexeme == ';') { advance(ps); } // Declaration
    else if (current(ps)->length == 1 && current(ps)->type == OPERATOR && *current(ps)->lexeme == '=') { parseExpression(ps); } // Definition
    else if (true) {} // TODO: arrays or other declarations/definitions
}

void parseKeyword(struct Parser* ps) {
    int length = current(ps)->length;

    if (length == 6 && !strncmp(current(ps)->lexeme, "return", 6)) {
        advance(ps); 
        if (current(ps)->length == 1 && *current(ps)->lexeme == ';') advance(ps); // return;
        else parseExpression(ps); // return something;
        return; // TODO maybe use switch here instead for continue/while/if ?
    }
    else if (length == 2 && !strncmp(current(ps)->lexeme, "if", 2)) {
        advance(ps); // TODO parse IF block
        return;
    }
    else if (length == 4 && !strncmp(current(ps)->lexeme, "else", 4)) {
        advance(ps); 
        if (current(ps)->lexeme == 2 && !strncmp(current(ps)->lexeme, "if", 2)) {
            // TODO Parse ELSE IF
        }
        // TODO parse ELSE block
        return;
    }
    else if (length == 5 && !strncmp(current(ps)->lexeme, "while", 4)) {
        // TODO parse WHILE block
        return;
    }
    else if (length == 3 && !strncmp(current(ps)->lexeme, "for", 3)) {
        // TODO parse FOR block
        return;
    }
    else if (length == 5 && !strncmp(current(ps)->lexeme, "break", 5)) {
        // TODO parse break block
        return;
    }
    else if (length == 8 && !strncmp(current(ps)->lexeme, "continue", 8)) {
        // TODO parse continue block
        return;
    }
    
    if (current(ps)->type == FUNCTION) { parseFunction(ps); } // ( -> function
    else if (current(ps)->type == IDENTIFIER) { parseVar(ps); } // identifier -> variable declaration/definition
    else {
        ps->errCount++;
        // TODO: Report Error
    }
}

void parseProgram(struct Parser* ps) {  
    while (!isAtEnd(ps)) {
        if (current(ps)->type == KEYWORD) {  parseKeyword(ps); } // int, float, etc -> func/var declaration/definition
        else if (current(ps)->type == FUNCTION) { parseFunction(ps); } // function -> function call
        else advance(ps); // dummy call, parseProgram should either call something or error.
    }
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

    while (ps.pos < ps.count) {
        print_token(&ps.tokens[ps.pos]);
        ps.pos++;
    }
    //parseProgram(&ps);

    free(tb.buf); // Free tokenbuffer buf and regular buf allocated in lexer (stored in .src and .buf)
    free(tb.src);
}