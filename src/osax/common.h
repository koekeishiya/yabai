#ifndef SA_COMMON_H
#define SA_COMMON_H

#define OSAX_VERSION                "2.0.9"

#define OSAX_PAYLOAD_SUCCESS        0
#define OSAX_PAYLOAD_NOT_FOUND      1
#define OSAX_PAYLOAD_ALREADY_LOADED 2
#define OSAX_PAYLOAD_NOT_LOADED     3

#define OSAX_ATTRIB_DOCK_SPACES     0x01
#define OSAX_ATTRIB_DPPM            0x02
#define OSAX_ATTRIB_ADD_SPACE       0x04
#define OSAX_ATTRIB_REM_SPACE       0x08
#define OSAX_ATTRIB_MOV_SPACE       0x10
#define OSAX_ATTRIB_SET_WINDOW      0x20

#define OSAX_ATTRIB_ALL             (OSAX_ATTRIB_DOCK_SPACES | \
                                     OSAX_ATTRIB_DPPM | \
                                     OSAX_ATTRIB_ADD_SPACE | \
                                     OSAX_ATTRIB_REM_SPACE | \
                                     OSAX_ATTRIB_MOV_SPACE | \
                                     OSAX_ATTRIB_SET_WINDOW)

#endif
