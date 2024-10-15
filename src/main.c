#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "http.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Give the power to your shit files! ARISE!\n\n");
        printf("Usage:\n");
        printf("  %s <path>\n", argv[0]);

        return 0;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        fprintf(stderr, "ERROR: socket() failed to create the file descriptor.\n");
        return 1;
    }

    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        fprintf(stderr, "ERROR: setsockopt() failed to set `SO_REUSEADDR` socket option.\n");
        return 1;
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(3000),
        .sin_addr = { htonl(INADDR_LOOPBACK) }
    };

    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        fprintf(stderr, "ERROR: bind() failed to bind the socket to local address.\n");
        return 1;
    }

    if (listen(server_fd, 10) == -1) {
        fprintf(stderr, "ERROR: listen() failed to start listen for connections.\n");
        return 1;
    }

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len;

        int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_fd == -1) {
            fprintf(stderr, "ERROR: connect() failed to stable connection with client.\n");
            continue;
        }

        char request[1024] = {0};
        size_t request_len = sizeof(request) / sizeof(request[0]);

        if (recv(client_fd, request, request_len, 0) == -1) {
            fprintf(stderr, "ERROR: recv() receives the request payload.\n");
            continue;
        }

        struct HTTPRequest req = HTTPRequest_parse(request);
        char filepath[100] = { '.' };

        strcat(filepath, req.path);

        if (req.path[strlen(req.path) - 1] == '/') {
            strcat(filepath, "index.html");
        }

        printf("Requested File: %s\n", filepath);

        close(client_fd);
    }

    close(server_fd);

    return 0;
}