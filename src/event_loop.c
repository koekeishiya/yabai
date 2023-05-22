#define IGNORE_EVENT(event_type) event_type: __builtin_unreachable()
#define HANDLE_EVENT(event_type) event_type: result = EVENT_HANDLER_##event_type(event->context, event->param1)
static void *event_loop_run(void *context)
{
    uint32_t result;
    struct event *event;
    struct event_loop_item *head;
    struct event_loop *event_loop = (struct event_loop *) context;

    while (event_loop->is_running) {
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

        for (;;) {
            do {
                head = event_loop->head;
                if (!head->next) goto empty;
            } while (!__sync_bool_compare_and_swap(&event_loop->head, head, head->next));

            event = &head->next->event;
            switch (event->type) {
            case IGNORE_EVENT(EVENT_TYPE_UNKNOWN);                 break;
            case HANDLE_EVENT(APPLICATION_LAUNCHED);               break;
            case HANDLE_EVENT(APPLICATION_TERMINATED);             break;
            case HANDLE_EVENT(APPLICATION_FRONT_SWITCHED);         break;
            case HANDLE_EVENT(APPLICATION_ACTIVATED);              break;
            case HANDLE_EVENT(APPLICATION_DEACTIVATED);            break;
            case HANDLE_EVENT(APPLICATION_VISIBLE);                break;
            case HANDLE_EVENT(APPLICATION_HIDDEN);                 break;
            case HANDLE_EVENT(WINDOW_CREATED);                     break;
            case HANDLE_EVENT(WINDOW_DESTROYED);                   break;
            case HANDLE_EVENT(WINDOW_FOCUSED);                     break;
            case HANDLE_EVENT(WINDOW_MOVED);                       break;
            case HANDLE_EVENT(WINDOW_RESIZED);                     break;
            case HANDLE_EVENT(WINDOW_MINIMIZED);                   break;
            case HANDLE_EVENT(WINDOW_DEMINIMIZED);                 break;
            case HANDLE_EVENT(WINDOW_TITLE_CHANGED);               break;
            case HANDLE_EVENT(SLS_WINDOW_MOVED);                   break;
            case HANDLE_EVENT(SLS_WINDOW_RESIZED);                 break;
            case HANDLE_EVENT(SLS_WINDOW_ORDER_CHANGED);           break;
            case HANDLE_EVENT(SLS_WINDOW_IS_VISIBLE);              break;
            case HANDLE_EVENT(SLS_WINDOW_IS_INVISIBLE);            break;
            case HANDLE_EVENT(SPACE_CHANGED);                      break;
            case HANDLE_EVENT(DISPLAY_ADDED);                      break;
            case HANDLE_EVENT(DISPLAY_REMOVED);                    break;
            case HANDLE_EVENT(DISPLAY_MOVED);                      break;
            case HANDLE_EVENT(DISPLAY_RESIZED);                    break;
            case HANDLE_EVENT(DISPLAY_CHANGED);                    break;
            case HANDLE_EVENT(MOUSE_DOWN);                         break;
            case HANDLE_EVENT(MOUSE_UP);                           break;
            case HANDLE_EVENT(MOUSE_DRAGGED);                      break;
            case HANDLE_EVENT(MOUSE_MOVED);                        break;
            case HANDLE_EVENT(MISSION_CONTROL_SHOW_ALL_WINDOWS);   break;
            case HANDLE_EVENT(MISSION_CONTROL_SHOW_FRONT_WINDOWS); break;
            case HANDLE_EVENT(MISSION_CONTROL_SHOW_DESKTOP);       break;
            case HANDLE_EVENT(MISSION_CONTROL_ENTER);              break;
            case HANDLE_EVENT(MISSION_CONTROL_CHECK_FOR_EXIT);     break;
            case HANDLE_EVENT(MISSION_CONTROL_EXIT);               break;
            case HANDLE_EVENT(DOCK_DID_RESTART);                   break;
            case HANDLE_EVENT(MENU_OPENED);                        break;
            case HANDLE_EVENT(MENU_CLOSED);                        break;
            case HANDLE_EVENT(MENU_BAR_HIDDEN_CHANGED);            break;
            case HANDLE_EVENT(DOCK_DID_CHANGE_PREF);               break;
            case HANDLE_EVENT(SYSTEM_WOKE);                        break;
            case HANDLE_EVENT(DAEMON_MESSAGE);                     break;
            case IGNORE_EVENT(EVENT_TYPE_COUNT);                   break;
            }
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
#undef HANDLE_EVENT
#undef IGNORE_EVENT

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
    if (!memory_pool_init(&event_loop->pool, KILOBYTES(512))) return false;

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
