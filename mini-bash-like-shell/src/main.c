#include "sig.h"
#include "common.h"
#include "parse.h"
#include <unistd.h>

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
        int code = 0;
        if (argv[1]) code = atoi(argv[1]);
        exit(code);
    }
    return false; /* 처리한 내장이 아니면 false */
}

static int run_external(char **argv) {
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }

    if (pid == 0) {
        restore_default_sigint();       /* 자식은 Ctrl+C에 종료 */
        execvp(argv[0], argv);          /* PATH 탐색 포함 */
        perror("execvp");               /* 도달 시 실패 */
        _exit(127);                     /* 관례: not found */
    }

    int status = 0;
    // 음수지만 EINTR이면 다시 시도, 음수면 아웃, 0이상이면.
    while (waitpid(pid, &status, 0) < 0) {
        if (errno == EINTR) continue;
        perror("waitpid");
        return 1;
    }
    if (WIFEXITED(status))   return WEXITSTATUS(status);
    if (WIFSIGNALED(status)) return 128 + WTERMSIG(status); /* bash 관례 */
    return 1;
}

int main(void) {
    ignore_sigint_in_parent();

    char *line = NULL;
    size_t cap = 0;
    int last_status = 0;

    for (;;) {
        print_prompt(last_status);

        ssize_t n = getline(&line, &cap, stdin);
        if (n < 0) {
            if (feof(stdin)) {
                fputc('\n', stderr);
                break;
            }
            if (errno == EINTR) continue;
            perror("getline"); break;
        }

        char *cmdline = str_trim(line);
        if (*cmdline == '\0') continue;

        StrVec argv; sv_init(&argv);
        if (tokenize(cmdline, &argv) != 0) {
            /* 파싱 오류: 메시지는 tokenize가 출력 */
            continue;
        } 
        if (argv.len == 0) continue;

        int rc = 0;
        if (!handle_builtin(argv.items, &rc)) {
            rc = run_external(argv.items);
        }
        last_status = rc;

    }
}