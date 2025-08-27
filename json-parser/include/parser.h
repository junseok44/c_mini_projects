#ifndef PARSER_H
#define PARSER_H

#include "lexer_next.h"
#include "value.h"

typedef struct {
    const char *msg;
    int line;
    int col;
} ParseError;

typedef struct {
    Lexer *lx;
    Token look;
    ParseError err;
    int depth;
    int max_depth;
} Parser;

void parser_init(Parser *p);

int parser_parse(Parser *p, Value *v);

int parser_init_buffer(Parser *p, const char *buf, size_t len);

const char *parser_error_msg(const Parser *p);

int parser_error_line(const Parser *p);

int parser_error_col(const Parser *p);


#endif