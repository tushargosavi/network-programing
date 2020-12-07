#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

char msg1[] = "This is message from parent to child";
char msg2[] = "Thiks is message from child to parent";

int main(int argc, char **argv) {
  int socks[2], ret, child;
  char data[1024];

  ret = socketpair(AF_UNIX, SOCK_STREAM, 0, socks);
  if (ret < 0) return ret;

  child = fork();
  if (child < 0) return child;
  if (child) {
    write(socks[1], msg1, sizeof(msg1));
    read(socks[1], data, 1024);
    printf("parent msg %s\n", data);
  } else {
    read(socks[0], data, 1024);
    printf("child msg %s\n", data);
    write(socks[0], msg2, sizeof(msg2));
  }

  close(socks[0]);
  return 0;
}



