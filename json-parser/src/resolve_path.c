
#include "value.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static Value* resolve_path(Value *root, const char *path) {
    
    if (!root || !path) return NULL;
    Value *cur = root;
    const char *p = path;

    while (*p) {
        if (*p == '.') { p++; continue; }

        const char *kstart = p;
        while (*p && *p != '.' && *p != '[') p++;

        if (p > kstart) {
            size_t klen = (size_t)(p - kstart);
            char *k = (char *) malloc(klen + 1);
            // p가 점에 있거나, 아니면 index거나 할테니까.
            if (!k) return NULL;;
            memcpy(k, kstart, klen);
            k[klen] = '\0';

            cur = jv_get_object(cur, k);
            free(k);
            if (!cur) return NULL;
        }

        while (*p == '[') {
            p++;
            if (!isdigit((unsigned char)*p)) return NULL;
            size_t idx = 0;
            while (isdigit((unsigned char) *p)) {
                idx = idx * 10 + (size_t)(*p - '0');
                p++;
            }
            if (*p != ']') return NULL;
            p++;

            cur = jv_get_array(cur, idx);
            if (!cur) return NULL;
        }

        if (*p == '.') {
            p++;
            continue;
        }
        if (*p == '\0') break;

        return NULL;
    }
    return cur;
}