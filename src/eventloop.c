#include "eventloop.h"

static void
queue_init(struct queue *queue)
{
    queue->head = malloc(sizeof(struct queue_item));
    queue->head->data = NULL;
    queue->head->next = NULL;
    queue->tail = queue->head;
};

static void
queue_push(struct queue *queue, struct event *event)
{
    bool success;
    struct queue_item *tail, *new_tail;

    new_tail = malloc(sizeof(struct queue_item));
    new_tail->data = event;
    new_tail->next = NULL;
    __asm__ __volatile__ ("" ::: "memory");

    do {
        tail = queue->tail;
        success = __sync_bool_compare_and_swap(&tail->next, NULL, new_tail);
        if (!success) {
            __sync_bool_compare_and_swap(&queue->tail, tail, tail->next);
        }
    } while (!success);
    __sync_bool_compare_and_swap(&queue->tail, tail, new_tail);
}

static struct event *
queue_pop(struct queue *queue)
{
    struct queue_item *head, *next;

    do {
        head = queue->head;
        if (!head->next) {
            return NULL;
        }
    } while (!__sync_bool_compare_and_swap(&queue->head, head, head->next));

    next = head->next;
    free(head);

    return next->data;
}

static void *
eventloop_run(void *context)
{
    struct eventloop *eventloop = (struct eventloop *) context;
    struct queue *queue = (struct queue *) &eventloop->queue;

    while (eventloop->is_running) {
        struct event *event = queue_pop(queue);
        if (event) {
            int result = event->handler(event->context, event->param1, event->param2);
            if (event->status) *event->status = EVENT_PROCESSED;
            if (event->result) *event->result = result;
            free(event);
        } else {
            sem_wait(eventloop->semaphore);
        }
    }

    return NULL;
}

void eventloop_post(struct eventloop *eventloop, struct event *event)
{
    if (eventloop->is_running) {
        queue_push(&eventloop->queue, event);
        sem_post(eventloop->semaphore);
    } else if (event->status) {
        *event->status = EVENT_IGNORED;
        free(event);
    }
}

bool eventloop_init(struct eventloop *eventloop)
{
    queue_init(&eventloop->queue);
    eventloop->is_running = 0;
    eventloop->semaphore = sem_open("eventloop_semaphore", O_CREAT, 0600, 0);
    return eventloop->semaphore != SEM_FAILED;
}

bool eventloop_begin(struct eventloop *eventloop)
{
    if (eventloop->is_running) return false;
    eventloop->is_running = true;
    pthread_create(&eventloop->thread, NULL, &eventloop_run, eventloop);
    return true;
}

bool eventloop_end(struct eventloop *eventloop)
{
    if (!eventloop->is_running) return false;
    eventloop->is_running = false;
    pthread_join(eventloop->thread, NULL);
    return true;
}
