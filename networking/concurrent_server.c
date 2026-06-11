#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>

void reap_children(int sig) {
    while(waitpid(-1, NULL, WNOHANG) > 0) {
    }
}

void handle_client(int client_fd) {
    char buffer[100];
    int n;

    printf("[child %d] client CONNECTED\n", getpid());

    while((n = read(client_fd, buffer, sizeof(buffer))) > 0) {
        printf("[%d's client]: ", getpid());
        fflush(stdout);
        write(1, buffer, n);
        write(client_fd, buffer, n);
    }

    printf("[child %d] client DISCONNECTED\n", getpid());
}

int main() {
    signal(SIGCHLD, reap_children);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(listen_fd, 5);

    printf("[parent %d] listening on port 8080...\n", getpid());

    while(1) {
        int client_fd = accept(listen_fd, NULL, NULL);
        printf("[parent %d] client accepted\n", getpid());

        pid_t pid = fork();

        if (pid == 0) {
            close(listen_fd);
            handle_client(client_fd);
            close(client_fd);
            exit(0);
        }

        close(client_fd);
    }

    close(listen_fd);
    return 0;
}