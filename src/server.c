#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "http.h"
#include "server.h"

int Server_init(struct Server *srv, uint32_t host, uint16_t port) {
    srv->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (srv->fd == -1) {
        return -1;
    }

    if (setsockopt(srv->fd, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)) == -1) {
        return -1;
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = { htonl(host) }
    };

    if (bind(srv->fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        return -1;
    }

    return 0;
}

int Server_listen(struct Server *srv) {
    if (listen(srv->fd, SERVER_MAX_QUEUE_CONN) == -1) {
        return -1;
    }

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len;

        int client_fd = accept(srv->fd, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_fd == -1) {
            fprintf(stderr, "ERROR: accept() failed to accept a new connection.\n");
            continue;
        }

        char request_raw[1024];

        if (recv(client_fd, request_raw, sizeof(request_raw) / sizeof(request_raw[0]), 0) == -1) {
            fprintf(stderr, "ERROR: recv() failed to receive client request payload.\n");
            continue;
        }

        struct HTTPRequest req = HTTPRequest_parse(request_raw);

        if (srv->on_request(req, client_fd) != 0) {
            fprintf(stderr, "ERROR: on_request() failed.\n");
            continue;
        }

        close(client_fd);
    }
}