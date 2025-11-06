/* 
 * S-C (simple C) Lexer Supports:
 * Basic scalar types (int, float, char, str, bool)
 * Bitwise operations (&, |, ^, ~, <<, >>)
 * Arithmetic operations (+, -, *, /, %)
 * Boolean operations (&&, ||, !, ==, !=, <, >, <=, >=)
 * Control flow (if, else, while, for (simple bounds), break, continue)
 * Simple (non-recursive) Functions (declaration, definition, calls)
 * Simple multi-dimensional arrays & restrictive pointers
*/
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "sc_token.h"
#define MAX_KEYWORD_LEN 8

static const char* validTripleOps[] = { "<<=", ">>=" }; int validTripleOpsSize = sizeof(validTripleOps)/sizeof(validTripleOps[0]);
static const char* validDoubleOps[] = { "==", "<=", ">=", "!=", "&&", "||", "++", "--", "+=", "-=", "*=", "%=", "&=", "|=", "^=", "<<", ">>", "->" }; int validDoubleOpsSize = sizeof(validDoubleOps)/sizeof(validDoubleOps[0]);
static const char validSingleOps[] = "+-*%=<>!&|~^.(){}[];,"; int validSingleOpsSize = sizeof(validSingleOps)/sizeof(validSingleOps[0]);
static const char* validKeywords[] = { "int", "float", "char", "bool", "void", "if", "else", "for", "while", "break", "continue", "return", "const", "static", "nullptr", "NULL" }; int validKeywordsSize = sizeof(validKeywords)/sizeof(validKeywords[0]);
static struct TokenBuffer tb;

int strInArray(const char* str, const char* arr[], int arrSize);
int charInArray(char c, const char* arr, int arrSize);
void scanForTokens(char** bpPtr, int* colPtr, int line);
void emitToken(struct Token* token);

struct Token scanFunction(char** bpPtr, char* start, int* colPtr, int startCol, int line) {
    int bracketDepth = 1;
    struct Token oBracketToken = { .type = DELIMITER, .line = line, .col = startCol, .lexeme = *bpPtr, .length = 1 };
    emitToken(&oBracketToken);
    (*bpPtr)++; (*colPtr)++;

    int isString = 0; int isChar = 0;
    char* argsStart = *bpPtr; char* argsEnd;

    while (bracketDepth > 0) {
        if (**bpPtr == '\"' && *((*bpPtr) - 1) != '\\' && !isChar) isString = !isString;
        else if (**bpPtr == '\'' && *((*bpPtr) - 1) != '\\' && !isString) isChar = !isChar;
        if (isString || isChar) { 
            if (**bpPtr == '\0') {
                struct Token emptyToken = { .type = EMPTY, .line = line, .col = *colPtr, .lexeme = NULL, .length = 0 };
                return emptyToken; // EOF
            }
            (*bpPtr)++; (*colPtr)++; 
            continue; 
        }
        if (**bpPtr == '\0') {
            struct Token emptyToken = { .type = EMPTY, .line = line, .col = startCol, .lexeme = NULL, .length = 0 };
            return emptyToken; // Temp
        }
        else if (**bpPtr == '(') { 
            bracketDepth++;
        }
        else if (**bpPtr == ')') {  
            bracketDepth--;
            if (bracketDepth == 0) {
                argsEnd = *bpPtr - 1;
                break;
            }
        }
        (*bpPtr)++; (*colPtr)++;
    }
    struct Token functionToken = { .type = FUNCTION, .line = line, .col = startCol, .lexeme = start, .length = (*bpPtr + 1) - start};

    char* inner = argsStart;
    int dummyCol = *colPtr;
    while (inner <= argsEnd) {
        scanForTokens(&inner, &dummyCol, line);
    }
    *bpPtr = argsEnd + 2;
    *colPtr += 2;

    while (**bpPtr == ' ') { (*bpPtr)++; (*colPtr)++; } // consume any extra spaces

    return functionToken;
}

struct Token scanArray(char** bpPtr, char* start, int* colPtr, int startCol,  int line) {
    int bracketDepth = 1;
    (*bpPtr)++; (*colPtr)++; // Consume bracket open

    while (bracketDepth > 0) { 
        if (**bpPtr == '\0'){
            struct Token emptyToken = { .type = EMPTY, .line = line, .col = startCol, .lexeme = NULL, .length = 0 };
            return emptyToken; // Temporary: emit empty token for EOF/no closing bracket.
        }
        else if (**bpPtr == '[') bracketDepth++;
        else if (**bpPtr == ']') bracketDepth--;
        (*bpPtr)++; (*colPtr)++; 
    }

    while (**bpPtr == ' ') { (*bpPtr)++; (*colPtr)++; } // consume any extra spaces

    if (start != *bpPtr) {
        char* end = *bpPtr;
        int length = end - start;

        if (**bpPtr == '[') { // Square/cube/n size matrix, recurse on bracket.
            return scanArray(bpPtr, start, colPtr, startCol, line);
        }
        else {
            struct Token arrayToken = { .type = ARRAY, .line = line, .col = startCol, .lexeme = start, .length = length };
            return arrayToken;
        }
    }
    struct Token emptyToken = { .type = EMPTY, .line = line, .col = *colPtr, .lexeme = NULL, .length = 0 };
    return emptyToken; // (?)
}

struct Token scanIdentifier(char** bpPtr, int* colPtr, int line) {
    char* start = *bpPtr;
    int startCol = *colPtr;

    while (isalnum(**bpPtr) || **bpPtr == '_') { (*bpPtr)++; (*colPtr)++; } // Find end of identifier

    if (start != *bpPtr) {
        char* end = *bpPtr;
        int length = end - start;
        int isKeyword = 0;
        char keyword[MAX_KEYWORD_LEN + 1]; 

        if (length <= MAX_KEYWORD_LEN) { // To avoid non-constant array allocation, we know the max length is 8 (continue) so we don't need to check any strings longer.
            memcpy(keyword, start, length); // Max length of current string is now 8, so this is always safe.
            keyword[length] = '\0'; // null terminate string
            if (strInArray(keyword, validKeywords, validKeywordsSize)) isKeyword = 1; // if its in the array we know its a keyword
        }

        if ((**bpPtr == '[') || (**bpPtr == ' ' && *(*bpPtr + 1) && *(*bpPtr + 1) == '[')) { // Basic Array definition ( arr[] or arr [] )
            if (**bpPtr == ' ') { (*bpPtr)++; (*colPtr)++; }
            return scanArray(bpPtr, start, colPtr, startCol, line);
        }

        else if (((**bpPtr == '(') || (**bpPtr == ' ' && *(*bpPtr + 1) && *(*bpPtr + 1) == '(')) && !isKeyword) { // Basic function defintion
            if (**bpPtr == ' ') { (*bpPtr)++; (*colPtr)++; }
            
            return scanFunction(bpPtr, start, colPtr, startCol, line);
        }
        else if (length <= MAX_KEYWORD_LEN && (!strcmp(keyword, "true") || !strcmp(keyword, "false"))) { // Emit bool token (use length check first to avoid strcmp'ing massive strings)
            struct Token boolToken = { .type = BOOL_LITERAL, .line = line, .col = startCol, .lexeme = start, .length = length };
            return boolToken;
        }

        if (isKeyword) { 
            // emit keyword token
            struct Token keywordToken =  { .type = KEYWORD, .line = line, .col = startCol, .lexeme = start, .length = length };
            return keywordToken;
        }
    
        // Else emit an idToken
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

            if (**bpPtr && *(*bpPtr + 1) && *(*bpPtr + 2)) { // Check for three-char operators
                char trio[4] = { **bpPtr, *((*bpPtr) + 1), *((*bpPtr) + 2), '\0' };
                if (strInArray(trio, validTripleOps, validTripleOpsSize)) {
                    (*bpPtr) += 3; (*colPtr) += 3;
                    struct Token opToken = { .type = OPERATOR, .line = line, .col = startCol, .lexeme = start, .length = 3 };
                    return opToken;
                }
            }

            if (**bpPtr && *(*bpPtr + 1)) { // Check for two-char operators
                char pair[3] = { **bpPtr, *((*bpPtr) + 1), '\0' }; // two-char operator check
                if (strInArray(pair, validDoubleOps, validDoubleOpsSize)) {
                    (*bpPtr) += 2; (*colPtr) += 2;
                    struct Token opToken = { .type = OPERATOR, .line = line, .col = startCol, .lexeme = start, .length = 2 };
                    return opToken;
                }
            } 

            if (**bpPtr) { // Check for single-char operator
                (*bpPtr)++; (*colPtr)++;
                struct Token opToken = { .type = OPERATOR, .line = line, .col = startCol, .lexeme = start, .length = 1 };
                return opToken;
            }
            else { // Emit empty token if **bpPtr points to \0. (Temporary, eventually will generate error.)
                struct Token emptyToken = { .type = EMPTY, .line = line, .col = startCol, .lexeme = NULL, .length = 0};
                return emptyToken;
            }

        }
        // Delimiter cases
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
    struct Token emptyToken = { .type = EMPTY, .line = line, .col = *colPtr, .lexeme = NULL, .length = 0 };
    return emptyToken;
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
    struct Token emptyToken = { .type = EMPTY, .line = line, .col = *colPtr, .lexeme = NULL, .length = 0 };
    return emptyToken;
}

struct Token scanFloatLiteral(char** bpPtr, char* start, int* colPtr, int startCol, int line) {
    (*bpPtr)++; *(colPtr)++; // Move past the . to avoid infinite loop
    while (isdigit(**bpPtr)) { // While we are reading digits, add them to the token
        (*bpPtr)++; (*colPtr)++; 
    } 
    if (**bpPtr == 'f' || **bpPtr == 'F') { (*bpPtr)++; (*colPtr)++; } // Allow f suffix (5.0f is valid)

    if (start != *bpPtr) {
        char* end = *bpPtr;
        int length = end - start;
        char* cur = start;

        float tokenValue = strtof(start, NULL);
        struct Token floatToken = { .type = FLOAT_LITERAL, .line = line, .val=tokenValue, .col = startCol, .lexeme = start, .length = length };
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
        struct Token intToken = { .type = INT_LITERAL, .line = line, .col = startCol, .val = (float)tokenValue, .lexeme = start, .length = length };
        return intToken;
    }
    struct Token emptyToken = { .type = EMPTY, .line = line, .col = *colPtr, .lexeme = NULL, .length = 0 };
    return emptyToken; // No integer literal found
}

void emitToken(struct Token* token) {
    if (tb.count == tb.capacity) {
        tb.capacity *= 2;
        struct Token* temp = realloc(tb.buf, tb.capacity * sizeof(struct Token));
        if (!temp) {
            fprintf(stderr, "Memory reallocation failed!\n");
            exit(1);
        }
        tb.buf = temp;
    }
    tb.buf[tb.count++] = *token;
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

void scanForTokens(char** bpPtr, int* colPtr, int line) {
    // Any numeric character
    if (isdigit(**bpPtr)) { // Integer literal
        struct Token intToken = scanIntLiteral(bpPtr, colPtr, line);
        if (intToken.lexeme) emitToken(&intToken); // Emit integer literal token
    }
    else if (**bpPtr == '.' && *(*bpPtr + 1) && isdigit(*(*bpPtr + 1))) { // Fractional float e.g: .5 
        struct Token floatToken = scanFloatLiteral(bpPtr, *bpPtr, colPtr, *colPtr, line);
        if (floatToken.lexeme) emitToken(&floatToken);
    }
    else if (**bpPtr == '\"') { // String literal
        struct Token strToken = scanStrLiteral(bpPtr, colPtr, line);
        if (strToken.lexeme) emitToken(&strToken);
    }

    else if (**bpPtr == '\'') { // Char literal
        struct Token charToken = scanCharLiteral(bpPtr, colPtr, line);
        if (charToken.lexeme) emitToken(&charToken);
    }

    else if (isalpha(**bpPtr) || **bpPtr == '_') { // Any alphanumeric character or underscore
        struct Token idToken = scanIdentifier(bpPtr, colPtr, line);
        if (idToken.lexeme) emitToken(&idToken); // Emit identifier token
    }

    else if (charInArray(**bpPtr, validSingleOps, validSingleOpsSize)) { // Operator or Delimiter
        struct Token opDelimToken = scanOpDelim(bpPtr, colPtr, line);
        if (opDelimToken.type != END_OF_FILE) emitToken(&opDelimToken); // Emit operator or delimiter token

        else { (*bpPtr)++; (*colPtr)++; } // Unknown character; skip
    }
    else { (*bpPtr)++; (*colPtr)++; }
}

struct TokenBuffer lexFile(char* fileName) {

    FILE* file = fopen(fileName, "r");
    if (!file) {
        printf("Error opening file\n");
        return tb;
    }
    
    // Get length of file for buffer allocation
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    if (fileSize <= 0) {
        printf("Empty file or error reading file size\n");
        fclose(file);
        tb.buf = NULL; tb.src = NULL;
        return tb;
    }
    
    // Allocate buffer for file contents
    char* buf = malloc(fileSize + 1);
    tb.capacity = 128;
    tb.buf = malloc(tb.capacity * sizeof(struct Token));

    if (!buf) {
        printf("Memory allocation failed\n");
        fclose(file);
        tb.buf = NULL; tb.src = NULL;
        return tb;
    }
    else if (!tb.buf) {
        printf("Memory allocation failed\n");
        fclose(file);
        free(buf);
        tb.buf = NULL; tb.src = NULL;
        return tb;       
    }
    
    tb.count = 0;
    tb.src = buf;
    
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
                    return tb;
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
            scanForTokens(&bp, &col, line);
        }
    }

    struct Token eofToken = { .type = END_OF_FILE, .line = line, .col = col, .val = 0, .lexeme = NULL, .length = 0 };
    emitToken(&eofToken);

    return tb;
}