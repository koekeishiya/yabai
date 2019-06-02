#ifndef MACROS_H
#define MACROS_H

#define array_count(a) (sizeof((a)) / sizeof(*(a)))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define MAXLEN 256

#define REGEX_MATCH_UD  0
#define REGEX_MATCH_YES 1
#define REGEX_MATCH_NO  2

#define DIR_NORTH 360
#define DIR_EAST   90
#define DIR_SOUTH 180
#define DIR_WEST  270

#define HANDLE_TOP    0x1
#define HANDLE_BOTTOM 0x2
#define HANDLE_LEFT   0x4
#define HANDLE_RIGHT  0x8

#endif
