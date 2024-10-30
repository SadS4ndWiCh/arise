#ifndef __TCP_H
#define __TCP_H

#include <sys/socket.h>

/* Errors */

#define ESOCKOPT    0
#define EADDRBIND   1
#define ESOCKLISTEN 2

int createsock(const char *host, const char *port);
void *get_in_addr(struct sockaddr *addr);

#endif
