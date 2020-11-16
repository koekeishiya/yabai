#ifndef TS_H
#define TS_H

static struct {
    void *memory;
    uint64_t size;
    volatile uint64_t used;
} g_temp_storage;

bool ts_init(uint64_t size)
{
    g_temp_storage.used = 0;
    g_temp_storage.size = size;
    g_temp_storage.memory = mmap(0, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    return g_temp_storage.memory != NULL;
}

void *ts_alloc(uint64_t size)
{
    uint64_t used = __sync_fetch_and_add(&g_temp_storage.used, size);
    assert(used + size < g_temp_storage.size);
    return g_temp_storage.memory + used;
}

void *ts_expand(void *ptr, uint64_t old_size, uint64_t increment)
{
    if (!ptr) return ts_alloc(increment);

    assert(ptr == g_temp_storage.memory + g_temp_storage.used - old_size);
    ts_alloc(increment);

    return ptr;
}

void ts_reset(void)
{
    // printf("resetting temp storage, used: %f%%.\n", (float)g_temp_storage.used / (float)g_temp_storage.size * 100);
    g_temp_storage.used = 0;
}

#endif
