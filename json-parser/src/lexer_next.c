
#include "lexer_next.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static int is_boundary(char p) {
    return (p == '\0' || isspace(p) || p == ',' || p == ']' || p == '}');
}

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
            case '\r':
                line++;
                col = 1;
                if (*(buf + index + 1) == '\n')
                    index++;
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

    int number_length = 0;
    int char_length = 0;

    if (buf[index] == '-' || isdigit((unsigned char) buf[index]) != 0) {
        number_length = scan_number(buf + index, &token, line, col);
        if (number_length) {
            // 이러면 숫자인거고. 숫자 길이만큼. index를 추가해준다.
            index += number_length;
            col += number_length;
            return token;
        } else {
            token.message = "invalid number";
            token.col = col;
            token.line = line;
            token.kind = TK_ERROR;
            token.lexeme = buf+ index;
            token.length = 1;
            index++;
            col++;
            return token;
        }
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
        case 't':
            if (scan_literal_true(buf + index, &token, line, col)) {
                index += 4;
                col += 4;
                return token;
            } else {
                token.message = "invalid token starts with: t";
                token.col = col;
                token.line = line;
                token.kind = TK_ERROR;
                token.lexeme = buf+ index;
                token.length = 1;
                index++;
                col++;
                return token;
            }
        case 'f':
            if (scan_literal_false(buf + index, &token, line, col)) {
                index += 5;
                col += 5;
                return token;
            } else {
                token.message = "invalid token starts with: f";
                token.col = col;
                token.line = line;
                token.kind = TK_ERROR;
                token.lexeme = buf+ index;
                token.length = 1;
                index++;
                col++;
                return token;
            }
        case 'n':
            if (scan_literal_null(buf + index, &token, line, col)) {
                index += 4;
                col += 4;
                return token;
            } else {
                token.message = "invalid token starts with: n";
                token.col = col;
                token.line = line;
                token.kind = TK_ERROR;
                token.lexeme = buf+ index;
                token.length = 1;
                index++;
                col++;
                return token;
            }
        case '"':
            char_length = scan_string(buf + index, &token, line,col);
            if (char_length) {
                index += char_length;
                col += char_length;
                return token;
            } else {
                token.message = "invalid string";
                token.col = col;
                token.line = line;
                token.kind = TK_ERROR;
                token.lexeme = buf+ index;
                token.length = 1;
                index++;
                col++;
                return token;
            }
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
        case TK_STRING:   return "TK_STRING";
        case TK_TRUE:     return "TK_TRUE";
        case TK_FALSE:    return "TK_FALSE";
        case TK_NUMBER:   return "TK_NUMBER";
        case TK_NULL:     return "TK_NULL";
        default:          return "UNKNOWN";
    }
}

void print_token(Token token) {
    if (token.kind != TK_ERROR) 
    printf("L%d:C%d %s\n",token.line, token.col, printTokenKind(token.kind));
    else 
    printf("L%d:C%d %s\n not supported type %c",token.line, token.col, printTokenKind(token.kind), *token.lexeme);
}


int scan_number(const char* p, Token *token, int line, int col) {
    int length = 0;

    // - 가 오는 경우는 한번은 허용.
    if (p[length] == '-') {
        length++;
    }

    // 정수부
    if (!isdigit((unsigned char)p[length])) return 0;
    if (p[length] == '0') {
        length++;
    } else {
        while (isdigit((unsigned char)p[length])) length++;
    }

    // 소수부
    if (p[length] == '.') {
        int j = length + 1;
        if (!isdigit((unsigned char)p[j])) return 0; // 점 뒤 최소 1자리
        length = j;
        while (isdigit((unsigned char)p[length])) length++;
    }

    // 지수부
    if (p[length] == 'e' || p[length] == 'E') {
        int j = length + 1;
        if (p[j] == '+' || p[j] == '-') j++;
        if (!isdigit((unsigned char)p[j])) return 0;
        length = j + 1;
        while (isdigit((unsigned char)p[length])) length++;
    }

    if (!is_boundary(p[length])) return 0;

    token -> length = length;
    token -> kind = TK_NUMBER;
    token -> col = col;
    token -> line = line;
    token -> lexeme = p;
        
    return length;
}

// col은 어떻게 처리하지? 이거 token.col을 기록하고 나서, col을 +4해야하는거 아니낙?
int scan_literal_true(const char *p, Token *token, int line, int col) {
    if (strncmp(p, "true", 4) == 0 && is_boundary(p[4])) {
        token -> kind = TK_TRUE;
        token -> length = 4;
        token -> line = line;
        token -> col = col;
        token -> lexeme = p;
        return 1;
    } else {
        return 0;
    }
}

int scan_literal_false(const char *p, Token *token, int line, int col) {
    if (strncmp(p, "false", 5) == 0 && is_boundary(p[5])) {
        token -> kind = TK_FALSE;
        token -> length = 5;
        token -> line = line;
        token -> col = col;
        token -> lexeme = p;
        return 1;
    } else {
        return 0;
    }
}

int scan_literal_null(const char *p, Token *token, int line, int col) {
    if (strncmp(p, "null", 4) == 0 && is_boundary(p[4])) {
        token -> kind = TK_NULL;
        token -> length = 4;
        token -> line = line;
        token -> col = col;
        token -> lexeme = p;
        return 1;
    } else {
        return 0;
    }
}

int scan_string(const char *p, Token *token, int line, int col) {
    int i = 0;
    if (p[i] != '"') return 0;
    i++; // opening quote

    for (;;) {
        char c = p[i];
        if (c == '\0') return 0; // EOF 전 종료 → 에러
        if (c == '\n' || c == '\r') return 0; // raw 개행 금지

        if (c == '\\') { // 이스케이프
            i++;
            char e = p[i];
            if (e == '\0') return 0;
            if (e == 'u') {
                // \uXXXX 처리 (간단 검증)
                for (int k = 0; k < 4; k++) {
                    char h = p[++i];
                    if (!isxdigit((unsigned char)h)) return 0;
                }
                i++;
            } else {
                i++;
            }
            continue;
        }

        if (c == '"') {
            i++; // closing quote 포함
            token->kind = TK_STRING;
            token->length = i;
            token->line = line;
            token->col = col;
            token->lexeme = p;
            return i;
        }

        i++; // 일반 문자
    }
}