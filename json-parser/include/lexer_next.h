#include <stddef.h>

typedef enum {
    TK_LBRACE, TK_RBRACE, TK_LBRACKET, TK_RBRACKET, TK_COLON, TK_COMMA, TK_EOF, TK_ERROR
} TokenKind;


typedef struct {
    TokenKind kind;
    int line;
    int col;
    const char *lexeme;
    size_t length;
    const char* message;
} Token;

Token lexer_next(char *buf);

void print_token(Token token);