#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <memory.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char **argv) {

  struct addrinfo hints;
  struct addrinfo *addrs;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  int ret = getaddrinfo(NULL, argv[1], &hints, &addrs);
  if (ret < 0) {
    fprintf(stderr, "Unable to get address\n");
    return -1;
  }

  int sock = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
  if (sock < 0) {
    perror("Unable to create socket\n");
    fprintf(stderr, "Unable to create socket\n");
    return -1;
  }

  ret = bind(sock, addrs->ai_addr, addrs->ai_addrlen);
  if (ret < 0) {
    perror("Unable to bind");
    return -1;
  }

  printf("started up server on %s port \n", argv[1]);

  while (1) {
    char buffer[1024];
    sockaddr_storage addr;
    socklen_t addr_len = sizeof(addr);
    ret = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, &addr_len);
    if (ret <= 0) {
     return -1;
    }
    // add null at the end of the string
    buffer[ret] = 0;
    printf("data received len=%d data=%s", ret, buffer);

    for (int i = 0; i < ret; i++) buffer[i] = toupper(buffer[i]);
    sendto(sock, buffer, ret, MSG_DONTWAIT, (struct sockaddr*)&addr, addr_len);
  }

  close(sock);
  return 0;
}
