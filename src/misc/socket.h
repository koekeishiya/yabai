#ifndef SOCKET_H
#define SOCKET_H

#define SOCKET_DAEMON_HANDLER(name) void name(char *message, int length, int sockfd)
typedef SOCKET_DAEMON_HANDLER(socket_daemon_handler);

#define FAILURE_MESSAGE "\x07"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include <poll.h>

struct daemon
{
    int sockfd;
    bool is_running;
    pthread_t thread;
    socket_daemon_handler *handler;
};

char *socket_read(int sockfd, int *len);
bool socket_write_bytes(int sockfd, char *message, int len);
bool socket_write(int sockfd, char *message);
bool socket_connect_in(int *sockfd, int port);
bool socket_connect_un(int *sockfd, char *socket_path);
void socket_wait(int sockfd);
void socket_close(int sockfd);
bool socket_daemon_begin_in(struct daemon *daemon, int port, socket_daemon_handler *handler);
bool socket_daemon_begin_un(struct daemon *daemon, char *socket_path, socket_daemon_handler *handler);
void socket_daemon_end(struct daemon *daemon);

#endif
