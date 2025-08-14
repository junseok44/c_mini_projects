#include <stdio.h>
#include <string.h>

#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include "read_file.h"
#include "lexer_next.h"

static void print_help(const char* prog) {
    printf("jsoncli - JSON parser\n");
    printf("usage:\n");
    printf("  %s --help\n", prog);
    printf("  %s parse <file.json>\n", prog);
    printf("  %s get <file.json> <path>\n", prog);
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

        Token t;

        Lexer lx = lexer_init(fd.buf);
        
        while ((t = lexer_next(&lx)).kind != TK_EOF) {
            print_token(t);
        }

        free_file_data(&fd);

        return 0;
    } else if (strcmp(argv[1], "get") == 0) {

        if (argc < 4) {
            print_help(argv[0]);
            return 2;
        }

        const char *path = argv[2];
        FileData fd = read_file_all(path);

        if (!fd.ok) {
            fprintf(stderr, "file error: %s: %s\n", path, fd.err ? fd.err : "unknown");
            free_file_data(&fd);
            return 2;
        }

        printf("%s", fd.buf);
        free_file_data(&fd);
        
        return 0;
    } else {
        fprintf(stderr, "unknown command: %s\n", argv[1]);
        return 2;
    }
}

