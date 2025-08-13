#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "read_file.h"
#include <errno.h>


char *my_strdup(const char *s) {
    if (s == NULL) return NULL;

    size_t n = strlen(s);

    char *p = (char *)malloc(n+1);
    if (!p) return NULL;

    memcpy(p, s, n+1);
    
    return p;
}

FileData read_file_all(const char *path) {

    FILE* fp;
    fp = fopen(path, "rb");

    FileData f1 = {0};

    if (fp == NULL) {
        f1.ok = 0;
        f1.len = 0;
        f1.buf = NULL;
        f1.err = my_strdup(strerror(errno));
        return f1;
    }
    
    char buffer[1024];

    size_t len = 0, cap = 0;
    char *storage = NULL;
    size_t bytesToRead;

    while ((bytesToRead = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        
        // 지금 현재 넣어야 할 거를 봤더니, cap을 초과한다?
        if (len + bytesToRead + 1 > cap) {
            size_t new_cap = cap ? cap : 8192;
            while (len + bytesToRead + 1 > new_cap) new_cap *= 2;
            char *p = realloc(storage, new_cap);
            if (!p) {
                int saved = errno;
                fclose(fp);
                free(storage);
                f1.ok = 0;
                f1.buf = NULL;
                f1.len = 0;
                f1.err = my_strdup(strerror(saved));
                return f1;
            }
            storage = p;
            cap = new_cap;
        }

        // cap을 초과하지 않는다? 그러면. 복사해주면 된다.
        // 기존 len만큼 offset을 주고.
        memcpy(storage + len, buffer, bytesToRead);
        len += bytesToRead;
    }

    if (ferror(fp)) {
        int saved = errno;
        fclose(fp);
        free(storage);
        f1.ok = 0;
        f1.err = my_strdup(strerror(saved));
        f1.len = 0;
        return f1;
    }

    if (!storage) {
        storage = malloc(1);
        if (!storage) {
            fclose(fp);
            f1.ok = 0;
            f1.buf = NULL;
            f1.len = 0;
            f1.err = my_strdup("out of memory");
            return f1;
        }
    }
    storage[len] = '\0';

    fclose(fp);

    f1.ok = 1;
    f1.buf = storage;
    f1.len = len;
    f1.err = NULL;

    return f1;
}

void free_file_data(FileData *fd) {
    free(fd -> buf);
    free(fd -> err);
}