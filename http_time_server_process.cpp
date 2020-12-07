#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

void process_client(int socket, struct sockaddr *caddr);

int main(int argc, char **argv)
{
    struct addrinfo hint;
    hint.ai_family = AF_INET6;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_addr;

    // Disable sigpipe signal
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    int ret = getaddrinfo(0, "8080", &hint, &bind_addr);
    if (ret < 0)
    {
        perror("getaddrinfo failed\n");
        return -errno;
    }

    int sock = socket(bind_addr->ai_family, bind_addr->ai_socktype, bind_addr->ai_protocol);
    if (sock < 0)
    {
        perror("Unable to create socket with familly");
        return -errno;
    }

    // allow both IPV4 and IPV6 connections
    int option = 0;
    ret = setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&option, sizeof(option));

    ret = bind(sock, bind_addr->ai_addr, bind_addr->ai_addrlen);
    if (ret < 0)
    {
        perror("Unable to bind");
        return -errno;
    }

    ret = listen(sock, 10);
    if (ret < 0)
    {
        perror("listen failed\n");
        return -errno;
    }

    struct sockaddr_storage client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (true)
    {
        int socket_client = accept(sock, (struct sockaddr *)&client_addr, &client_len);
        if (socket_client < 0)
        {
            perror("Unable to accept the connection\n");
            goto error;
        }
        else
        {
            char address_buffer[100];
            getnameinfo((struct sockaddr *)&client_addr, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
            printf("client connected from %s\n", address_buffer);
            int cpid = fork();
            if (cpid == 0)
            {
                close(sock);
                process_client(socket_client, (struct sockaddr *)&client_addr);
                exit(0);
            } else {
                // close socket in parent, let child process handle it
                close(socket_client);
            }
        }
    }

    return 0;
error:
    return -errno;
}

void process_client(int fd, struct sockaddr *caddr)
{
    // do nothing for now, just close the socket
    // read http read requst
    char buffer[1024];
    int ret = recv(fd, &buffer, 1024, 0);
    if (ret < 0)
    {
        close(fd);
        return;
    }

    // write http response with current system time
    const char resp[] = "HTTP/1.1 200 OK\r\n"
                        "Connection: close\r\n"
                        "Content-Type: text/plain\r\n\r\n"
                        "Local time is: ";

    ret = send(fd, resp, sizeof(resp) - 1, 0);
    if (ret != sizeof(resp) - 1)
    {
        close(fd);
        return;
    }

    time_t timer;
    time(&timer);
    char *time_str = ctime(&timer);
    send(fd, time_str, strlen(time_str), 0);
    close(fd);
}