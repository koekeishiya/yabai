#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

struct event_loop_item
{
    struct event_loop_item *next;
    struct event event;
};

struct event_loop
{
    bool is_running;
    pthread_t thread;
    sem_t *semaphore;
    struct memory_pool pool;
    struct event_loop_item *head;
    struct event_loop_item *tail;
};

bool event_loop_init(struct event_loop *event_loop);
bool event_loop_begin(struct event_loop *event_loop);
bool event_loop_end(struct event_loop *event_loop);
void event_loop_post(struct event_loop *event_loop, enum event_type type, void *context, int param1, volatile uint32_t *info);

#endif
