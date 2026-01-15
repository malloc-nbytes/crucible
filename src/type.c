#include "type.h"

type_void *
type_void_alloc(arena *a)
{
        type_void *t;

        t = arena_alloc(a, sizeof(type_void));
        t->base.kind = TYPE_KIND_VOID;
        return t;
}

type_i32 *
type_i32_alloc(arena *a)
{
        type_i32 *t;

        t = arena_alloc(a, sizeof(type_i32));
        t->base.kind = TYPE_KIND_I32;
        return t;
}

type_never *
type_never_alloc(arena *a)
{
        type_never *t;

        t = arena_alloc(a, sizeof(type_never));
        t->base.kind = TYPE_KIND_NEVER;
        return t;
}
