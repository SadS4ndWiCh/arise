#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#include "tcp.h"
#include "process.h"

void handle_connection(int clientfd) {
    // read the request payload
    char request_buf[1024];
    if (recv(clientfd, request_buf, sizeof(request_buf) / sizeof(request_buf[0]), 0) == -1) {
        fprintf(stderr, "ERROR: failed to read request payload\n");
        return;
    }

    // get the request path
    strtok(request_buf, " ");
    char *request_path = strtok(NULL, " ");

    // translate the request path into file path
    char filepath[255] = {'.'};
    strcat(filepath, request_path);

    if (filepath[strlen(filepath) - 1] == '/') {
        strcat(filepath, "index.html");
    }

    // read requested file
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        char *notfound = "HTTP/1.1 404 Not Found\r\n";
        if (send(clientfd, notfound, strlen(notfound), 0) == -1) {
            fprintf(stderr, "ERROR: failed to reply with `Not Found`\n");
        }

        return;
    }

    // get file size
    fseek(fp, 0L, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    // build response payload
    char *response_line = "HTTP/1.1 200 OK\r\n\r\n";
    size_t response_line_len = strlen(response_line);

    char *response_body = (char *) malloc(fsize);
    if (fread(response_body, 1, fsize, fp) == -1) {
        free(response_body);

        fprintf(stderr, "ERROR: failed to read file\n");
        return;
    }

    char *response = (char *) malloc(response_line_len + fsize);
    strcpy(response, response_line);
    strcpy(&response[response_line_len], response_body);

    // send response
    if (send(clientfd, response, response_line_len + fsize, 0) == -1) {
        fprintf(stderr, "ERROR: failed to send response\n");
        return;
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: arise <port>");
        exit(0);
    }

    int serverfd = createsock("localhost", argv[1]);

    // Reap all dead proccesses
    reap_processes();

    printf("Server is waiting for connections...\n");

    while (1) {
        struct sockaddr_storage clientaddr;
        socklen_t clientaddr_len = sizeof(clientaddr);

        int clientfd = accept(serverfd, (struct sockaddr *) &clientaddr, &clientaddr_len);
        if (clientfd == -1) {
            fprintf(stderr, "ERROR: failed to accept connection\n");
            continue;
        }

        char ipbuf[INET6_ADDRSTRLEN];
        inet_ntop(clientaddr.ss_family, get_in_addr((struct sockaddr *) &clientaddr), ipbuf, sizeof(ipbuf));

        printf("INFO: got connection from %s\n", ipbuf);

        if (!fork()) {
            close(serverfd);

            handle_connection(clientfd);

            close(clientfd);
            exit(0);
        }

        close(clientfd);
    }

    close(serverfd);
    return 0;
}
