#ifndef SA_H
#define SA_H

#include "common.h"

#define PAYLOAD_STATUS_SUCCESS   0
#define PAYLOAD_STATUS_OUTDATED  1
#define PAYLOAD_STATUS_NO_ATTRIB 2
#define PAYLOAD_STATUS_CON_ERROR 3

int scripting_addition_check(void);
int scripting_addition_load(void);
bool scripting_addition_is_installed(void);
int scripting_addition_uninstall(void);
int scripting_addition_install(void);

#endif
