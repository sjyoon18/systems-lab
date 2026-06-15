#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include<sys/wait.h>

void reap_children(int sig) {
    while(waitpid(-1, NULL, WNOHANG) > 0) {
    }
}

void send_file_response(int client_fd, char *file_path) {
    char body[4096];
    
    int file_fd = open(file_path, O_RDONLY);

    if (file_fd < 0) {
        char *not_found =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 14\r\n"
        "Connection: close\r\n"
        "\r\n"
        "404 Not Found\n";

        write(client_fd, not_found, strlen(not_found));
        return;
    }

    int body_len = read(file_fd, body, sizeof(body));
    
    close(file_fd);

    char header[256];
    int header_len = snprintf(
        header,
        sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        body_len
    );

    write(client_fd, header, header_len);
    write(client_fd, body, body_len);
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

             char buffer[4096];

            int n = read(client_fd, buffer, sizeof(buffer));

            printf("\n[server] client CONNECTED\n");

            printf("-------REQUEST-------\n");
            write(1, buffer, n);
            printf("\n---------------------\n");

            if (strncmp(buffer, "GET / ", 6) == 0) {
                send_file_response(client_fd, "www/index.html");
            }
            else if (strncmp(buffer, "GET /cats ", 10) == 0) {
                send_file_response(client_fd, "www/cats.html");
            }
            else if (strncmp(buffer, "GET /dogs ", 10) == 0) {
                send_file_response(client_fd, "www/dogs.html");
            }
            else {
                send_file_response(client_fd, "nofile");
            }

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