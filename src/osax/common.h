#ifndef SA_COMMON_H
#define SA_COMMON_H

#define OSAX_VERSION                "2.1.10"

#define OSAX_ATTRIB_DOCK_SPACES     0x01
#define OSAX_ATTRIB_DPPM            0x02
#define OSAX_ATTRIB_ADD_SPACE       0x04
#define OSAX_ATTRIB_REM_SPACE       0x08
#define OSAX_ATTRIB_MOV_SPACE       0x10
#define OSAX_ATTRIB_SET_WINDOW      0x20
#define OSAX_ATTRIB_ANIM_TIME       0x40

#define OSAX_ATTRIB_ALL             (OSAX_ATTRIB_DOCK_SPACES | \
                                     OSAX_ATTRIB_DPPM | \
                                     OSAX_ATTRIB_ADD_SPACE | \
                                     OSAX_ATTRIB_REM_SPACE | \
                                     OSAX_ATTRIB_MOV_SPACE | \
                                     OSAX_ATTRIB_SET_WINDOW | \
                                     OSAX_ATTRIB_ANIM_TIME)

enum sa_opcode
{
    SA_OPCODE_HANDSHAKE             = 0x01,
    SA_OPCODE_SPACE_FOCUS           = 0x02,
    SA_OPCODE_SPACE_CREATE          = 0x03,
    SA_OPCODE_SPACE_DESTROY         = 0x04,
    SA_OPCODE_SPACE_MOVE            = 0x05,
    SA_OPCODE_WINDOW_MOVE           = 0x06,
    SA_OPCODE_WINDOW_OPACITY        = 0x07,
    SA_OPCODE_WINDOW_OPACITY_FADE   = 0x08,
    SA_OPCODE_WINDOW_LAYER          = 0x09,
    SA_OPCODE_WINDOW_STICKY         = 0x0A,
    SA_OPCODE_WINDOW_SHADOW         = 0x0B,
    SA_OPCODE_WINDOW_FOCUS          = 0x0C,
    SA_OPCODE_WINDOW_SCALE          = 0x0D,
    SA_OPCODE_WINDOW_SWAP_PROXY_IN  = 0x0E,
    SA_OPCODE_WINDOW_SWAP_PROXY_OUT = 0x0F,
    SA_OPCODE_WINDOW_ORDER          = 0x10,
    SA_OPCODE_WINDOW_ORDER_IN       = 0x11,
};

#endif
