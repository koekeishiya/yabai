#ifndef HASHTABLE_H
#define HASHTABLE_H

#define TABLE_HASH_FUNC(name) unsigned long name(void *key)
typedef TABLE_HASH_FUNC(table_hash_func);

#define TABLE_COMPARE_FUNC(name) int name(void *key_a, void *key_b)
typedef TABLE_COMPARE_FUNC(table_compare_func);

struct bucket
{
    void *key;
    void *value;
    struct bucket *next;
};
struct table
{
    int count;
    int capacity;
    float max_load;
    table_hash_func *hash;
    table_compare_func *cmp;
    struct bucket **buckets;
};

void table_init(struct table *table, int capacity, table_hash_func hash, table_compare_func cmp);
void table_free(struct table *table);

#define table_add(table, key, value) _table_add(table, key, sizeof(*key), value)
void _table_add(struct table *table, void *key, int key_size, void *value);
void table_remove(struct table *table, void *key);
void *table_find(struct table *table, void *key);

#endif

#ifdef HASHTABLE_IMPLEMENTATION
void table_init(struct table *table, int capacity, table_hash_func hash, table_compare_func cmp)
{
    table->count = 0;
    table->capacity = capacity;
    table->max_load = 0.75f;
    table->hash = hash;
    table->cmp = cmp;
    table->buckets = malloc(sizeof(struct bucket *) * capacity);
    memset(table->buckets, 0, sizeof(struct bucket *) * capacity);
}

void table_free(struct table *table)
{
    for (int i = 0; i < table->capacity; ++i) {
        struct bucket *next, *bucket = table->buckets[i];
        while (bucket) {
            next = bucket->next;
            free(bucket->key);
            free(bucket);
            bucket = next;
        }
    }

    if (table->buckets) {
        free(table->buckets);
        table->buckets = NULL;
    }
}

static struct bucket **
table_get_bucket(struct table *table, void *key)
{
    struct bucket **bucket = table->buckets + (table->hash(key) % table->capacity);
    while (*bucket) {
        if (table->cmp((*bucket)->key, key)) {
            break;
        }
        bucket = &(*bucket)->next;
    }
    return bucket;
}

static void
table_rehash(struct table *table)
{
    struct bucket **old_buckets = table->buckets;
    int old_capacity = table->capacity;

    table->count = 0;
    table->capacity = 2 * table->capacity;
    table->buckets = malloc(sizeof(struct bucket *) * table->capacity);
    memset(table->buckets, 0, sizeof(struct bucket *) * table->capacity);

    for (int i = 0; i < old_capacity; ++i) {
        struct bucket *next_bucket, *old_bucket = old_buckets[i];
        while (old_bucket) {
            struct bucket **new_bucket = table_get_bucket(table, old_bucket->key);
            *new_bucket = malloc(sizeof(struct bucket));
            (*new_bucket)->key = old_bucket->key;
            (*new_bucket)->value = old_bucket->value;
            (*new_bucket)->next = NULL;
            ++table->count;
            next_bucket = old_bucket->next;
            free(old_bucket);
            old_bucket = next_bucket;
        }
    }

    free(old_buckets);
}

void _table_add(struct table *table, void *key, int key_size, void *value)
{
    struct bucket **bucket = table_get_bucket(table, key);
    if (*bucket) {
        if (!(*bucket)->value) {
            (*bucket)->value = value;
        }
    } else {
        *bucket = malloc(sizeof(struct bucket));
        (*bucket)->key = malloc(key_size);
        (*bucket)->value = value;
        memcpy((*bucket)->key, key, key_size);
        (*bucket)->next = NULL;
        ++table->count;

        float load = (1.0f * table->count) / table->capacity;
        if (load > table->max_load) {
            table_rehash(table);
        }
    }
}

void table_remove(struct table *table, void *key)
{
    struct bucket *next, **bucket = table_get_bucket(table, key);
    if (*bucket) {
        free((*bucket)->key);
        next = (*bucket)->next;
        free(*bucket);
        *bucket = next;
        --table->count;
    }
}

void *table_find(struct table *table, void *key)
{
    struct bucket *bucket = *table_get_bucket(table, key);
    return bucket ? bucket->value : NULL;
}
#endif
