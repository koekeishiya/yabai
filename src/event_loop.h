#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#define EVENT_POOL_SIZE KILOBYTES(36)
#define EVENT_MAX_COUNT ((EVENT_POOL_SIZE) / (sizeof(struct event)))

#define QUEUE_POOL_SIZE KILOBYTES(16)
#define QUEUE_MAX_COUNT ((QUEUE_POOL_SIZE) / (sizeof(struct queue_item)))

struct queue_item
{
    struct event *data;
    struct queue_item *next;
};

struct queue
{
    struct memory_pool pool;
    struct queue_item *head;
    struct queue_item *tail;
#ifdef DEBUG
    volatile uint64_t count;
#endif
};

struct event_loop
{
    bool is_running;
    pthread_t thread;
    sem_t *semaphore;
    struct queue queue;
    struct memory_pool pool;
#ifdef DEBUG
    volatile uint64_t count;
#endif
};

bool event_loop_init(struct event_loop *event_loop);
bool event_loop_begin(struct event_loop *event_loop);
bool event_loop_end(struct event_loop *event_loop);
void event_loop_post(struct event_loop *event_loop, struct event *event);

#endif
