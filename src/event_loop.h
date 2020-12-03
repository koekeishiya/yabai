#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#define EVENT_POOL_SIZE KILOBYTES(128)

struct queue_item
{
    struct event *data;
    struct queue_item *next;
};

struct event_loop
{
    bool is_running;
    pthread_t thread;
    sem_t *semaphore;
    struct memory_pool pool;
    struct queue_item *head;
    struct queue_item *tail;
};

bool event_loop_init(struct event_loop *event_loop);
bool event_loop_begin(struct event_loop *event_loop);
bool event_loop_end(struct event_loop *event_loop);
void event_loop_post(struct event_loop *event_loop, enum event_type type, void *context, int param1, volatile uint32_t *info);

#endif
