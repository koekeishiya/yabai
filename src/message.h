#ifndef MESSAGE_H
#define MESSAGE_H

void handle_message(FILE *rsp, char *message);
bool message_loop_begin(char *socket_path);

#endif
