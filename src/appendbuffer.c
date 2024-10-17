#include <string.h>

#include "appendbuffer.h"

int AppendBuffer_append(struct AppendBuffer *ab, char *data, size_t length) {
    char *new = (char *) realloc(ab->data, ab->length + length);
    if (!new) {
        return -1;
    }

    memcpy(&new[ab->length], data, length);
    ab->data = new;
    ab->length += length;

    return 0;
}

void AppendBuffer_free(struct AppendBuffer *ab) {
    free(ab->data);
}