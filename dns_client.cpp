#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <memory.h>
#include <string.h>

unsigned char dns_query[] = {
    0xAB, 0xCD,                           /* ID */
    0x01, 0x00,                           /* Recursion */
    0x00, 0x01,                           /* QDCOUNT */
    0x00, 0x00,                           /* ANCOUNT */
    0x00, 0x00,                           /* NSCOUNT */
    0x00, 0x00,                           /* ARCOUNT */
    7, 'e', 'x', 'a', 'm', 'p', 'l', 'e', /* label */
    3, 'c', 'o', 'm',                     /* label */
    0,                                    /* End of name */
    0x00, 0x01,                           /* QTYPE = A */
    0x00, 0x01                            /* QCLASS */
};

typedef struct buffer {
  unsigned char *ptr;
  ssize_t len;
} buffer_t;

void print_buffer(unsigned char *buf, int len) {
  int i = 0;
  while (i < len) printf("0x%x ", buf[i++]);
  printf("\n");
}

/* print a segment in the name */
unsigned char *print_segment(unsigned char *p)
{
  if (*p) {
    unsigned int n = *p++;
    unsigned int i = 0;
    while(i++ < n) printf("%c", *p++);
  }
  return p;
}

/* the name starts at pointer p,
 * return the pointer to next byte after name */
unsigned char *print_name(unsigned char *s, unsigned char *p) {
  printf("NAME: ");
  while (*p) {
    // asuming there is only one name with pointer
    if ((*p & 0xC0) == 0xC0) {
      // the provided name is the pointer in the buffer
      int offset = (p[0] & 0x3F) << 8 | p[1]; p+=2;
      print_name(s, s + offset);
      return p;
    }
    //printf("len of segment %d %x\n", *p, *p);
    unsigned int n = *p++;
    unsigned int i = 0;
    while (i++ < n) printf("%c", *p++);
    if (*p) printf(".");
  }
  p++;
  return p;
}

unsigned char* print_query(unsigned char *s, unsigned char *e, unsigned char *p) {
  p = print_name(s, p);
  unsigned int qtype = p[0] << 8 | p[1]; p+=2;
  unsigned int qclass = p[0] << 8 | p[1]; p+=2;
  printf("\nQTYPE 0x%x QCLASS 0x%x\n", qtype, qclass);
  return p;
}

unsigned char * print_answer(unsigned char *s, unsigned char *e, unsigned char *p) {
  p = print_name(s, p);
  unsigned int type = p[0] << 8 | p[1]; p+=2;
  unsigned int qclass = p[0] << 8 | p[1]; p+=2;
  unsigned int ttl = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];  p+=4;
  unsigned int rdlength = p[0] << 8 | p[1]; p+=2;

  printf("\nQTYPE 0x%x QCLASS 0x%x\n", type, qclass);
  printf("\nTTL %d  RDLENG %d\n", ttl, rdlength);
  //TODO switch based on the type
  switch (type) {
    case 1: {
              printf("IP: %d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);
              p += 4;
            }
            break;
    default:
            p += rdlength;
  }
  return p;
}

void print_response(unsigned char *buf, size_t len) {
  if (len < 12) {
    printf("buffer is too small to contain dns packet\n");
    return;
  }

  printf("ID: 0x%x 0x%x\n", buf[0], buf[1]);

  printf("QR: %d Opcode %d AA %d TC %d RD %d\n",
      buf[2] & 0x80 >> 7,
      ((buf[2] & 0x78) >> 3),
      (buf[2] & 0x04) >> 2,
      (buf[2] & 0x02) >> 1,
      buf[2] & 0x01);

  printf("RA %d RCOCDE %d\n",
      buf[3] & 0x80 >> 7,
      buf[3] & 0x0f);

  int qdcount = buf[4] << 8 | buf[5];
  int ancount = buf[6] << 8 | buf[7];
  int nscount = buf[8] << 8 | buf[9];
  int arcount = buf[10] << 8 | buf[11];

  printf("QDCOUNT %d ANCOUNT %d NSCOUNT %d ARCOUNT %d\n",
      qdcount, ancount, nscount, arcount);

  /* In general there should be only one query, but standard supports
   * multiple queries, hence, for loop here */
  unsigned char *p = &buf[12];
  for (int i = 0; i < qdcount; i++) {
    printf("=== START Query %d ===\n", i+1);
    p = print_query(buf, buf+len, p);
    printf("=== END Query %d ===\n", i+1);
  }

  for (int i = 0; i < ancount; i++) {
    printf("=== START Answer %d ===\n", i);
    p = print_answer(buf, buf+len, p);
    printf("=== END Answer %d ===\n", i);
  }
}

/* create a dns packet requesting address for name, and type record */
buffer_t make_packet(const char *name, int type) {
  // stack allocated buffer
  unsigned char buf[1024];
  // current pointer where data will be written
  unsigned char *ptr = buf;
  // end pointer for checking buffer overflow
  unsigned char *end = buf + sizeof(buf);

  memset(buf, 0, sizeof(buf));

  *ptr++ = 0xAB; *ptr++ = 0xCD; // id for the packet
  *ptr++ = 0x01; *ptr++ = 0x00;
  *ptr++ = 0x00; *ptr++ = 0x01; // qd count 1
  *ptr++ = 0x00; *ptr++ = 0x00; // qd
  *ptr++ = 0x00; *ptr++ = 0x00; // ns
  *ptr++ = 0x00; *ptr++ = 0x00; // ar

  unsigned char *len = ptr;
  ptr++;
  int count = 0;
  while(*name) {
    if (*name == '.') {
      *len = (ptr - len) - 1;
      len = ptr;
      name++;
      ptr++;
    } else {
      *ptr++ = *name++;
    }
  }
  *len = (ptr - len) - 1;
  *ptr++ = 0x00; // a null terminating last name segment
  *ptr++ = 0x00; *ptr++ = 0x01;
  *ptr++ = 0x00; *ptr++ = 0x01;

  buffer_t rb;
  rb.len = ptr - buf;
  rb.ptr = (unsigned char*)malloc(rb.len);
  memcpy(rb.ptr, buf, rb.len);
  return rb;
}

int main(int argc, char **argv) {
  struct addrinfo hint;
  struct addrinfo *peers;
  //const char *dns_server = "8.8.8.8";
  const char *dns_server = "10.204.4.5";
  const char *dns_port = "53";
  socklen_t dns_port1 = 53;
  memset(&hint, 0, sizeof(hint));
  hint.ai_socktype = SOCK_DGRAM;

  const char *server = argv[1];
  const char *service = NULL;
  int ret = getaddrinfo(dns_server, dns_port, (struct addrinfo*)&hint, &peers);
  if (ret != 0) {
    fprintf(stderr, "getaddrinfo failed\n");
    return -1;
  }

  int sock = socket(peers->ai_family, peers->ai_socktype, peers->ai_protocol);
  if (sock < 0) {
    perror("Unable to create socket\n");
    return -1;
  }

  buffer_t query = make_packet(server, 1);
  print_buffer(query.ptr, query.len);

  ret = sendto(sock, query.ptr, query.len, 0, peers->ai_addr, peers->ai_addrlen);
  if (ret < 0) {
    printf("Unable to send dns query\n");
    return -1;
  }

  unsigned char buf[4096];
  struct sockaddr_storage client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  ret = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr*)&client_addr, &client_addr_len);
  if (ret < 0) {
    printf("unable to receive data \n");
    return -1;
  }

  printf("received data from server of len=%d\n", ret);
  print_buffer(buf, ret);
  print_response(buf, ret);
  close(sock);
  return 0;
}

