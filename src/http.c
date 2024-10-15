#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "http.h"

struct HTTPRequest HTTPRequest_parse(char *raw) {
    struct HTTPRequest request;

    request.method = strtok(raw, HTTP_REQUEST_DELIM_SPACE);
    request.path = strtok(NULL, HTTP_REQUEST_DELIM_SPACE);
    request.version = strtok(NULL, HTTP_REQUEST_DELIM_SPACE);

    return request;
}