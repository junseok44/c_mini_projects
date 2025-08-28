#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include "read_file.h"
#include "lexer_next.h"
#include "parser.h"
#include "dump.h"
#include "value.h"

static void print_help(const char* prog) {
    printf("jsoncli - JSON parser\n");
    printf("usage:\n");
    printf("  %s --help\n", prog);
    printf("  %s parse <file.json>\n", prog);
    printf("  %s get <file.json> <path>\n", prog);
}

// path 예: foo.bar[2].baz  / arr[0][1] / root
static Value* resolve_path(Value *root, const char *path) {
    if (!root || !path) return NULL;
    Value *cur = root;
    const char *p = path;

    while (*p) {
        // 선택적으로 선행 '.' 스킵
        if (*p == '.') { p++; continue; }

        // 1) 키 세그먼트 읽기: [ '.' / '[' / '\0' ] 전까지
        const char *kstart = p;
        while (*p && *p != '.' && *p != '[') p++;

        if (p > kstart) {
            size_t klen = (size_t)(p - kstart);
            char *k = (char*)malloc(klen + 1);
            if (!k) return NULL;
            memcpy(k, kstart, klen);
            k[klen] = '\0';

            cur = jv_get_object(cur, k);
            free(k);
            if (!cur) return NULL;
        }

        // 2) 연속된 [index] 처리 (예: arr[0][1])
        while (*p == '[') {
            p++; // '['
            if (!isdigit((unsigned char)*p)) return NULL;
            size_t idx = 0;
            while (isdigit((unsigned char)*p)) {
                idx = idx * 10 + (size_t)(*p - '0');
                p++;
            }
            if (*p != ']') return NULL;
            p++; // ']'

            cur = jv_get_array(cur, idx);
            if (!cur) return NULL;
        }

        if (*p == '.') { p++; continue; }
        if (*p == '\0') break;

        // 알 수 없는 문자가 나오면 실패
        return NULL;
    }
    return cur;
}

int main(int argc, char **argv) {

    if (argc < 2) {
        print_help(argv[0]);
        return 2;
    }

    if (strcmp(argv[1], "--help") == 0) {
       print_help(argv[0]);
        return 0;
    }
    else if (strcmp(argv[1], "parse") == 0) {
        if (argc < 3) {
            fprintf(stderr, "usage: %s parse <file.json>\n", argv[0]);
            return 2;
        }

        const char *path = argv[2];

        FileData fd = read_file_all(path);

        if (!fd.ok) {
            fprintf(stderr, "file error: %s: %s\n", path, fd.err ? fd.err : "unknown");
            free_file_data(&fd);
            return 2;
        }

        Lexer lx = lexer_init(fd.buf);

        Parser p;

        parser_init(&p);
        p.lx = &lx;

        Value *root = jv_new();

        if (!parser_parse(&p, root)) {
            fprintf(stderr, "parse error @%d:%d: %s\n",
                parser_error_line(&p), parser_error_col(&p), parser_error_msg(&p));
            jv_free(root);
            free_file_data(&fd);
            return 1;
        }

        dump_json(root, stdout);

        jv_free(root);
        free_file_data(&fd);

        return 0;
    } else if (strcmp(argv[1], "get") == 0) {

        if (argc < 4) {
            print_help(argv[0]);
            return 2;
        }

        const char *path = argv[2];
        const char *json_path_expr = argv[3];

        FileData fd = read_file_all(path);

        if (!fd.ok) {
            fprintf(stderr, "file error: %s: %s\n", path, fd.err ? fd.err : "unknown");
            free_file_data(&fd);
            return 2;
        }

        Lexer lx = lexer_init(fd.buf);

        Parser p = {0};
        parser_init(&p);
        p.lx = &lx;

        Value *root = jv_new();

        if (!root) {
            fprintf(stderr, "oom\n");
            free_file_data(&fd);
            return 1;
        }

        if (!parser_parse(&p, root)) {
            fprintf(stderr, "parse error @%d:%d: %s\n",
                parser_error_line(&p), parser_error_col(&p), parser_error_msg(&p));
            jv_free(root);
            free_file_data(&fd);
            return 1;
        }

        // ---- 경로 해석 ----
        Value *target = resolve_path(root, json_path_expr);

        if (!target) {
            fprintf(stderr, "path not found: %s\n", json_path_expr);
            jv_free(root);
            free_file_data(&fd);
            return 1;
        }

        dump_json_compact(target, stdout);

        jv_free(root);
        free_file_data(&fd);
        
        return 0;
    } else {
        fprintf(stderr, "unknown command: %s\n", argv[1]);
        return 2;
    }
}

