#include "event_loop.h"

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
event_loop_run(void *context)
{
    struct event_loop *event_loop = (struct event_loop *) context;
    struct queue *queue = (struct queue *) &event_loop->queue;

    while (event_loop->is_running) {
        struct event *event = queue_pop(queue);
        if (event) {
            int result = event_handler[event->type](event->context, event->param1, event->param2);
            if (result == EVENT_SUCCESS) event_signal_transmit(event->context, event->type);

            if (event->result) *event->result = result;

            /*
             * NOTE(koekeishiya): We REQUIRE the result to be updated BEFORE the event is marked as processed,
             * because the calling thread should be allowed to spin-lock on the passed variable to halt until
             * the event has been processed. This is because it may be useful or even necessary to check the
             * result of the event before proceeding. Emit mfence for this purpose.
             * */

            __asm__ __volatile__ ("mfence" ::: "memory");

            if (event->status) *event->status = EVENT_PROCESSED;

            event_destroy(event);
        } else {
            sem_wait(event_loop->semaphore);
        }
    }

    return NULL;
}

void event_loop_post(struct event_loop *event_loop, struct event *event)
{
    if (event_loop->is_running) {
        queue_push(&event_loop->queue, event);
        sem_post(event_loop->semaphore);
    } else if (event->status) {
        *event->status = EVENT_IGNORED;
        event_destroy(event);
    }
}

bool event_loop_init(struct event_loop *event_loop)
{
    queue_init(&event_loop->queue);
    event_loop->is_running = 0;
    event_loop->semaphore = sem_open("yabai_event_loop_semaphore", O_CREAT, 0600, 0);
    sem_unlink("yabai_event_loop_semaphore");
    return event_loop->semaphore != SEM_FAILED;
}

bool event_loop_begin(struct event_loop *event_loop)
{
    if (event_loop->is_running) return false;
    event_loop->is_running = true;
    pthread_create(&event_loop->thread, NULL, &event_loop_run, event_loop);
    return true;
}

bool event_loop_end(struct event_loop *event_loop)
{
    if (!event_loop->is_running) return false;
    event_loop->is_running = false;
    pthread_join(event_loop->thread, NULL);
    return true;
}
