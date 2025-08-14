#ifndef LEXER_NEXT_H
#define LEXER_NEXT_H

#include <stddef.h>

typedef enum {
    TK_LBRACE, TK_RBRACE, TK_LBRACKET, TK_RBRACKET, TK_COLON, TK_COMMA, TK_EOF, TK_ERROR,
    TK_STRING, TK_NUMBER, TK_TRUE, TK_FALSE, TK_NULL
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

int scan_number(const char *p, Token *token, int line, int col);

int scan_literal_true(const char *p, Token *token, int line, int col);

int scan_literal_false(const char *p, Token *token, int line, int col);

int scan_literal_null(const char *p, Token *token, int line, int col);

int scan_string(const char *p, Token *token, int line, int col);

#endif