#include <stdio.h>

// Token types
enum TokenType {
    INT_LITERAL,
    FLOAT_LITERAL,
    CHAR_LITERAL,
    STR_LITERAL,
    BOOL_LITERAL,
    IDENTIFIER,
    FUNCTION,
    ARRAY,
    KEYWORD,
    OPERATOR,
    DELIMITER,
    EMPTY,
    END_OF_FILE
};

// Token struct
struct Token {
    int val; // Value for integer/float tokens 
    enum TokenType type;
    char* lexeme; // Start of token
    int line, col; // line/col for error reporting
    int length; // Length of token (end = length - start)
};

// TokenBuffer (tb) struct
struct TokenBuffer {
    struct Token* buf; // Buffer of tokens
    size_t count; // Current number of tokens in buf
    size_t capacity; // Capacity of buf (default = 128)
    const char* src; // For storing buf allocated in sc_lexer.c
};

// Declare lexFile() so it can be seen across files
struct TokenBuffer lexFile();

// Parser struct
struct Parser {
    struct Token* tokens; // Array of tokens
    size_t count; // Total tokens in array
    size_t pos; // Current position
    const char* src; // buf allocated in sc_lexer.c
    int errCount; // Number of errors
};
