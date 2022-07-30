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
  hint.ai_socktype = SOCK_STREAM;

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
  printf("value of socket is %d\n", sock);
  ret = connect(sock, peers->ai_addr, peers->ai_addrlen);
  if (ret < 0) {
    perror("Unable to connect to remote host\n");
    return -1;
  }
  freeaddrinfo(peers);

  printf("connected \n");

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
          send(sock, line, strlen(line), MSG_DONTWAIT);
        }
        if (i == sock) {
          char data[1024];
          ret = recv(sock, data, 1023, 0);
          if (ret <= 0) {
            return -1;
          } else {
            data[ret+1] = 0;
            printf("%s", data);
          }
        }
      }
    }
  }
  close(sock);
  return 0;
}
