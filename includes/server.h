#ifndef __SERVERH
#define __SERVERH

#include <stdlib.h>
#include <netinet/in.h>

#include "http.h"

#define SERVER_MAX_QUEUE_CONN 10

struct Server {
  int fd;
  struct sockaddr_in addr;

  int (*on_request) (struct HTTPRequest req, int client_fd);
};

int Server_init(struct Server *srv, uint32_t host, uint16_t port);
int Server_listen(struct Server *srv);

#endif