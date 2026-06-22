#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <errno.h>

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

            printf("client CONNECTED: fd = %d\n", client_fd);

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

                int n = read(client_fd, buffer, sizeof(buffer));
                
                if (n < 0) {
                    perror("read");

                    close(client_fd);
                    clients[i] = -1;
                }
                else if (n == 0) {
                    printf("client DISCONNECTED: fd = %d\n", client_fd);

                    close(client_fd);
                    clients[i] = -1;
                }
                else {
                    printf("\n[client_fd = %d]:\n", client_fd);
                    write(1, buffer, n);

                    char *response =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 12\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "Hello World\n";

                    write(client_fd, response, strlen(response));

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