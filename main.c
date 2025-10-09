/* 
 * S-C (simple C) Optimizer Supports:
 * Basic scalar types (int, float, char, str, bool)
 * Bitwise operations (&, |, ^, ~, <<, >>)
 * Arithmetic operations (+, -, *, /, %)
 * Boolean operations (&&, ||, !, ==, !=, <, >, <=, >=)
 * Control flow (if, else, while, for (simple bounds), break, continue)
 * Simple (non-recursive) Functions (declaration, definition, calls)
 * Basic arrays and very restrictive pointers
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static const char* validTripleOps[] = { "<<=", ">>=" }; int validTripleOpsSize = sizeof(validTripleOps)/ sizeof(validTripleOps[0]);
static const char* validDoubleOps[] = { "==", "<=", ">=", "!=", "&&", "||", "++", "--", "+=", "-=", "*=", "%=", "&=", "|=", "^=", "<<", ">>", "->" }; int validDoubleOpsSize = sizeof(validDoubleOps)/sizeof(validDoubleOps[0]);
static const char validSingleOps[] = "+-*%=<>!&|~^.(){}[];,"; int validSingleOpsSize = sizeof(validSingleOps)/sizeof(validSingleOps[0]);
static const char* validKeywords[] = { "int", "float", "char", "if", "else", "for", "while", "break", "continue" }; int validKeywordsSize = sizeof(validKeywords)/sizeof(validKeywords[0]);
int strInArray(const char* str, const char* arr[], int arrSize);
int charInArray(char c, const char* arr, int arrSize);

enum TokenType {
    INT_LITERAL,
    FLOAT_LITERAL,
    CHAR_LITERAL,
    STR_LITERAL,
    IDENTIFIER,
    KEYWORD,
    OPERATOR,
    DELIMITER,
    EMPTY,
    END_OF_FILE
};

struct Token {
    int val;
    enum TokenType type;
    char* lexeme;
    int line, col;
    int length;
};

struct Token scanIdentifier(char** bpPtr, int* colPtr, int line) {
    char* start = *bpPtr;
    int startCol = *colPtr;

    while (isalnum(**bpPtr) || **bpPtr == '_') { (*bpPtr)++; (*colPtr)++; } // Find end of identifier

    if (start != *bpPtr) {
        char* end = *bpPtr;
        int length = end - start;
        struct Token idToken = { .type = IDENTIFIER, .line = line, .col = startCol, .lexeme = start, .length = length};
        return idToken;
    }
    struct Token emptyToken = { .type = EMPTY, .line = line, .col = *colPtr, .lexeme = NULL, .length = 0 };
    return emptyToken; // No identifier found
}

struct Token scanOpDelim(char** bpPtr, int* colPtr, int line) {
    switch (**bpPtr) {
        case '+': case '-': case '*': case '%': case '=': case '<': case '>': case '!': case '&': case '|': case '^': case '~': {
            int startCol = *colPtr;
            char* start = *bpPtr;

            if (**bpPtr && *(*(bpPtr)+1) && *(*(bpPtr)+2)) { // Check for three-char operators
                char trio[4] = { **bpPtr, *((*bpPtr) + 1), *((*bpPtr) + 2), '\0' };
                if (strInArray(trio, validTripleOps, validTripleOpsSize)) {
                    (*bpPtr) += 3; (*colPtr) += 3;
                    struct Token opToken = { .type = OPERATOR, .line = line, .col = startCol, .lexeme = start, .length = 3 };
                    return opToken;
                }
            }

            else if (**bpPtr && *(*(bpPtr)+1)) { // Check for two-char operators
                char pair[3] = { **bpPtr, *((*bpPtr) + 1), '\0' }; // two-char operator check
                if (strInArray(pair, validDoubleOps, validDoubleOpsSize)) {
                    (*bpPtr) += 2; (*colPtr) += 2;
                    struct Token opToken = { .type = OPERATOR, .line = line, .col = startCol, .lexeme = start, .length = 2 };
                    return opToken;
                }
            } 

            else if (**bpPtr) { // Check for single-char operator
            (*bpPtr)++; (*colPtr)++;
            struct Token opToken = { .type = OPERATOR, .line = line, .col = startCol, .lexeme = start, .length = 1 };
            return opToken;
            }
            else { // Emit empty token if **bpPtr points to \0. (Temporary, eventually will generate error.)
                struct Token emptyToken = { .type = EMPTY, .line = line, .col = startCol, .lexeme = NULL, .length = 0};
                return emptyToken;
            }

        }
        case '(': case ')': case '{': case '}': case '[': case ']': case ';': case ',': {
            struct Token delToken = { .type = DELIMITER, .line = line, .col = *colPtr, .lexeme = *bpPtr, .length = 1 };
            (*bpPtr)++; (*colPtr)++;
            return delToken;
        }
        default:
            struct Token emptyToken = { .type = EMPTY, .line = line, .col = *colPtr, .lexeme = NULL, .length = 0 };
            return emptyToken; // Unknown character
    }
}

struct Token scanCharLiteral(char** bpPtr, int* colPtr, int line) {
    char* start = *bpPtr;
    int startCol = *colPtr;
    (*bpPtr)++; (*colPtr)++; // Consume '

    while ((**bpPtr) != '\'') {
        if ((**bpPtr) == '\0') {
            struct Token emptyToken = { .type = EMPTY, .line = line, .col = startCol, .lexeme = NULL, .length = 0};
            return emptyToken;
        }
        else if ((**bpPtr) == '\\') { (*bpPtr)++; (*colPtr)++; }
        (*bpPtr)++; (*colPtr)++;
    }
    (*bpPtr)++; (*colPtr)++; // Skip closing '

    if (start != *bpPtr) {
        char* end = *bpPtr;
        int length = end - start;

        struct Token charToken = { .type = CHAR_LITERAL, .line = line, .col = startCol, .lexeme = start, .length = length };
        return charToken;
    }
}

struct Token scanStrLiteral(char** bpPtr, int* colPtr, int line) {
    char* start = *bpPtr;
    int startCol = *colPtr;

    (*bpPtr)++; (*colPtr)++; // Consume "

    while ((**bpPtr) != '\"') {
        if ((**bpPtr) == '\0') {
            struct Token emptyToken = { .type = EMPTY, .line = line, .col = startCol, .lexeme = NULL, .length = 0};
            return emptyToken;
        }
        else if ((**bpPtr) == '\\') { (*bpPtr)++; (*colPtr)++; }
        (*bpPtr)++; (*colPtr)++;
    }
    (*bpPtr)++; (*colPtr)++; // Skip closing "

    if (start != *bpPtr) {
        char* end = *bpPtr;
        int length = end - start;
        
        struct Token strToken = { .type = STR_LITERAL, .line = line, .col = startCol, .lexeme = start, .length = length };
        return strToken;
    }
}

struct Token scanFloatLiteral(char** bpPtr, char* start, int* colPtr, int startCol, int line) {
    (*bpPtr)++; *(colPtr)++; // Move past the . to avoid infinite loop
    while (isdigit(**bpPtr)) { (*bpPtr)++; (*colPtr)++; } // While we are reading digits, add them to the token

    if (start != *bpPtr) {
        char* end = *bpPtr;
        int length = end - start;
        
        struct Token floatToken = { .type = FLOAT_LITERAL, .line = line, .col = startCol, .lexeme = start, .length = length };
        return floatToken;
    }

    struct Token emptyToken = { .type = EMPTY, .line = line, .col = *colPtr, .lexeme = NULL, .length = 0 };
    return emptyToken; // Something weird happens (?)
}

struct Token scanIntLiteral(char** bpPtr, int* colPtr, int line) {
    char* start = *bpPtr;
    int startCol = *colPtr;

    while (isdigit(**bpPtr) || **bpPtr == '.') { // Scan integer literal
        if (**bpPtr == '.') { // If theres a dot, its a float
            return scanFloatLiteral(bpPtr, start, colPtr, startCol, line);
        }
        (*bpPtr)++; (*colPtr)++; 
    } 

    if (start != *bpPtr) {
        char* end = *bpPtr;
        int length = end - start;
        char* cur = start;

        int tokenValue = 0;
        while (cur < end) { // Transform string to int
            tokenValue = tokenValue * 10 + (*cur - '0');
            cur++;
        }
        struct Token intToken = { .type = INT_LITERAL, .line = line, .col = startCol, .val = tokenValue, .lexeme = start, .length = length };
        return intToken;
    }
    struct Token emptyToken = { .type = EMPTY, .line = line, .col = *colPtr, .lexeme = NULL, .length = 0 };
    return emptyToken; // No integer literal found
}

void emitToken(struct Token* token) {
    if (token->type == INT_LITERAL) {
        printf("INT_LITERAL: %d\n", token->val);
    }
    else if (token->type == FLOAT_LITERAL) {
        printf("FLOAT_LITERAL: %.*s\n", token->length, token->lexeme);
    }
    else if (token->type == STR_LITERAL) {
        printf("STR_LITERAL: %.*s\n", token->length, token->lexeme);
    }
    else if (token->type == CHAR_LITERAL) {
        printf("CHAR_LITERAL: %.*s\n", token->length, token->lexeme);
    }
    else if (token->type == IDENTIFIER) {
        printf("IDENTIFIER: %.*s\n", token->length, token->lexeme);
    }
    else if (token->type == OPERATOR) {
        printf("OPERATOR: %.*s\n", token->length, token->lexeme);
    }
    else if (token->type == DELIMITER) {
        printf("DELIMITER: %.*s\n", token->length, token->lexeme);
    }
    else if (token->type == END_OF_FILE) {
        printf("END_OF_FILE\n");
    }
}

int strInArray(const char* str, const char* arr[], int arrSize) {
    for (int i = 0; i < arrSize; i++) {
        if (strcmp(str, arr[i]) == 0) return 1;
    }
    return 0;
}

int charInArray(char c, const char* arr, int arrSize) {
    for (int i = 0; i < arrSize; i++) {
        if (c == arr[i]) return 1;
    }
    return 0;
}

int main() {
    char fileName[256];
    printf("Entire path to input file: \n");

    scanf("%255s", fileName);
    FILE* file = fopen(fileName, "r");
    if (!file) {
        printf("Error opening file\n");
        return -1;
    }
    
    // Get length of file for buffer allocation
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    if (fileSize <= 0) {
        printf("Empty file or error reading file size\n");
        fclose(file);
        return -1;
    }

    // Allocate buffer for file contents
    char* buf = malloc(fileSize + 1);
    if (!buf) {
        printf("Memory allocation failed\n");
        fclose(file);
        return -1;
    }

    size_t bytes = fread(buf, 1, fileSize, file);
    buf[bytes] = '\0'; // Null-terminate the buffer
    fclose(file);

    int line = 1, col = 0;
    char* bp = buf; // Buffer pointer
    while (*bp) {
        if (*bp == ' ' || *bp == '\t' || *bp == '\n') {
            if (*bp == '\n') { line++; bp++; col=0; continue; }
            bp++; col++;
            continue;
        }
        else if (*bp == '/') {
            // Handle comments
            if (*(bp + 1) && *(bp + 1) == '/') {
                // Single-line comment
                while (*bp && *bp != '\n') { bp++; col++; }
                continue;
            } 
            else if (*(bp + 1) && *(bp + 1) == '*') {
                // Multi-line comment
                bp += 2; col+=2; // Skip '/*'
                if (*bp == '\0') { // Unterminated comment
                    printf("Error: Unterminated comment\n");
                    free(buf);
                    return -1;
                }
                while (*bp && *(bp+1) && !(*bp == '*'  && *(bp + 1) == '/')) { 
                    bp++; col++;  
                    if (*bp == '\n') { line++; col=0; }
                }
                if (*bp) { bp += 2; col+=2; } // Skip '*/'
                continue;
            } 
            else {
                // Division operator
                if (*(bp + 1) && *(bp + 1) == '=') { // '/=' operator
                    struct Token opToken = { .type = OPERATOR, .line = line, .col = col, .lexeme = bp, .length = 2 };
                    emitToken(&opToken); // Emit division-equals operator token
                    bp += 2; col += 2;
                }
                else {
                    struct Token opToken = { .type = OPERATOR, .line = line, .col = col, .lexeme = bp, .length = 1 };
                    emitToken(&opToken); // Emit division operator token
                    bp++; col++;
                }
            }
        } 
        else {
            // Any numeric character
            if (isdigit(*bp)) { // Integer literal
                struct Token intToken = scanIntLiteral(&bp, &col, line);
                if (intToken.lexeme) emitToken(&intToken); // Emit integer literal token
            }
            else if (*bp == '.' && *(bp + 1) && isdigit(*(bp + 1))) { // Fractional float e.g: .5 
                struct Token floatToken = scanFloatLiteral(&bp, bp, &col, col, line);
                if (floatToken.lexeme) emitToken(&floatToken);
            }
            else if (*bp == '\"') { // String literal
                struct Token strToken = scanStrLiteral(&bp, &col, line);
                if (strToken.lexeme) emitToken(&strToken);
            }

            else if (*bp == '\'') { // Char literal
                struct Token charToken = scanCharLiteral(&bp, &col, line);
                if (charToken.lexeme) emitToken(&charToken);
            }

            else if (isalpha(*bp) || *bp == '_') { // Any alphanumeric character or underscore
                struct Token idToken = scanIdentifier(&bp, &col, line);
                if (idToken.lexeme) emitToken(&idToken); // Emit identifier token
            }

            else if (charInArray(*bp, validSingleOps, validSingleOpsSize)) { // Operator or Delimiter
                struct Token opDelimToken = scanOpDelim(&bp, &col, line);
                if (opDelimToken.type != END_OF_FILE) emitToken(&opDelimToken); // Emit operator or delimiter token

                else { bp++; col++; } // Unknown character; skip
            }
            else { bp++; col++; }
        }
    }

    struct Token eofToken = { .type = END_OF_FILE, .line = line, .col = col, .val = 0, .lexeme = NULL, .length = 0 };
    emitToken(&eofToken);
    
    free(buf);
}