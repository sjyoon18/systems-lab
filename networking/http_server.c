#include "http_common.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

void reap_children(int sig) {
    (void)sig;
    while(waitpid(-1, NULL, WNOHANG) > 0) {
    }
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

    printf("Server listening on port 8080...\n");

    while(1) {
        int client_fd = accept(listen_fd, NULL, NULL);

        pid_t pid = fork();

        if (pid == 0) {
            close(listen_fd);

            handle_client(client_fd);
            
            close(client_fd);
            exit(0);
        }
        else {
            close(client_fd);
        }
    }

    close(listen_fd);
    return 0;
}