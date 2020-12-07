#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <memory.h>
#include <string.h>

int main(int argc, char **argv) {
  struct addrinfo hint;
  struct addrinfo *peers;

  memset(&hint, 0, sizeof(hint));
  hint.ai_socktype = SOCK_DGRAM;

  const char *server = argv[1];
  const char *service = NULL;
  if (argc > 2) service = argv[2];
  int ret = getaddrinfo(server, service, (struct addrinfo*)&hint, &peers);
  if (ret != 0) {
    fprintf(stderr, "getaddrinfo failed\n");
    return -1;
  }

  int sock = socket(peers->ai_family, peers->ai_socktype, peers->ai_protocol);
  if (sock < 0) {
    perror("Unable to create socket\n");
    return -1;
  }

  while (1) {
    fd_set fds;
    FD_ZERO(&fds);
    // add socket to set
    FD_SET(sock, &fds);
    // add stdin to set
    FD_SET(0, &fds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    ret = select(sock+1, &fds, NULL, NULL, &timeout);
    if (ret < 0) {
      fprintf(stderr, "select failed with %d\n", ret);
      return -1;
    }

    for (int i = 0; i < sock+1; i++) {
      if (FD_ISSET(i, &fds)) {
        if (i == 0) {
          char line[1024];
          fgets(line, 1023, stdin);
          sendto(sock, line, strlen(line), MSG_DONTWAIT, peers->ai_addr, peers->ai_addrlen);
        }
        if (i == sock) {
          char data[1024];
          struct sockaddr_storage client_addr;
          socklen_t client_addr_len = sizeof(client_addr);
          ret = recvfrom(sock, data, 1023, 0, (struct sockaddr*)&client_addr, &client_addr_len);
          if (ret <= 0) {
            return -1;
          } else {
            data[ret+1] = 0;
            printf("len=%d %s",ret, data);
          }
        }
      }
    }
  }

  close(sock);
  return 0;
}

