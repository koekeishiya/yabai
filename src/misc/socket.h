#ifndef SOCKET_H
#define SOCKET_H

#define FAILURE_MESSAGE "\x07"

static inline bool socket_write_bytes(int sockfd, char *message, int len)
{
    return send(sockfd, message, len, 0) != -1;
}

static inline bool socket_write(int sockfd, char *message)
{
    return send(sockfd, message, strlen(message), 0) != -1;
}

static inline bool socket_connect_un(int *sockfd, char *socket_path)
{
    struct sockaddr_un socket_address;
    socket_address.sun_family = AF_UNIX;

    *sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (*sockfd == -1) return false;

    snprintf(socket_address.sun_path, sizeof(socket_address.sun_path), "%s", socket_path);
    return connect(*sockfd, (struct sockaddr *) &socket_address, sizeof(socket_address)) != -1;
}

static inline void socket_wait(int sockfd)
{
    struct pollfd fds[] = {
        { sockfd, POLLIN, 0 }
    };

    char dummy[1];
    int bytes = 0;

    while (poll(fds, 1, -1) > 0) {
        if (fds[0].revents & POLLIN) {
            if ((bytes = recv(sockfd, dummy, 0, 0)) <= 0) {
                break;
            }
        }
    }
}

static inline void socket_close(int sockfd)
{
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
}

#endif
