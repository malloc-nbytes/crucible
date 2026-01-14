#include "type.h"

type_i32 *
type_i32_alloc(arena *a)
{
        type_i32 *t;

        t = arena_alloc(a, sizeof(type_i32));
        t->base.kind = TYPE_KIND_I32;
        return t;
}
