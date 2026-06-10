#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_add;

    memset(&server_add, 0, sizeof(server_add));

    server_add.sin_family = AF_INET;
    server_add.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_add.sin_addr);

    connect(sockfd, (struct sockaddr *)&server_add, sizeof(server_add));

    write(sockfd, "Hello from client\n", 18);

    close(sockfd);

    return 0;
}

