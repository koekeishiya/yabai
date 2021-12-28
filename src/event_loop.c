static void *event_loop_run(void *context)
{
    struct event_loop_item *head;
    struct event_loop *event_loop = (struct event_loop *) context;

    while (event_loop->is_running) {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

        for (;;) {
            do {
                head = event_loop->head;
                if (!head->next) goto empty;
            } while (!__sync_bool_compare_and_swap(&event_loop->head, head, head->next));

            struct event *event = &head->next->event;

            uint32_t result = event_handler[event->type](event->context, event->param1);
            if (event->info) *event->info = (result << 0x1) | EVENT_PROCESSED;

            event_signal_flush();
            ts_reset();
        }

empty:
        [pool drain];
        sem_wait(event_loop->semaphore);
    }

    return NULL;
}

void event_loop_post(struct event_loop *event_loop, enum event_type type, void *context, int param1, volatile uint32_t *info)
{
    bool success;
    struct event_loop_item *tail, *new_tail;

    new_tail = memory_pool_push(&event_loop->pool, sizeof(struct event_loop_item));
    new_tail->event.type = type;
    new_tail->event.context = context;
    new_tail->event.param1 = param1;
    new_tail->event.info = info;
    new_tail->next = NULL;

    __asm__ __volatile__ ("" ::: "memory");

    do {
        tail = event_loop->tail;
        success = __sync_bool_compare_and_swap(&tail->next, NULL, new_tail);
    } while (!success);
    __sync_bool_compare_and_swap(&event_loop->tail, tail, new_tail);

    sem_post(event_loop->semaphore);
}

bool event_loop_init(struct event_loop *event_loop)
{
    if (!memory_pool_init(&event_loop->pool, KILOBYTES(128))) return false;

    event_loop->head = memory_pool_push(&event_loop->pool, sizeof(struct event_loop_item));
    event_loop->head->next = NULL;
    event_loop->tail = event_loop->head;

    event_loop->is_running = false;
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
