#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "http.h"
#include "server.h"

int on_request(struct HTTPRequest req, int client_fd) {
    char filepath[100] = ".";

    strcat(filepath, req.path);

    if (req.path[strlen(req.path) - 1] == '/') {
        strcat(filepath, "index.html");
    }

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        fprintf(stderr, "ERROR: fopen() failed to open the file: %s\n", filepath);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char header[100];
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");

    size_t header_size = strlen(header);
    char *response = (char *) malloc(file_size + header_size);
    strcat(response, header);

    char *file_buf = response + header_size;
    fread(file_buf, file_size, 1, fp);

    if (send(client_fd, response, file_size + header_size, 0) == -1) {
        fprintf(stderr, "ERROR: send() failed to send response to client.\n");
        return -1;
    }

    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Give the power to your shit files! ARISE!\n\n");
        printf("Usage:\n");
        printf("  %s <path>\n", argv[0]);

        return 0;
    }

    struct Server srv;
    if (Server_init(&srv, INADDR_LOOPBACK, 3000) == -1) {
        fprintf(stderr, "ERROR: Server_init() failed to initialize the server.\n");
        return 1;
    }

    srv.on_request = &on_request;

    if (Server_listen(&srv) == -1) {
        fprintf(stderr, "ERROR: Server_listen() failed to start listening for new connections.\n");
        return 1;
    }

    close(srv.fd);

    return 0;
}