#ifndef EVENT_SIGNAL_H
#define EVENT_SIGNAL_H

struct signal
{
    char *app;
    char *title;
    bool app_regex_valid;
    bool title_regex_valid;
    bool app_regex_exclude;
    bool title_regex_exclude;
    regex_t app_regex;
    regex_t title_regex;
    char *command;
    char *label;
};

void event_signal_transmit(void *context, enum event_type type);
void event_signal_add(enum event_type type, struct signal *signal);
void event_signal_destroy(struct signal *signal);
bool event_signal_remove_by_index(int index);
bool event_signal_remove(char *label);
void event_signal_list(FILE *rsp);
enum event_type event_signaltype_from_string(const char *str);

#endif
