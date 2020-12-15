#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

struct memory_pool
{
    void *memory;
    uint64_t size;
    volatile uint64_t used;
};

bool memory_pool_init(struct memory_pool *pool, uint64_t size)
{
    uint64_t pa_size = size + (size % PAGE_SIZE);

    pool->used = 0;
    pool->size = pa_size;
    pool->memory = mmap(0, pa_size + PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

    bool result = pool->memory != MAP_FAILED;
    if (result) mprotect(pool->memory + pa_size, PAGE_SIZE, PROT_NONE);

    return result;
}

void *memory_pool_push(struct memory_pool *pool, uint64_t size)
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
