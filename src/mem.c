#include "mem.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static inline size_t
alignup(size_t n)
{
        return (n + ARENA_ALIGN_MASK) & ~ARENA_ALIGN_MASK;
}

void *
s_realloc(void *b, size_t sz)
{
        if (!(b = realloc(b, sz))) {
                perror("realloc");
                exit(1);
        }
        return b;
}

void *
s_malloc(void *p, size_t b)
{
        if (!(p = malloc(b))) {
                perror("malloc");
                exit(1);
        }
        return p;
}

void *
s_calloc(size_t bytes, size_t fill)
{
        void *p;

        if (!(p = calloc(bytes, fill))) {
                perror("calloc");
                exit(1);
        }

        return p;
}

void
arena_init(arena  *a,
           size_t  bytes)
{
        a->buf    = (uint8_t *)s_calloc(bytes, 1);
        a->cap    = bytes;
        a->offset = 0;
}

void *
arena_alloc(arena *a, size_t size)
{
        assert(a->offset <= a->cap);
        assert(a->buf != NULL);
        assert(a->cap >= ARENA_DEFAULT_ALLOC_SIZE);

        size_t  aligned;
        size_t  new_offset;
        void   *p;

        aligned    = alignup(size);
        new_offset = a->offset + aligned;

        if (new_offset > a->cap) {
                a->cap *= 2;
                a->buf = (uint8_t *)s_realloc(a->buf, a->cap);
        }

        p         = a->buf + a->offset;
        a->offset = new_offset;

        return p;
}

void *
arena_allocz(arena *a, size_t size)
{
        void *p;

        if (p = arena_alloc(a, size))
                memset(p, 0, size);

        return p;
}

void
arena_clear(arena *a)
{
        a->offset = 0;
}

void
arena_free(arena *a)
{
        if (a->buf)
                free(a->buf);

        a->buf    = NULL;
        a->cap    = 0;
        a->offset = 0;
}
