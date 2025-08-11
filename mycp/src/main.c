#include <stdio.h>
#include <dirent.h>     // opendir, readdir, closedir
#include <stdlib.h>     // malloc, free, exit
#include <string.h>     // strcpy, strcat, strlen

#include <sys/stat.h>   // stat, mkdir, S_ISDIR, S_ISREG
#include <fcntl.h>      // open, O_RDONLY, O_WRONLY, etc.
#include <unistd.h>     // access, read, write, close

#include <errno.h>      // errno, strerror

typedef struct dirent Dirent;

// 파일 열기 -> 파일 일정 크기씩 읽기 -> write로 대상 파일에 쓰기 -> close로 마무리하기.
void copy_file(char *origin_file_dir, char *dest_file_dir) {

    int fd = open(origin_file_dir, O_RDONLY);

    if (fd < 0) {
        perror("open origin");
        return;
    }

    // open은 경로가 파일이어야 열 수 있다. 즉 dest_dir도 파일이라고 가정하는 것.
    int fd2 = open(dest_file_dir, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd2 <0) {
        perror("open dest");
        close(fd);
        return;
    }
    
    // 문제는 이거는 딱 99바이트만 읽고 있다는 것. 만약 파일이 99바이트를 넘긴다면?
    // 그리고 왜 fread fopen이 아니라 open, read를 쓴걸까?

    char buff[1024];

    ssize_t n;

    while ((n = read(fd, buff, sizeof(buff))) > 0) {
        if (write(fd2, buff, n) != n) {
            perror("write");
            break;
        }
    }

    if (n < 0) {
        perror("read");
    }

    close(fd);
    close(fd2);
}

void copy_dir(char *origin_dir_path, char *dest_dir_path) {

    DIR *origin_dir = opendir(origin_dir_path);

    if (origin_dir == NULL) {
        perror("열기 실패");
        return;
    }

    struct stat st;
    // 만약 대상 디렉토리가 없다면,
    if (stat(dest_dir_path, &st) == -1) {
        // 디렉토리를 일단 만든다.
        if (mkdir(dest_dir_path, 0755) == -1) {
            perror("대상 디렉토리 생성 실패");
            closedir(origin_dir);
            return;
        }
    }

    Dirent *dir_content;

    while ((dir_content = readdir(origin_dir)) != NULL) {
        if (strcmp(dir_content -> d_name, ".") == 0 || strcmp(dir_content -> d_name, "..") == 0) {
            continue;
        }
        // 왜 +2인가? 하나는 /, 하나는 null 문자 고려해서? 근데 그럼 null 문자는 어디서 들어가는가?
        char *origin_path = malloc(strlen(origin_dir_path) + strlen(dir_content -> d_name) + 2); 
        char *dest_path = malloc(strlen(dest_dir_path) + strlen(dir_content->d_name) + 2);
        
        strcpy(origin_path, origin_dir_path);
        strcat(origin_path, "/");
        strcat(origin_path, dir_content -> d_name);

        strcpy(dest_path, dest_dir_path);
        strcat(dest_path, "/");
        strcat(dest_path, dir_content -> d_name);

        struct stat info;

        if (stat(origin_path, &info) == -1) {
            perror("stat 실패");
            exit(1);
        }
    
        if (S_ISREG(info.st_mode)) {
            copy_file(origin_path, dest_path);
        } else if (S_ISDIR(info.st_mode)) {
            copy_dir(origin_path, dest_path);
        }

        free(origin_path);
        free(dest_path);
    }

    closedir(origin_dir);    
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "사용법: %s <원본> <대상>\n", argv[0]);
        return 1;
    }

    char *origin = argv[1];
    char *dest = argv[2];

    struct stat st_origin, st_dest;
    if (stat(origin, &st_origin) == -1) {
        perror("원본 없음");
        return 1;
    }

    int dest_exists = (stat(dest, &st_dest) == 0);

    // ---------- 원본이 파일인 경우 ----------
    if (S_ISREG(st_origin.st_mode)) {
        if (!dest_exists) {
            size_t len = strlen(dest);
            if (len > 0 && dest[len-1] == '/') {
                // 디렉토리 의도 → 만들고 그 안에 파일 복사
                if (mkdir(dest, 0755) == -1) {
                    perror("대상 디렉토리 생성 실패");
                    return 1;
                }
                char *origin_file_name = strrchr(origin, '/');
                origin_file_name = origin_file_name ? origin_file_name + 1 : origin;
                char *new_path = malloc(strlen(dest) + strlen(origin_file_name) + 1);
                sprintf(new_path, "%s%s", dest, origin_file_name);
                copy_file(origin, new_path);
                free(new_path);
            } else {
                // 그냥 파일 생성
                copy_file(origin, dest);
            }
        } else {
            if (S_ISDIR(st_dest.st_mode)) {
                char *origin_file_name = strrchr(origin, '/');
                origin_file_name = origin_file_name ? origin_file_name + 1 : origin;
                char *new_path = malloc(strlen(dest) + strlen(origin_file_name) + 2);
                sprintf(new_path, "%s/%s", dest, origin_file_name);
                copy_file(origin, new_path);
                free(new_path);
            } else {
                copy_file(origin, dest);
            }
        }
    }

    // ---------- 원본이 디렉토리인 경우 ----------
    else if (S_ISDIR(st_origin.st_mode)) {
        if (dest_exists && S_ISREG(st_dest.st_mode)) {
            fprintf(stderr, "오류: 디렉토리를 파일에 복사할 수 없습니다.\n");
            return 1;
        }
        if (!dest_exists) {
            if (mkdir(dest, 0755) == -1) {
                perror("대상 디렉토리 생성 실패");
                return 1;
            }
        }
        copy_dir(origin, dest);
    }

    else {
        fprintf(stderr, "지원하지 않는 파일 형식입니다.\n");
        return 1;
    }

    return 0;
}



