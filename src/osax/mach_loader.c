#ifdef __x86_64__
#include "x86_64/mach_loader.c"
#elif __arm64__
#include "arm64/mach_loader.c"
#endif
