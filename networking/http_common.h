#ifndef HTTP_COMMON_H
#define HTTP_COMMON_H

struct HttpRequest {
    char method[16];
    char path[256];
    char version[32];
};

char *get_content_type(char *file_path);

void send_403(int client_fd);

void send_405(int client_fd);

void build_file_path(
    char *url_path,
    char *file_path,
    int file_path_size
);

void send_file_response(
    int client_fd,
    char *file_path
);

void parse_request(
    char *buffer,
    struct HttpRequest *req
);

void handle_client(int client_fd);

#endif