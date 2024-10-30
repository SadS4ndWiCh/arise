#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "tcp.h"

int createsock(const char *host, const char *port) {
    errno = 0;

    struct addrinfo hints, *servinfo, *curr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    // hints.ai_flags    = AI_PASSIVE;

    int status = getaddrinfo(host, port, &hints, &servinfo);
    if (status != 0) {
        return -1;
    }

    int sockfd;
    for (curr = servinfo; curr != NULL; curr = curr->ai_next) {
        if ((sockfd = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol)) == -1) {
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
            close(sockfd);
            errno = ESOCKOPT;
            return -1;
        }

        if (bind(sockfd, (struct sockaddr *) curr->ai_addr, curr->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }

       break;
    }

    freeaddrinfo(servinfo);

    if (curr == NULL) {
        errno = EADDRBIND;
        return -1;
    }

    if (listen(sockfd, 10) == -1) {
        errno = ESOCKLISTEN;
        return -1;
    }

    return sockfd;
}

void *get_in_addr(struct sockaddr *addr) {
    if (addr->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)addr)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)addr)->sin6_addr);
}
