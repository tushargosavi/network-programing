#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define SOCK_FILE "sock_file"
int main(int argc, char **argv) {
  int sock;
  struct sockaddr_un my_addr;
  char buf[1024];

  sock = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (sock < 0) { perror("sock"); return sock; }

  my_addr.sun_family = AF_UNIX;
  strcpy(my_addr.sun_path, SOCK_FILE);
  if (bind(sock, (const sockaddr*)&my_addr, sizeof(struct sockaddr_un))==-1) {
    perror("Unable to bind");
    exit(-errno);
  }

  printf("socket --> %s\n", SOCK_FILE);
  if (read(sock, buf, 1024) < 0) {
    perror("Unable to read data");
    return -errno;
  }

  printf("msg %s\n", buf);
  close(sock);
  unlink(SOCK_FILE);
  return 0;
}


