
#include "lexer_next.h"
#include <stddef.h>
#include <stdio.h>

Token lexer_next(char *buf) {
    Token token = {0};

    static int line = 1;
    static int col = 1;
    static size_t index = 0;

    while (buf[index] == ' ' || buf[index] == '\n' || buf[index] == '\r' || buf[index] == '\t') {
        switch(buf[index]) {
            case ' ':
                col++;
                break;
            case '\n':
                line++;
                col = 1;
                break;
            case '\t':
                col += 4;
                break;
        }
        index++;
    }
    
    switch(buf[index]) {
        case '{':
            token.kind = TK_LBRACE;
            break;
        case '}':
            token.kind = TK_RBRACE;
            break;
        case '[':
            token.kind = TK_LBRACKET;
            break;
        case ']':
            token.kind = TK_RBRACKET;
            break;
        case ':':
            token.kind = TK_COLON;
            break;
        case ',':
            token.kind = TK_COMMA;
            break;
        case '\0':
            token.kind = TK_EOF;
            token.lexeme = buf + index;
            token.col = col;
            token.line = line;
            token.length = 0;
            return token;
        default:
            token.kind = TK_ERROR;
            token.message = "not supported token";
            token.col = col;
            token.line = line;
            token.lexeme = buf + index;
            token.length = 1;
            index++;
            col++;
            return token;
    }

    token.col = col;
    token.line = line;  
    token.lexeme = buf + index;
    token.length = 1;
    index++;
    col++;

    return token;
}

const char *printTokenKind(TokenKind kind) {
    switch (kind) {
        case TK_LBRACE:   return "TK_LBRACE";
        case TK_RBRACE:   return "TK_RBRACE";
        case TK_LBRACKET: return "TK_LBRACKET";
        case TK_RBRACKET: return "TK_RBRACKET";
        case TK_COLON:    return "TK_COLON";
        case TK_COMMA:    return "TK_COMMA";
        case TK_EOF:      return "TK_EOF";
        case TK_ERROR:    return "TK_ERROR";
        default:          return "UNKNOWN";
    }
}

void print_token(Token token) {
    printf("L%d:C%d %s\n",token.line, token.col, printTokenKind(token.kind));
}