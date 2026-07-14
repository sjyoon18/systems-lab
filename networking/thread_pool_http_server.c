#include "http_common.h"
#include "thread_pool.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

static volatile sig_atomic_t stop_server = 0;

static void handle_sigint(int signal_number) {
    (void)signal_number;
    stop_server = 1;
}

static void handle_client_job(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);

    handle_client(client_fd);
    close(client_fd);
}

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));

    listen(listen_fd, 5);

    printf("Server listening on port 8080...\n");

    struct thread_pool pool;

    if (thread_pool_init(&pool, 4) != 0) {
        fprintf(stderr, "Failed to initialize thread pool\n");
        close(listen_fd);
        return 1;
    }

    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_sigint;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGINT, &action, NULL) < 0) {
        perror("sigaction");
        thread_pool_destroy(&pool);
        close(listen_fd);
        return 1;
    }

    while(!stop_server) {
        int client_fd = accept(listen_fd, NULL, NULL);

        if (client_fd < 0) {
            if (errno == EINTR && stop_server) {
                break;
            }
            perror("accept");
            continue;
        }

        int *client_fd_ptr = malloc(sizeof(int));

        if (client_fd_ptr == NULL) {
            close(client_fd);
            continue;
        }

        *client_fd_ptr = client_fd;

        if (thread_pool_submit(&pool, handle_client_job, client_fd_ptr) != 0) {
            close(client_fd);
            free(client_fd_ptr);
        }
    }

    close(listen_fd);
    thread_pool_destroy(&pool);
    return 0;
}