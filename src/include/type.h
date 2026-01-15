#ifndef TYPE_H_INCLUDED
#define TYPE_H_INCLUDED

#include "mem.h"
#include "ds/array.h"

#include <stdint.h>
#include <stddef.h>

typedef enum {
        TYPE_KIND_VOID = 0,
        TYPE_KIND_I32,
        TYPE_KIND_NEVER,
        TYPE_KIND_PTR,
} type_kind;

typedef struct type {
        type_kind kind;
        uint32_t id;
        size_t size;
        size_t align;

        union {
                struct { struct type *to; } ptr;
        };
} type;

ARRAY_TYPE(type *, type_array);

typedef struct {
    arena *arena;
    uint32_t next_id;

    type *t_void;
    type *t_i32;
    type *t_never;

    // cache for compound types
    type_array ptr_types;
} type_context;

void type_context_init(type_context *ctx, arena *a);
type *type_void(type_context *ctx);
type *type_i32(type_context *ctx);
type *type_never(type_context *ctx);
const char *type_to_cstr(const type *t);

#endif // TYPE_H_INCLUDED
