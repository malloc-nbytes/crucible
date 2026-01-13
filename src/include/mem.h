#ifndef MEM_H_INCLUDED
#define MEM_H_INCLUDED

#include <stdint.h>
#include <stddef.h>

#define ARENA_DEFAULT_ALLOC_SIZE 4096
#ifndef ARENA_ALIGN_SIZE
#define ARENA_ALIGN_SIZE 16
#endif
#define ARENA_ALIGN_MASK (ARENA_ALIGN_SIZE-1)

typedef struct arena_blk {
        uint8_t *buf;
        size_t cap;
        size_t offset;
        struct arena_blk *n;
} arena_blk;

typedef struct {
        arena_blk *hd;
} arena;

void *s_realloc(void *b, size_t sz);
void *s_malloc(void *p, size_t b);
void *s_calloc(size_t bytes, size_t fill);

void arena_init(arena *a, size_t bytes);
void *arena_alloc(arena *a, size_t size);
void *arena_allocz(arena *a, size_t size);
void arena_clear(arena *a);
void arena_free(arena *a);

#endif // MEM_H_INCLUDED
