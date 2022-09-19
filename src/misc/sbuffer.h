#ifndef SBUFFER_H
#define SBUFFER_H

struct buf_hdr
{
    size_t len;
    size_t cap;
    char buf[0];
};

#define OFFSETOF(t, f) (size_t)((char *)&(((t *)0)->f) - (char *)0)

#define buf__hdr(b) ((struct buf_hdr *)((char *)(b) - OFFSETOF(struct buf_hdr, buf)))
#define buf__should_grow(b, n) (buf_len(b) + (n) >= buf_cap(b))
#define buf__fit(b, n) (buf__should_grow(b, n) ? ((b) = buf__grow_f(b, buf_len(b) + (n), sizeof(*(b)))) : 0)

#define buf_len(b) ((b) ? buf__hdr(b)->len : 0)
#define buf_cap(b) ((b) ? buf__hdr(b)->cap : 0)
#define buf_last(b) ((b)[buf_len(b)-1])
#define buf_push(b, x) (buf__fit(b, 1), (b)[buf_len(b)] = (x), buf__hdr(b)->len++)
#define buf_del(b, x) ((b) ? (b)[x] = (b)[buf_len(b)-1], buf__hdr(b)->len-- : 0)
#define buf_free(b) ((b) ? free(buf__hdr(b)) : 0)

static void *buf__grow_f(const void *buf, size_t new_len, size_t elem_size)
{
    size_t new_cap = max(1 + 2*buf_cap(buf), new_len);
    size_t new_size = OFFSETOF(struct buf_hdr, buf) + new_cap*elem_size;
    struct buf_hdr *new_hdr = realloc(buf ? buf__hdr(buf) : 0, new_size);
    new_hdr->cap = new_cap;
    if (!buf) {
        new_hdr->len = 0;
    }
    return new_hdr->buf;
}

struct ts_buf_hdr
{
    size_t len;
    size_t cap;
    char buf[0];
};

#define ts_buf__hdr(b) ((struct ts_buf_hdr *)((char *)(b) - OFFSETOF(struct ts_buf_hdr, buf)))
#define ts_buf__should_grow(b, n) (ts_buf_len(b) + (n) >= ts_buf_cap(b))
#define ts_buf__fit(b, n) (ts_buf__should_grow(b, n) ? ((b) = ts_buf__grow_f(b, ts_buf_len(b) + (n), sizeof(*(b)))) : 0)

#define ts_buf_len(b) ((b) ? ts_buf__hdr(b)->len : 0)
#define ts_buf_cap(b) ((b) ? ts_buf__hdr(b)->cap : 0)
#define ts_buf_last(b) ((b)[ts_buf_len(b)-1])
#define ts_buf_push(b, x) (ts_buf__fit(b, 1), (b)[ts_buf_len(b)] = (x), ts_buf__hdr(b)->len++)
#define ts_buf_del(b, x) ((b) ? (b)[x] = (b)[ts_buf_len(b)-1], ts_buf__hdr(b)->len-- : 0)

static void *ts_buf__grow_f(const void *buf, size_t new_len, size_t elem_size)
{
    struct ts_buf_hdr *new_hdr;
    size_t new_cap = max(1 + 2*ts_buf_cap(buf), new_len);
    size_t new_size = OFFSETOF(struct ts_buf_hdr, buf) + new_cap*elem_size;

    if (buf) {
        __sync_fetch_and_add(&g_temp_storage.used, new_size);
        new_hdr = ts_buf__hdr(buf);
    } else {
        new_hdr = ts_alloc_unaligned(new_size);
    }

    new_hdr->cap = new_cap;
    if (!buf) {
        new_hdr->len = 0;
    }
    return new_hdr->buf;
}

#endif
