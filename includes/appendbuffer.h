#ifndef __APPENDBUFFER_H
#define __APPENDBUFFER_H

#define APPEND_BUFFER_DEFAULT { NULL, 0 }

#include <stdlib.h>

struct AppendBuffer {
  char *data;
  size_t length;
};

int AppendBuffer_append(struct AppendBuffer *ab, char *data, size_t length);
void AppendBuffer_free(struct AppendBuffer *ab);

#endif