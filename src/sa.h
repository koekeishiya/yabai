#ifndef SA_H
#define SA_H

#define PAYLOAD_STATUS_SUCCESS   0
#define PAYLOAD_STATUS_OUTDATED  1
#define PAYLOAD_STATUS_NO_ATTRIB 2
#define PAYLOAD_STATUS_CON_ERROR 3

int scripting_addition_check(void);
int scripting_addition_load(void);
bool scripting_addition_is_installed(void);
int scripting_addition_uninstall(void);
int scripting_addition_install(void);

bool scripting_addition_create_space(uint64_t sid);
bool scripting_addition_destroy_space(uint64_t sid);
bool scripting_addition_focus_space(uint64_t sid);
bool scripting_addition_move_space_after_space(uint64_t src_sid, uint64_t dst_sid, bool focus);
bool scripting_addition_move_window(uint32_t wid, int x, int y);
bool scripting_addition_set_opacity(uint32_t wid, float opacity, float duration);
bool scripting_addition_set_layer(uint32_t wid, int layer);
bool scripting_addition_set_sticky(uint32_t wid, bool sticky);
bool scripting_addition_set_shadow(uint32_t wid, bool shadow);
bool scripting_addition_focus_window(uint32_t wid);
bool scripting_addition_scale_window(uint32_t wid, float x, float y, float w, float h);

extern bool workspace_is_macos_monterey(void);
extern bool workspace_is_macos_bigsur(void);
extern bool workspace_is_macos_highsierra(void);

extern unsigned char __src_osax_mach_bootstrap[];
extern unsigned int __src_osax_mach_bootstrap_len;
extern unsigned char __src_osax_loader[];
extern unsigned int __src_osax_loader_len;
extern unsigned char __src_osax_payload[];
extern unsigned int __src_osax_payload_len;
extern unsigned char __src_osax_mach_loader[];
extern unsigned int __src_osax_mach_loader_len;

#endif
