#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <pthread.h>

int request_count = 0;
pthread_mutex_t count_lock;

struct HttpRequest {
    char method[16];
    char path[256];
    char version[32];
};

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

void build_file_path(char *url_path, char *file_path, int file_path_size) {
    if (strcmp(url_path, "/") == 0) {
        snprintf(file_path, file_path_size, "www/index.html");
    } else {
        snprintf(file_path, file_path_size, "www%s", url_path);
    }
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

void parse_request(char *buffer, struct HttpRequest *req) {
    sscanf(
        buffer,
        "%15s %255s %31s",
        req->method,
        req->path,
        req->version
    );
}

void handle_client(int client_fd) {
    char buffer[4096];

    int n = read(client_fd, buffer, sizeof(buffer));

    if (n <= 0) {
        return;
    }

    struct HttpRequest req;

    parse_request(buffer, &req);

    pthread_mutex_lock(&count_lock);
    request_count++;
    printf("\n[stats] request_count = %d\n", request_count);
    pthread_mutex_unlock(&count_lock);


    printf(
        "[request] method = %s; path = %s; version = %s\n",
        req.method,
        req.path,
        req.version
    );

    if (strcmp(req.method, "GET") != 0) {
        send_405(client_fd);
        return;
    }

    if (strstr(req.path, "..") != NULL) {
        send_403(client_fd);
        return;
    }
    
    char file_path[512];

    build_file_path(req.path, file_path, sizeof(file_path));
    send_file_response(client_fd, file_path);
}

void *thread_worker(void *arg) {
    int client_fd = *(int *)arg;

    free(arg);

    handle_client(client_fd);
    
    close(client_fd);

    return NULL;
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

    pthread_mutex_init(&count_lock, NULL);

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