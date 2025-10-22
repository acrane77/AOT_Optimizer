#include <stdio.h>
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

struct Token {
    int val;
    enum TokenType type;
    char* lexeme;
    int line, col;
    int length;
};

struct TokenBuffer {
    struct Token* buf;
    size_t count;
    size_t capacity;
    const char* src;
};