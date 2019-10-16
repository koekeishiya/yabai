#ifndef MESSAGE_H
#define MESSAGE_H

struct token
{
    char *text;
    unsigned int length;
};

static SOCKET_DAEMON_HANDLER(message_handler);
void handle_message(FILE *rsp, char *message);

#endif
