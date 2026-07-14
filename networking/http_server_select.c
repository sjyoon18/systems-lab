#include "http_common.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

#define MAX_CLIENTS 100

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return 1;
    }

    if (listen(listen_fd, 5) < 0) {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    printf("select server listening on port 8080...\n");

    int clients[MAX_CLIENTS];

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = -1;
    }

    while(1) {
        fd_set read_fds;

        FD_ZERO(&read_fds);
        FD_SET(listen_fd, &read_fds);

        int max_fd = listen_fd;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] != -1) {
                FD_SET(clients[i], &read_fds);

                if (clients[i] > max_fd) {
                    max_fd = clients[i];
                }
            }
        }

        int ready = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

        if (ready < 0) {
            perror("select");
            continue;
        } 

        if(FD_ISSET(listen_fd, &read_fds)) {
            int client_fd = accept(listen_fd, NULL, NULL);

            if (client_fd < 0) {
                perror("accept");
                continue;
            }

            printf("\nclient CONNECTED: fd = %d\n", client_fd);

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] == -1) {
                    clients[i] = client_fd;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int client_fd = clients[i];

            if (client_fd != -1 && FD_ISSET(client_fd, &read_fds)) {
                char buffer[4096];

                int n = read(client_fd, buffer, sizeof(buffer) - 1);
                
                if (n < 0) {
                    perror("read");

                    close(client_fd);
                    clients[i] = -1;
                }
                else if (n == 0) {
                    printf("\nclient DISCONNECTED: fd = %d\n", client_fd);

                    close(client_fd);
                    clients[i] = -1;
                }
                else {
                    printf("[client_fd = %d]:\n", client_fd);
                    
                    struct HttpRequest req;

                    buffer[n] = '\0';

                    parse_request(buffer, &req);

                    printf(
                        "method = %s; path = %s; version = %s\n",
                        req.method,
                        req.path,
                        req.version
                    );

                    if (strcmp(req.method, "GET") != 0) {
                        send_405(client_fd);

                        printf("client DISCONNECTED: fd = %d\n", client_fd);
                        
                        close(client_fd);
                        clients[i] = -1;
                        continue;
                    }

                    if (strstr(req.path, "..") != NULL) {
                        send_403(client_fd);

                        printf("client DISCONNECTED: fd = %d\n", client_fd);
                        
                        close(client_fd);
                        clients[i] = -1;
                        continue;
                    }

                    char file_path[512];

                    build_file_path(req.path, file_path, sizeof(file_path));

                    send_file_response(client_fd, file_path);

                    printf("client DISCONNECTED: fd = %d\n", client_fd);
                    
                    close(client_fd);
                    clients[i] = -1;
                }
            }
        }
    }

    close(listen_fd);
    return 0;
}