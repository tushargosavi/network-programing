#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/un.h>

#define SOCK_FILE "sock_file"

#define DATA "This is message to server"

int main(int argc, char **argv) {
  int sock;
  struct sockaddr_un addr;

  sock = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (sock < 0) return sock;
  addr.sun_family = AF_UNIX;
  strcpy(addr.sun_path, argv[1]);
  if (sendto(sock, DATA, sizeof(DATA), 0,
        (const struct sockaddr*)&addr, sizeof(struct sockaddr_un)) < 0) {
    perror("sending message");
  }
  close(sock);
}

