#include <stdio.h>
#include <string.h>
#include "value.h"

static void indent_n(FILE *out, int n) { for (int i=0;i<n;i++) fputc(' ', out); }

static void dump_string_escaped(const char *s, size_t len, FILE *out) {
    fputc('"', out);
    for (size_t i=0;i<len;i++) {
        unsigned char c = (unsigned char)s[i];
        switch (c) {
            case '\\': fputs("\\\\", out); break;
            case '"' : fputs("\\\"", out); break;
            case '\b': fputs("\\b", out); break;
            case '\f': fputs("\\f", out); break;
            case '\n': fputs("\\n", out); break;
            case '\r': fputs("\\r", out); break;
            case '\t': fputs("\\t", out); break;
            default:
                if (c < 0x20) fprintf(out, "\\u%04x", c);
                else fputc(c, out);
        }
    }
    fputc('"', out);
}

static void dump_impl(const Value *v, FILE *out, int indent, int pretty) {
    switch (v->kind) {
        case JV_NULL:  fputs("null", out); break;
        case JV_TRUE:  fputs("true", out); break;
        case JV_FALSE: fputs("false", out); break;
        case JV_NUMBER: {
            char buf[64];
            snprintf(buf, sizeof(buf), "%.17g", v->u.num);
            fputs(buf, out);
            break;
        }
        case JV_STRING:
            dump_string_escaped(v->u.str.ptr, v->u.str.len, out);
            break;
        case JV_ARRAY: {
            fputc('[', out);
            if (v->u.arr.len) { if (pretty) fputc('\n', out); }
            for (size_t i=0;i<v->u.arr.len;i++) {
                if (pretty) indent_n(out, indent+2);
                dump_impl(v->u.arr.items[i], out, indent+2, pretty);
                if (i+1 < v->u.arr.len) fputc(',', out);
                if (pretty) fputc('\n', out);
            }
            if (v->u.arr.len && pretty) indent_n(out, indent);
            fputc(']', out);
            break;
        }
        case JV_OBJECT: {
            fputc('{', out);
            if (v->u.obj.len) { if (pretty) fputc('\n', out); }
            for (size_t i=0;i<v->u.obj.len;i++) {
                const JMember *m = &v->u.obj.items[i];
                if (pretty) indent_n(out, indent+2);
                dump_string_escaped(m->key.ptr, m->key.len, out);
                fputc(':', out);
                if (pretty) fputc(' ', out);
                dump_impl(m->value, out, indent+2, pretty);
                if (i+1 < v->u.obj.len) fputc(',', out);
                if (pretty) fputc('\n', out);
            }
            if (v->u.obj.len && pretty) indent_n(out, indent);
            fputc('}', out);
            break;
        }
        default:
            fputs("null", out);
    }
}

void dump_json(const Value *v, FILE *out)      { dump_impl(v, out, 0, 1); fputc('\n', out); }
void dump_json_compact(const Value *v, FILE *out){ dump_impl(v, out, 0, 0); fputc('\n', out); }
