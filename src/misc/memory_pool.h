#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#define KILOBYTES(value) ((value) * 1024ULL)
#define MEGABYTES(value) (KILOBYTES(value) * 1024ULL)
#define GIGABYTES(value) (MEGABYTES(value) * 1024ULL)

struct memory_pool
{
    void *memory;
    uint64_t size;
    volatile uint64_t used;
};

bool memory_pool_init(struct memory_pool *pool, uint64_t size)
{
    pool->used = 0;
    pool->size = size;
    pool->memory = mmap(0, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    return pool->memory != NULL;
}

#define memory_pool_push(p, t) memory_pool_push_size(p, sizeof(t))
void *memory_pool_push_size(struct memory_pool *pool, uint64_t size)
{
    for (;;) {
        uint64_t used = pool->used;
        uint64_t new_used = used + size;

        if (new_used < pool->size) {
            if (__sync_bool_compare_and_swap(&pool->used, used, new_used)) {
                return pool->memory + used;
            }
        } else {
            if (__sync_bool_compare_and_swap(&pool->used, used, size)) {
                return pool->memory;
            }
        }
    }
}

#endif
