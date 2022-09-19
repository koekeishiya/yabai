#ifndef MACROS_H
#define MACROS_H

#define KILOBYTES(value) ((value) * 1024ULL)
#define MEGABYTES(value) (KILOBYTES(value) * 1024ULL)
#define GIGABYTES(value) (MEGABYTES(value) * 1024ULL)

#define array_count(a) (sizeof((a)) / sizeof(*(a)))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define add_and_clamp_to_zero(a, b) (((a) + (b) <= 0) ? 0 : (a) + (b))
#define in_range_ii(a, b, c) (((a) >= (b)) && ((a) <= (c)))
#define in_range_ie(a, b, c) (((a) >= (b)) && ((a) <  (c)))
#define in_range_ei(a, b, c) (((a) >  (b)) && ((a) <= (c)))
#define in_range_ee(a, b, c) (((a) >  (b)) && ((a) <  (c)))
#define lerp(a, t, b) (((1.0-t)*a) + (t*b))

#define FAILURE_MESSAGE "\x07"

#define MAXLEN 512

#define REGEX_MATCH_UD  0
#define REGEX_MATCH_YES 1
#define REGEX_MATCH_NO  2

#define DIR_NORTH 360
#define DIR_EAST   90
#define DIR_SOUTH 180
#define DIR_WEST  270

#define STACK     111

#define TYPE_ABS 0x1
#define TYPE_REL 0x2

#define HANDLE_TOP    0x01
#define HANDLE_BOTTOM 0x02
#define HANDLE_LEFT   0x04
#define HANDLE_RIGHT  0x08
#define HANDLE_ABS    0x10

#define LAYER_BELOW   kCGBackstopMenuLevelKey
#define LAYER_NORMAL  kCGNormalWindowLevelKey
#define LAYER_ABOVE   kCGFloatingWindowLevelKey

#endif
