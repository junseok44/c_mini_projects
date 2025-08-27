#include "lexer_next.h"
#include <parser.h>
#include <stdlib.h>
#include <string.h>
#include "value.h"

// ---- 내부 유틸 선언 (이 파일 안에서만 사용) -----------------
static void set_err(Parser *p, const char *msg, int line, int col);
static int  accept(Parser *p, TokenKind k);
static int  expect(Parser *p, TokenKind k, const char *what);

static int  parse_value(Parser *p, Value *out);
static int  parse_object(Parser *p, Value *out);   // '{' 소비된 뒤에 진입
static int  parse_array(Parser *p, Value *out);    // '[' 소비된 뒤에 진입

static int  set_string_owned(Value *v, const char *ptr, size_t len);
static int  arr_push(Value *arr, Value *elem_ptr_owned);
static int  obj_push(Value *obj, const JMember *m);

static void next(Parser *p);

void parser_init(Parser *p) {
    p->depth = 0;
    p->max_depth = 0;
    p->err = (ParseError){0};
    p->look = (Token){0};
    p->lx = NULL;
}

int parser_parse(Parser *p, Value *v) {
    if (!p || !p->lx || !v) return 0;

    next(p);
    
    if (!parse_value(p, v)) {
        *v = (Value){0};
        return 0;
    }
    if (p->look.kind != TK_EOF) {
        set_err(p, "trailing data after JSON value", p->look.line, p->look.col);
        jv_free(v);
        *v = (Value){0};
        return 0;
    }

    return 1;
}

static void next(Parser *p) {
    p->look = lexer_next(p->lx);
}

static void set_err(Parser *p, const char *msg, int line, int col) {
    if (!p) return;
    p->err.msg = msg;
    p->err.line = line;
    p->err.col = col;
}

static int accept(Parser *p, TokenKind k) {
    if (p->look.kind == k) {
        next(p);
        return 1;
    }
    return 0;
}

static int set_string_owned(Value *v, const char *ptr, size_t len) {
    v->kind = JV_STRING;
    v->u.str.len = len;
    v->u.str.is_owned = 1;

    char *buf = malloc(len + 1);
    if (!buf) return 0;
    memcpy(buf, ptr, len);
    buf[len] = '\0';
    v->u.str.ptr = buf;

    return 1;
}

static int expect(Parser *p, TokenKind k, const char *error) {
    if (accept(p,k)) return 1;
    set_err(p, error, p->look.line, p->look.col);
    
    return 0;
}

static int parse_value(Parser *p, Value *out) {
    p->depth++;

    if (p->depth > p->max_depth) {
        p->max_depth = p->depth;
    }

    int ok = 0;

    switch(p->look.kind) {
        case TK_LBRACE: {
            next(p);
            ok = parse_object(p, out);
            break;
        }
        case TK_LBRACKET: {
            next(p);
            ok = parse_array(p, out);
            break;
        }
        case TK_STRING: {
            ok = set_string_owned(out, p->look.lexeme, p->look.length);
            next(p);
            break;
        }
        case TK_NUMBER: {
            char *ptr = malloc(p->look.length + 1);
            if (!ptr) { ok = 0; break; }
            memcpy(ptr,p->look.lexeme, p->look.length);
            ptr[p->look.length] = '\0';
            
            out->kind = JV_NUMBER;
            out->u.num = strtod(ptr,NULL);

            free(ptr);
            next(p);

            ok = 1;

            break;
        }
        case TK_TRUE: {
            out->kind = JV_TRUE;
            next(p);
            ok = 1;
            break;
        }
        case TK_FALSE: {
            out->kind = JV_FALSE;
            next(p);
            ok = 1;
            break;
        }
        case TK_NULL: {
            out->kind = JV_NULL;
            next(p);
            ok = 1;
            break;
        }
        default : {
            set_err(p, "unexpected token while parsing value", p->look.line, p->look.col);
            ok = 0;
            break;
        }
    }

    p -> depth--;
    return ok;
}

static int obj_push(Value *v, const JMember *m) {
    if (v->u.obj.len >= v->u.obj.cap) {
        
        size_t new_cap = v->u.obj.cap ? v->u.obj.cap * 2 : 10;
        JMember *new_items = realloc(v->u.obj.items, sizeof(JMember) * new_cap);
        
        if (!new_items) return 0;

        v->u.obj.items = new_items;
        v->u.obj.cap = new_cap;
    }

    v->u.obj.items[v->u.obj.len++] = *m;
    return 1;
}

static int arr_push(Value *v, Value *element) {
    if (v->u.arr.len >= v->u.arr.cap) {
        size_t new_cap = v->u.arr.cap ? v->u.arr.cap * 2 : 10;
        Value **new_items = realloc(v->u.arr.items, sizeof(Value *) * new_cap);
        if (!new_items) return 0;
        
        v -> u.arr.items = new_items;
        v -> u.arr.cap = new_cap;
    }
    
    v->u.arr.items[v->u.arr.len++] = element;
    return 1;
}

static int parse_array(Parser *p, Value *out) {
    out->kind = JV_ARRAY;
    out->u.arr.items = NULL;
    out->u.arr.len = 0;
    out->u.arr.cap = 0;

    if (accept(p, TK_RBRACKET)) return 1;

    for (;;) {
        Value *element = jv_new();
        if (!element) return 0;

        if (!parse_value(p, element)) {
            jv_free(element);
            return 0;
        }

        if (!arr_push(out, element)) {
            jv_free(element);
            return 0;
        }

        if (accept(p, TK_COMMA)) continue;

        if (!expect(p, TK_RBRACKET, "expected ']' at end of array")) return 0;
        break;
    }

    return 1;
}

static int parse_object(Parser *p, Value *out) {
    out->kind = JV_OBJECT;
    out->u.obj.items = NULL;
    out->u.obj.len = 0;
    out->u.obj.cap = 0;

    if (accept(p, TK_RBRACE)) return 1;

    for (;;) {

        if (p->look.kind != TK_STRING) {
            set_err(p, "object key must be string", p->look.line, p->look.col);
            return 0;
        }

        JMember m;
        memset(&m, 0, sizeof(m));
        m.key.ptr = NULL;
        m.key.len = 0;
        m.key.is_owned = 0;

        m.key.len = p->look.length;

        // 널 문자 추가.
        char *key_buffer = malloc(m.key.len + 1);

        if (!key_buffer) {
            return 0;
        }

        memcpy((void *)key_buffer, p->look.lexeme, p->look.length);

        key_buffer[m.key.len] = '\0';
        m.key.ptr = key_buffer;
        m.key.is_owned = 1;

        next(p);
        
        if (!expect(p, TK_COLON, "expected ':' after object key")) {
            free(key_buffer);
            return 0;
        }

        m.value = jv_new();
        if (!m.value) {
            free(key_buffer);
            return 0;
        }
        
        if (!parse_value(p, m.value)) {
            jv_free(m.value);
            free((void *)m.key.ptr);
            return 0;
        }        

        if (!obj_push(out, &m)) {
            jv_free(m.value);
            free((void*)m.key.ptr);
            return 0;
        }

        if (accept(p, TK_COMMA)) continue;

        if (!expect(p, TK_RBRACE, "expected '}' at end of object")) return 0;
        
        break;
    }

    return 1;
}

const char *parser_error_msg(const Parser *p) {
    return p->err.msg ? p->err.msg : "unknown error";
}

int parser_error_line(const Parser *p) {
    return p->err.line;
}

int parser_error_col(const Parser *p) {
    return p->err.col;
}