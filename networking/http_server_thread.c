#include "http_common.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <pthread.h>

void *thread_worker(void *arg) {
    int client_fd = *(int *)arg;

    free(arg);

    handle_client(client_fd);
    
    close(client_fd);

    return NULL;
}

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    int reuse = 1;

    if (
        setsockopt(
            listen_fd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &reuse,
            sizeof(reuse)
        ) < 0
    ) {
        perror("setsockopt");
        close(listen_fd);
        return 1;
    }

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

        int *client_fd_ptr = malloc(sizeof(int));
        *client_fd_ptr = client_fd;

        pthread_t thread;

        pthread_create(
            &thread,
            NULL,
            thread_worker,
            client_fd_ptr
        );

        pthread_detach(thread);
    }

    close(listen_fd);
    return 0;
}