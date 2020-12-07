#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

const char msg[] = "This is sample message";
const char *msg1 = "This is message 2";

int main(int argc, char **argv) {
  int fds[2], ret;
  int child;

  ret = pipe(fds);
  if (ret < 0) {
    fprintf(stderr, "Can not create pipe");
    return ret;
  }

  child = fork();
  if (child < 0) {
    return child;
  }

  if (child) {
    char data[1024];
    close(fds[1]);
    read(fds[0], data, 1024);
    printf("parent message from client %s\n", data);
    close(fds[0]);
  } else {
    close(fds[0]);
    write(fds[1], msg, strlen(msg) + 1);
    close(fds[1]);
  }

  return 0;
}


