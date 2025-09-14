#include "sig.h"
#include "common.h"
#include "parse.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>   // strcmp 추가
#include <stdlib.h>   // getenv, exit, atoi 추가
#include <stdbool.h>  // bool 타입 추가

extern char **environ; // env 출력용

// --- 기존 함수 (수정 없음) ---

static void print_prompt(int last_status) {
    char cwd[4096];
    const char *where = getcwd(cwd, sizeof(cwd)) ? cwd : "?";
    if (last_status == 0)
        fprintf(stderr, "[%s] mini$ ", where);
    else
        fprintf(stderr, "[%s] mini(%d)$ ", where, last_status);
    fflush(stderr);
}

static bool handle_builtin(char **argv, int *retcode) {
    if (!argv || !argv[0]) return false;
    if (strcmp(argv[0], "exit") == 0) {
        int code = (argv[1]) ? atoi(argv[1]) : 0;
        *retcode = code;
        exit(code);
    }
    if (strcmp(argv[0], "pwd") == 0) {
        char cwd[4096];
        if (getcwd(cwd, sizeof(cwd))) {
            puts(cwd); *retcode = 0;
        } else {
            perror("pwd"); *retcode = 1;
        }
        return true;
    }
    if (strcmp(argv[0], "cd") == 0) {
        const char *dir = argv[1] ? argv[1] : getenv("HOME");
        if (chdir(dir) < 0) {
            perror("cd"); *retcode = 1;
        } else {
            *retcode = 0;
        }
        return true;
    }
    if (strcmp(argv[0], "env") == 0) {
        for (char **p = environ; p && *p; ++p) puts(*p);
        *retcode = 0;
        return true;
    }
    return false;
}

static int run_external(char **argv) {
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }
    if (pid == 0) {
        restore_default_sigint();
        execvp(argv[0], argv);
        perror("execvp");
        _exit(127);
    }
    int status = 0;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno == EINTR) continue;
        perror("waitpid");
        return 1;
    }
    if (WIFEXITED(status))   return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
    return 1;
}

// --- 추가된 실행 로직 ---

// 파이프(|)를 처리하는 함수
static int run_pipe(char **argv, int pipe_idx) {
    argv[pipe_idx] = NULL;
    char **left_cmd = argv;
    char **right_cmd = &argv[pipe_idx + 1];

    int pfd[2];
    if (pipe(pfd) < 0) { perror("pipe"); return 1; }

    pid_t pid1 = fork();
    if (pid1 < 0) { perror("fork"); return 1; }
    if (pid1 == 0) { // 첫 번째 자식 (왼쪽 명령어)
        restore_default_sigint();
        close(pfd[0]);
        dup2(pfd[1], STDOUT_FILENO);
        close(pfd[1]);
        execvp(left_cmd[0], left_cmd);
        perror("execvp (left)");
        _exit(127);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) { perror("fork"); return 1; }
    if (pid2 == 0) { // 두 번째 자식 (오른쪽 명령어)
        restore_default_sigint();
        close(pfd[1]);
        dup2(pfd[0], STDIN_FILENO);
        close(pfd[0]);
        execvp(right_cmd[0], right_cmd);
        perror("execvp (right)");
        _exit(127);
    }

    close(pfd[0]);
    close(pfd[1]);
    int status1, status2;
    waitpid(pid1, &status1, 0);
    waitpid(pid2, &status2, 0);

    if (WIFEXITED(status2)) return WEXITSTATUS(status2);
    return 1;
}

// 리디렉션(>, <)을 처리하는 함수
static int run_redirect(char **argv, int redirect_idx) {
    char *op = argv[redirect_idx];
    char *filename = argv[redirect_idx + 1];
    if (!filename) {
        fprintf(stderr, "syntax error: no filename for redirection\n");
        return 1;
    }
    argv[redirect_idx] = NULL;

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }
    if (pid == 0) {
        restore_default_sigint();
        int fd = -1;
        if (strcmp(op, ">") == 0) {
            fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd >= 0) dup2(fd, STDOUT_FILENO);
        } else if (strcmp(op, "<") == 0) {
            fd = open(filename, O_RDONLY);
            if (fd >= 0) dup2(fd, STDIN_FILENO);
        }
        
        if (fd < 0) { perror("open"); _exit(1); }
        close(fd);

        execvp(argv[0], argv);
        perror("execvp (redirect)");
        _exit(127);
    }
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return 1;
}

// 명령어 종류를 분석하고 적절한 실행 함수를 호출하는 '지휘자'
static int run_command(char **argv) {
    if (!argv || !argv[0]) return 0;

    for (size_t i = 0; argv[i]; ++i) {
        if (strcmp(argv[i], "|") == 0) {
            return run_pipe(argv, i);
        }
        if (strcmp(argv[i], ">") == 0 || strcmp(argv[i], "<") == 0) {
            return run_redirect(argv, i);
        }
    }
    return run_external(argv);
}


// --- 수정된 main 함수 ---

int main(void) {
    ignore_sigint_in_parent();

    char *line = NULL;
    size_t cap = 0;
    int last_status = 0;

    for (;;) {
        print_prompt(last_status);

        ssize_t n = getline(&line, &cap, stdin);
        if (n < 0) {
            if (feof(stdin)) { fputc('\n', stderr); break; }
            if (errno == EINTR) continue;
            perror("getline"); break;
        }

        char *cmdline = str_trim(line);
        if (*cmdline == '\0') continue;

        StrVec argv;
        sv_init(&argv);
        // tokenize 후 NULL 종단까지 포함해 원소가 1개 이하면 빈 명령어
        if (tokenize(cmdline, &argv) != 0 || argv.len <= 1) {
            sv_free_all(&argv); // 실패하거나 빈 입력이면 메모리 해제
            continue;
        }

        int rc = 0;
        if (!handle_builtin(argv.items, &rc)) {
            // [변경점] run_external 대신 새로운 지휘자 함수 호출
            rc = run_command(argv.items);
        }
        last_status = rc;
        
        sv_free_all(&argv); // 루프 끝에서 argv 메모리 해제
    }

    free(line); // 프로그램 종료 전 getline 버퍼 해제
    return last_status;
}