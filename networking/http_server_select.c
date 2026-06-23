#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX_CLIENTS 100

struct HttpRequest {
    char method[16];
    char path[256];
    char version[32];
};

void send_403(int client_fd) {
    char *invalid_method =
        "HTTP/1.1 403 Forbidden\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 14\r\n"
        "Connection: close\r\n"
        "\r\n"
        "403 Forbidden\n";

    write(client_fd, invalid_method, strlen(invalid_method));
}

void send_405(int client_fd) {
    char *invalid_method =
        "HTTP/1.1 405 Method Not Allowed\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 23\r\n"
        "Connection: close\r\n"
        "\r\n"
        "405 Method Not Allowed\n";

    write(client_fd, invalid_method, strlen(invalid_method));
}

void parse_request(char *buffer, struct HttpRequest *req) {
    sscanf(
        buffer,
        "%15s %255s %31s",
        req->method,
        req->path,
        req->version
    );
}

void build_file_path(char *url_path, char *file_path, int file_path_size) {
    if (strcmp(url_path, "/") == 0) {
        snprintf(file_path, file_path_size, "www/index.html");
    } else {
        snprintf(file_path, file_path_size, "www%s", url_path);
    }
}

char *get_content_type(char *file_path) {
    if (strstr(file_path, ".html") != NULL) {
        return "text/html";
    }
    if (strstr(file_path, ".css") != NULL) {
        return "text/css";
    }
    if (strstr(file_path, ".js") != NULL) {
        return "application/javascript";
    }
    if (strstr(file_path, ".png") != NULL) {
        return "image/png";
    }

    return "application/octet-stream";
}

void send_file_response(int client_fd, char *file_path) {

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

    struct stat file_info;
    fstat(file_fd, &file_info);

    int file_size = file_info.st_size;

    char *content_type = get_content_type(file_path);
    printf("[server] content_type = %s\n", content_type);

    char header[256];
    int header_len = snprintf(
        header,
        sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        content_type,
        file_size
    );

    write(client_fd, header, header_len);

    char buffer[4096];
    int n;

    while ((n = read(file_fd, buffer, sizeof(buffer))) > 0) {
        write(client_fd, buffer, n);
    }

    close(file_fd);
}

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