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

typedef struct {
        type_kind kind;
        uint32_t id;
        size_t size;
        size_t align;
} type;

ARRAY_TYPE(type *, type_array);

/* typedef struct type { */
/*         type_kind kind; */
/*         uint32_t id; */
/*         size_t size; */
/*         size_t align; */

/*         union { */
/*                 struct { type *to; } ptr; */
/*         }; */
/* } type; */

typedef struct { type base; } type_void;
typedef struct { type base; } type_i32;
typedef struct { type base; } type_never;
typedef struct {
        type base;
        type *to;
} type_ptr;

type_void *type_void_alloc(arena *a);
type_i32 *type_i32_alloc(arena *a);
type_never *type_never_alloc(arena *a);
const char *type_to_cstr(const type *t);

#endif // TYPE_H_INCLUDED
