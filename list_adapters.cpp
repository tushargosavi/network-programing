#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main() {
  struct ifaddrs *addresses;
  int ret = 0;

  ret = getifaddrs(&addresses);
  if (ret < 0) {
    perror("getifaddrs call failed\n");
    fprintf(stderr, "getifaddrs call failed with %d\n", ret);
    return -1;
  }

  struct ifaddrs *addr = addresses;
  while (addr) {
    int family = addr->ifa_addr->sa_family;

    if (family == AF_INET || family == AF_INET6) {
    printf("%s\t", addr->ifa_name);
    printf("%s\t", family == AF_INET? "IPv4" : "IPv6");

    char ap[100];
    const int family_size = family == AF_INET ?
      sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
    getnameinfo(addr->ifa_addr,
        family_size, ap, sizeof(ap), 0, 0, NI_NUMERICHOST);
    printf("\t%s\n", ap);
  }
    addr = addr->ifa_next;
  }
}


