#ifndef __HTTPH
#define __HTTPH

#define HTTP_REQUEST_DELIM_SPACE " "
#define HTTP_REQUEST_DELIM_LINE "\r\n"

struct HTTPRequest {
    char *method;
    char *path;
    char *version;
};

/*
    Parse the HTTP Request.

    Receives the raw request payload and parse into `HTTPRequest` struct.

    A basic HTTP Request payload is:

    ```
    GET /index.html HTTP/1.1\r\n
    Accept: text/html\r\n
    User-Agent: Mozilla/5.0\r\n
    ```
*/
struct HTTPRequest HTTPRequest_parse(char *raw);

#endif