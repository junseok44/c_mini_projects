#ifndef READ_FILE_H
#define READ_FILE_H

#include <stddef.h>

typedef struct {
    char *buf;
    size_t len;
    int ok;
    char *err;
} FileData;

FileData read_file_all(const char *path);

void free_file_data(FileData *fd);

#endif