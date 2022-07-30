#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <memory.h>
#include <netdb.h>
#include <string.h>
#include <sys/epoll.h>

void handle_client(int);
void handle_read_from(int);
void disconnect_client(int);
void send_all(const char *, int);

/* the fds for connected clients */
int clients[1024];
/* number of connected clients */
int num_clients = 0;
/* maximum number of clients */

/* max fd for select call */
int max_fd = -1;

int s_sock = -1; // server socket
int epollfd = -1;
int accept_connection(int sock);
int setup_connection();
int event_loop();

// prepare fd for eventloop
int event_loop_prepare(int fd);

#define MAX(a,b) ((a > b)? a : b)

int setup_connection(const char *hostname, const char *port) {
  struct addrinfo hints;
  struct addrinfo *addrs = NULL;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  int ret = getaddrinfo(hostname, port, &hints, &addrs);
  if (ret < 0) {
    fprintf(stderr, "Unable to get address\n");
    return -1;
  }

  s_sock = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
  if (s_sock < 0) {
    perror("Unable to create socket\n");
    fprintf(stderr, "Unable to create socket\n");
    return -1;
  }

  ret = bind(s_sock, addrs->ai_addr, addrs->ai_addrlen);
  if (ret < 0) {
    perror("Unable to bind");
    return -1;
  }

  ret = listen(s_sock, 10);
  if (ret < 0) {
    perror("Unable to listen");
    return -1;
  }

  printf("started server on %s port \n", hostname);
  event_loop_prepare(s_sock);
  return 0;
}

int event_loop_init() {
  epollfd = epoll_create1(0);
  return 0;
}

int event_loop_prepare(int fd) {
  epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = fd;
  max_fd = MAX(max_fd, fd);
  return epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

int event_lopp() {
  while (1) {
    fd_set fds;

    FD_ZERO(&fds);
    FD_SET(s_sock, &fds);
    for (int i = 0; i < num_clients; i++) {
      FD_SET(clients[i], &fds);
    }

    timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    int ret = select(max_fd+1, &fds, 0, 0, &timeout);
    if (ret < 0) {
      perror("select failed ");
      return -1;
    }
    // timeout occured continue
    if (ret == 0) {
      continue;
    }

    for (int i = 0; i <= max_fd; i++) {
      if (FD_ISSET(i, &fds)) {
        if (i == s_sock) {
          printf("accepting new connection\n");
          ret = accept_connection(s_sock);
          if (ret < 0) {
            fprintf(stderr,"can't accept connection\n");
            return -1;
          }
        } else {
          handle_read_from(i);
        }
      }
    }
  }
}


int main(int argc, char **argv) {
  setup_connection("localhost", "8080");
  event_loop();
  close(s_sock);
  return 0;
}

int accept_connection(int sock)
{
    struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int sc = accept(sock, (struct sockaddr*)&client_addr, &addr_len);
    if (sc < 0) {
      perror("Unable to accept connection");
      return -1;
    }

    // get address of peer
    char address[100];
    int ret = getnameinfo((struct sockaddr*)&client_addr, addr_len, address, sizeof(address), 0, 0, NI_NUMERICHOST);
    if (ret < 0) {
      perror("Unable to get peer address");
    } else {
      printf("connection from %s fd=%d\n", address, sc);
    }

    handle_client(sc);
    return 0;
}

/* handle new client connection */
void handle_client(int fd)
{
  max_fd = MAX(max_fd, fd);
  for (int i = 0; i < num_clients; i++) {
    if (clients[i] == fd) return;
  }
  clients[num_clients++] = fd;
}

/** file descriptor of the client */
void disconnect_client(int fd)
{
  printf("disconnecting client %d\n", fd);
  int i = 0;
  for (i = 0; i < num_clients; i++) {
    if (clients[i] == fd) {
      break;
    }
  }
  if (i < num_clients) {
    for (int j = i+1; j < num_clients; j++) {
      clients[j-1] = clients[j];
    }
    num_clients--;
  }
}

void handle_read_from(int fd) {
  char buf[1024];
  int ret = recv(fd, buf, sizeof(buf), 0);
  if (ret <= 0) {
    disconnect_client(fd);
    close(fd);
    return;
  }
  printf("read %d bytes from client %d\n", ret, fd);
  // send same message to all
  send_all(buf, ret);
}

void send_all(const char *msg, int msg_len) {
  for (int i = 0; i < num_clients; i++) {
    send(clients[i], msg, msg_len, MSG_DONTWAIT);
  }
}

