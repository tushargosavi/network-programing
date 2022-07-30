#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct http_header {
  char *key;
  char *value;
} http_header;

// maximum number of headers supported
#define MAX_LINE_LEN 1024 * 1024
#define MAX_HEADERS 255
#define MAX_HEADER_REQ_BUF 1024 * 1024

typedef enum parse_state {
  HTTP_H = 0,
  HTTP_T1,
  HTTP_T2,
  HTTP_P,
  HTTP_SLASH,
  HTTP_1,
  HTTP_DOT,
  HTTP_2,
  HTTP_SPACE,
  HTTP_URI_SPACE,
  HTTP_URI_START,
  HTTP_URI,
  HTTP_PROTOCOL,
  HTTP_VERSION,
  HTTP_START_HEADER,
  HTTP_HEADER,
  HTTP_HEADER_VAL,
  HTTP_BODY,
} parse_state;

typedef struct http_parse_state {
  parse_state state;

  int len; // data consumed till now
  unsigned char str[MAX_LINE_LEN]; // buffer holding current unprocessed data
  int n;

  char url[1024];
  int url_len;

  char version[10];
  int version_len;

  char *protocol;
  int retcode;
  char *desc;

  http_header headers[MAX_HEADERS];
  char *cur_header; // current header key being parsed
  char *cur_header_value; // current header value being parse

  unsigned char *body;
  unsigned char body_len;

  unsigned int chunked;
  unsigned int current_chunk_len;

  int num_headers;
} http_parse_state;

void add_header(struct http_parse_state* state, char *name, char *value) {
  http_header header = { name, value };
  state->headers[state->num_headers++] = header;
  if (strcmp(name, "Content-Length") == 0) {
    state->body_len = atoi(value);
  }
}

int add_line(char *line, int len, char **data, int *datalen, int *done) {
  char *dest = &line[len];
  int i = 0;
  *done = 0;
  while (i < *datalen && *data[i] != '\r') { line[i] = *data[i]; i++; }
  i++;
  if (i < *datalen && *data[i] == '\n') {
    *done = 1;
  }

  *data = data[i];
  *datalen = *datalen - i;
  return 0;
}

/**
 * Add data to the HTTP server state
 **/
void add_header_data(struct http_parse_state* state, unsigned char *data, int len) {
  int ps = state->state;
  int i = 0;
  while (i < len) {
    unsigned char c = data[i];
  }
}

inline int expect(unsigned char c, unsigned char e, int next) {
  return (e == c)? next : -1;
}

int consume_char(struct http_parse_state* s, unsigned char c) {
  switch (s->state) {
    case HTTP_H: s->state = (parse_state)expect(c, 'H', HTTP_T1); break;
    case HTTP_T1: s->state = (parse_state)expect(c, 'T', HTTP_T2); break;
    case HTTP_T2: s->state = (parse_state)expect(c, 'T', HTTP_P); break;
    case HTTP_P: s->state = (parse_state)expect(c, 'P', HTTP_P); break;
    case HTTP_SPACE: s->state = (parse_state)expect(c, ' ', HTTP_URI); break;
    case HTTP_URI: {
      if (c != ' ' || c != '\n') s->url[s->url_len++] = c;
      s->url[s->url_len] = 0;
      s->state = HTTP_URI_SPACE;
    }
    case HTTP_URI_SPACE: s->state = (parse_state)expect(c, ' ', HTTP_VERSION); break;
    case HTTP_VERSION:
      if (c != ' ' || c != '\n') s->version[s->version_len++] = c; break;
      s->version[s->version_len] = 0;
      s->state = HTTP_HEADER;
    case HTTP_HEADER:
      if (c == '\n') s->state = HTTP_BODY;
      if (c != ' ' && c != '=') s->cur_key[s->cur_key++]=c; break;
      s->cur_key[s->cur_key_len] = 0;
      s->state = HTTP_HEADER_VAL;
    case HTTP_HEADER_VAL:
      if (c != '\n') s->cur_val[c->cur_val_len++] = c ; break;
      s->state = HTTP_HEADER;
    
        
    break;
    default:
      printf("Invalide state");
  }
}

void expect() {

}

void print_http_resp(http_parse_state *state) {
  
}

