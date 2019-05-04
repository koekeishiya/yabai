#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <semaphore.h>
#include <pthread.h>

struct queue_item
{
    struct event *data;
    struct queue_item *next;
};

struct queue
{
    struct queue_item *head;
    struct queue_item *tail;
};

struct eventloop
{
    bool is_running;
    pthread_t thread;
    sem_t *semaphore;
    struct queue queue;
};

bool eventloop_init(struct eventloop *eventloop);
bool eventloop_begin(struct eventloop *eventloop);
bool eventloop_end(struct eventloop *eventloop);
void eventloop_post(struct eventloop *eventloop, struct event *event);

#endif
