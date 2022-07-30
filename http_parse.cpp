#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

typedef enum parse_state {
  START,
  URL,
  HEADER,
  DATA,
  CHUNKED_DATA,
  MULTIPART_DATA
} parse_state;

typedef struct http_state {
  parse_state state;
  unsigned char *data;
  unsigned char *pos;
  ssize_t len;
} http;

void add_data();
