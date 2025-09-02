#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

int setup_listen_socket(uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        perror("socket"); exit(1);
    }

    int yes = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        perror("setsockopt"); exit(1);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));       
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1);
    }

    if (listen(fd, SOMAXCONN) < 0) {
        perror("listen");
        exit(1);
    }
    return fd;
}

static void handle_client_blocking(int cfd) {
    char buf[4096];
    for (;;) {
        ssize_t n = recv(cfd, buf, sizeof(buf), 0);
        if (n == 0) {
            break;
        } else if (n <0) {
            if (errno == EINTR) continue;
            perror("send");
            break;
        }

        ssize_t sent = 0;
        while (sent < n) {
            ssize_t m = send(cfd, buf + sent, (size_t)(n-sent), 0);
            if (m < 0) {
                if (errno == EINTR) continue;
                perror("send");
                return;
            } 
            sent += m;  
        }
    }
}

int main(int argc, char** argv) {

    signal(SIGPIPE, SIG_IGN);

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return 1;
    }

    uint16_t port = (uint16_t)atoi(argv[1]);    

    int lfd = setup_listen_socket(port);
    printf("[day1] listening on port %u\n", port);

    for (;;) {
        struct sockaddr_in cli;
        socklen_t len = sizeof(cli);

        int cfd = accept(lfd, (struct sockaddr*)&cli, &len);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            continue;
        }

        char ip[64];
        inet_ntop(AF_INET, &cli.sin_addr, ip, sizeof(ip));
        printf("client connected: %s:%d\n", ip, ntohs(cli.sin_port));
        
        handle_client_blocking(cfd);

        close(cfd);
        printf("client disconnected\n");
    }

}