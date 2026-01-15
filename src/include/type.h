#ifndef TYPE_H_INCLUDED
#define TYPE_H_INCLUDED

#include "mem.h"
#include "ds/array.h"

typedef enum {
        TYPE_KIND_VOID = 0,
        TYPE_KIND_I32,
        TYPE_KIND_NEVER,
} type_kind;

typedef struct {
        type_kind kind;
} type;

ARRAY_TYPE(type *, type_array);

typedef struct { type base; } type_void;
typedef struct { type base; } type_i32;
typedef struct { type base; } type_never;

type_void *type_void_alloc(arena *a);
type_i32 *type_i32_alloc(arena *a);
type_never *type_never_alloc(arena *a);

#endif // TYPE_H_INCLUDED
