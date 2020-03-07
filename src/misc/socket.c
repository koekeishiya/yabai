#include "socket.h"

char *socket_read(int sockfd, int *len)
{
    int cursor = 0;
    int bytes_read = 0;
    char *result = NULL;
    char buffer[BUFSIZ];

    while ((bytes_read = read(sockfd, buffer, sizeof(buffer)-1)) > 0) {
        char *temp = realloc(result, cursor+bytes_read+1);
        if (!temp) goto err;

        result = temp;
        memcpy(result+cursor, buffer, bytes_read);
        cursor += bytes_read;
    }

    if (result && bytes_read != -1) {
        result[cursor] = '\0';
        *len = cursor;
    } else {
err:
        if (result) free(result);
        result = NULL;
        *len = 0;
    }

    return result;
}

bool socket_write_bytes(int sockfd, char *message, int len)
{
    return send(sockfd, message, len, 0) != -1;
}

bool socket_write(int sockfd, char *message)
{
    return send(sockfd, message, strlen(message), 0) != -1;
}

bool socket_connect_in(int *sockfd, int port)
{
    struct sockaddr_in socket_address;

    *sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (*sockfd == -1) return false;

    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons(port);
    socket_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    memset(&socket_address.sin_zero, '\0', 8);

    return connect(*sockfd, (struct sockaddr*) &socket_address, sizeof(struct sockaddr)) != -1;
}

bool socket_connect_un(int *sockfd, char *socket_path)
{
    struct sockaddr_un socket_address;
    socket_address.sun_family = AF_UNIX;

    *sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (*sockfd == -1) return false;

    snprintf(socket_address.sun_path, sizeof(socket_address.sun_path), "%s", socket_path);
    return connect(*sockfd, (struct sockaddr *) &socket_address, sizeof(socket_address)) != -1;
}

void socket_wait(int sockfd)
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

void socket_close(int sockfd)
{
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
}

static void *socket_connection_handler(void *context)
{
    struct daemon *daemon = context;

    while (daemon->is_running) {
        int sockfd = accept(daemon->sockfd, NULL, 0);
        if (sockfd == -1) continue;

        int length;
        char *message = socket_read(sockfd, &length);
        if (message) {
            daemon->handler(message, length, sockfd);
        } else {
            socket_close(sockfd);
        }
    }

    return NULL;
}

bool socket_daemon_begin_in(struct daemon *daemon, int port, socket_daemon_handler *handler)
{
    struct sockaddr_in socket_address;
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons(port);
    socket_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    memset(&socket_address.sin_zero, '\0', 8);

    if ((daemon->sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        return false;
    }

    if (bind(daemon->sockfd, (struct sockaddr *) &socket_address, sizeof(socket_address)) == -1) {
        return false;
    }

    if (listen(daemon->sockfd, SOMAXCONN) == -1) {
        return false;
    }

    daemon->handler = handler;
    daemon->is_running = true;
    pthread_create(&daemon->thread, NULL, &socket_connection_handler, daemon);

    return true;
}

bool socket_daemon_begin_un(struct daemon *daemon, char *socket_path, socket_daemon_handler *handler)
{
    struct sockaddr_un socket_address;
    socket_address.sun_family = AF_UNIX;
    snprintf(socket_address.sun_path, sizeof(socket_address.sun_path), "%s", socket_path);
    unlink(socket_path);

    if ((daemon->sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        return false;
    }

    if (bind(daemon->sockfd, (struct sockaddr *) &socket_address, sizeof(socket_address)) == -1) {
        return false;
    }

    if (chmod(socket_path, 0600) != 0) {
        return false;
    }

    if (listen(daemon->sockfd, SOMAXCONN) == -1) {
        return false;
    }

    daemon->handler = handler;
    daemon->is_running = true;
    pthread_create(&daemon->thread, NULL, &socket_connection_handler, daemon);

    return true;
}

void socket_daemon_end(struct daemon *daemon)
{
    daemon->is_running = false;
    pthread_join(daemon->thread, NULL);
    socket_close(daemon->sockfd);
}
