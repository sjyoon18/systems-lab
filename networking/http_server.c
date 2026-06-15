#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

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

    
    while(1) {
        int client_fd = accept(listen_fd, NULL, NULL);

        char buffer[4096];

        int n = read(client_fd, buffer, sizeof(buffer));

        printf("\n[server] client CONNECTED\n");

        printf("\n-------REQUEST-------\n");
        write(1, buffer, n);
        printf("\n---------------------\n");

        char *response;

        if (strncmp(buffer, "GET / ", 6) == 0) {
            response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 12\r\n"
            "Connection: close\r\n"
            "\r\n"
            "Hello World\n";
        }
        else if (strncmp(buffer, "GET /cats ", 10) == 0) {
            response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 11\r\n"
            "Connection: close\r\n"
            "\r\n"
            "Hello Cats\n";
        }
        else if (strncmp(buffer, "GET /dogs ", 10) == 0) {
            response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 11\r\n"
            "Connection: close\r\n"
            "\r\n"
            "Hello Dogs\n";
        }
        else {
            response = 
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 14\r\n"
            "Connection: close\r\n"
            "\r\n"
            "404 Not Found\n";
        }

        write(client_fd, response, strlen(response));

        close(client_fd);
    }

    close(listen_fd);

    return 0;
}