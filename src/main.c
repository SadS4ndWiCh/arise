#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "appendbuffer.h"

#define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

int setup_socket(void) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        fprintf(stderr, "[\x1b[31m-\x1b[m] %s:%d ERROR: socket() failed to create socket.\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "[\x1b[32m+\x1b[m] Socket file description created.\n");

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
        fprintf(stderr, "[\x1b[31m-\x1b[m] %s:%d ERROR: setsockopt() failed to set `SO_REUSEADDR` socket option.\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    if (setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))) {
        fprintf(stderr, "[\x1b[31m-\x1b[m] %s:%d ERROR: setsockopt() failed to set `SO_RCVTIMEO` socket option.\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(3000),
        .sin_addr = { htonl(INADDR_LOOPBACK) }
    };

    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        fprintf(stderr, "[\x1b[31m-\x1b[m] %s:%d ERROR: bind() failed to bind the address to socket.\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "[\x1b[32m+\x1b[m] Address binded to socket.\n");

    if (listen(server_fd, 10) == -1) {
        fprintf(stderr, "[\x1b[31m-\x1b[m] %s:%d ERROR: listen() failed to start listening to connections.\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "[\x1b[32m+\x1b[m] Start listening to connections.\n");

    return server_fd;
}

int accept_conn(int server_fd) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;

    return accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
}

char *read_request(int client_fd) {
    struct AppendBuffer request = APPEND_BUFFER_DEFAULT;
    char buf[1024] = {0};

    fprintf(stdout, "[\x1b[33m*\x1b[m] Reading request payload...\n");
    while (1) {
        ssize_t nbytes = recv(client_fd, buf, ARRAY_LEN(buf), 0);
        fprintf(stdout, "[\x1b[33m*\x1b[m] '%ld' bytes read.\n", nbytes);
        if (nbytes == 0 || nbytes == EOF) {
            fprintf(stdout, "[\x1b[33m*\x1b[m] End of request payload.\n");
            break;
        }

        if (AppendBuffer_append(&request, buf, nbytes) == -1) {
            fprintf(stderr, "[\x1b[31m-\x1b[m] %s:%d ERROR: AppendBuffer_append() failed to append to the buffer.\n", __FILE__, __LINE__);
            break;
        }

        if ((size_t) nbytes < ARRAY_LEN(buf)) {
            fprintf(stdout, "[\x1b[33m*\x1b[m] End of request payload.\n");
            break;
        }
    }

    fprintf(stdout, "[\x1b[32m+\x1b[m] Readed request payload.\n");

    return request.data;
}

char *get_request_path(char *request_raw) {
    strtok(request_raw, " ");

    return strtok(NULL, " ");
}

void request_path_to_file_path(char *request_path, char *fpath) {
    strcat(fpath, request_path);

    if (fpath[strlen(fpath) - 1] == '/') {
        strcat(fpath, "index.html");
    }
}

int main(int argc, char **argv) {
    // Setup socket connection
    int server_fd = setup_socket();

    while (1) {
        // Accept connection
        int client_fd = accept_conn(server_fd);
        if (client_fd == -1) {
            fprintf(stderr, "[\x1b[31m-\x1b[m] %s:%d ERROR: accept() failed to accept new connection.\n", __FILE__, __LINE__);
            continue;
        }

        fprintf(stdout, "[\x1b[32m+\x1b[m] New connecton stablished.\n");

        // Read request payload
        char *request_raw = read_request(client_fd);
        if (request_raw == NULL) {
            fprintf(stdout, "[\x1b[33m+\x1b[m] Closed the client connection due timeout.\n");
            close(client_fd);
            continue;
        }

        // Transform request path to file path
        char *request_path = get_request_path(request_raw);
        if (request_path == NULL) {
            fprintf(stderr, "[\x1b[31m-\x1b[m] %s:%d ERROR: strtok() failed to request path token.\n", __FILE__, __LINE__);
            close(client_fd);
            continue;
        }

        fprintf(stdout, "[\x1b[32m+\x1b[m] Get request path: %s\n", request_path);

        char fpath[128] = { '.' };
        request_path_to_file_path(request_path, fpath);
        fprintf(stdout, "[\x1b[32m+\x1b[m] Transform request path into file path: %s\n", fpath);

        // Open the requested file
        FILE *fp = fopen(fpath, "r");
        if (!fp) {
            fprintf(stderr, "[\x1b[31m-\x1b[m] %s:%d ERROR: fopen() failed to open requested file.\n", __FILE__, __LINE__);
            close(client_fd);
            continue;
        }

        fprintf(stdout, "[\x1b[32m+\x1b[m] Open the requested file.\n");

        // Get the file length
        fseek(fp, 0, SEEK_END);
        long fsize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        fprintf(stdout, "[\x1b[32m+\x1b[m] Get file size: %ld bytes\n", fsize);

        // Build the response header
        char res_header[128];
        sprintf(res_header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
        size_t res_header_len = strlen(res_header);

        // Read the file into request body
        char *response = (char *) malloc(res_header_len + fsize);
        char *response_body = response + res_header_len;

        strcpy(response, res_header);
        fread(response_body, fsize, 1, fp);

        fprintf(stdout, "[\x1b[32m+\x1b[m] Build the response: %ld bytes\n", res_header_len + fsize);

        // Send response
        if (send(client_fd, response, res_header_len + fsize, 0) == -1) {
            fprintf(stderr, "[\x1b[31m-\x1b[m] %s:%d ERROR: send() failed to send response.\n", __FILE__, __LINE__);
            free(response);
            close(client_fd);
            continue;
        }

        fprintf(stdout, "[\x1b[32m+\x1b[m] Sended the response to the client.\n");

        free(response);
        close(client_fd);
    }

    close(server_fd);

    return 0;
}