#ifndef MACROS_H
#define MACROS_H

#define array_count(a) (sizeof((a)) / sizeof(*(a)))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define DIR_NORTH   0
#define DIR_EAST   90
#define DIR_SOUTH 180
#define DIR_WEST  270

#endif
