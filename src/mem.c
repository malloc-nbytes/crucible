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

static arena_blk *
arena_blk_new(size_t cap)
{
        arena_blk *b = (arena_blk *)s_malloc(NULL, sizeof(*b));
        b->buf       = s_calloc(cap, 1);
        b->cap       = cap;
        b->offset    = 0;
        b->n         = NULL;
        return b;
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
arena_init(arena *a, size_t bytes)
{
        assert(bytes > 0);
        a->hd = arena_blk_new(bytes);
}

void *
arena_alloc(arena *a, size_t size)
{
        assert(a);
        assert(a->hd);

        size_t aligned = alignup(size);

        arena_blk *b = a->hd;

        // Find a block with enough space
        while (b) {
                if (b->offset + aligned <= b->cap)
                        break;
                if (!b->n)
                        break;
                b = b->n;
        }

        // Need a new block
        if (b->offset + aligned > b->cap) {
                size_t cap = b->cap * 2;
                if (cap < aligned)
                        cap = aligned;

                b->n = arena_blk_new(cap);
                b = b->n;
        }

        void *p = b->buf + b->offset;
        b->offset += aligned;

        return p;
}

void *
arena_allocz(arena *a, size_t size)
{
        void *p = arena_alloc(a, size);
        memset(p, 0, size);
        return p;
}

void
arena_clear(arena *a)
{
        for (arena_blk *b = a->hd; b; b = b->n)
                b->offset = 0;
}

void
arena_free(arena *a)
{
        arena_blk *b = a->hd;
        while (b) {
                arena_blk *next = b->n;
                free(b->buf);
                free(b);
                b = next;
        }
        a->hd = NULL;
}
