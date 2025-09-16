#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <forge/array.h>

typedef struct parameter_array parameter_array;

typedef enum {
        TYPE_KIND_I8 = 0,
        TYPE_KIND_I16,
        TYPE_KIND_I32,
        TYPE_KIND_I64,

        TYPE_KIND_U8,
        TYPE_KIND_U16,
        TYPE_KIND_U32,
        TYPE_KIND_U64,

        TYPE_KIND_STR,
        TYPE_KIND_VOID,
        TYPE_KIND_NORETURN,

        TYPE_KIND_PTR,
        TYPE_KIND_PROC,
} type_kind;

typedef struct { type_kind kind; } type;

typedef struct { type base; } type_i8;
typedef struct { type base; } type_i16;
typedef struct { type base; } type_i32;
typedef struct { type base; } type_i64;

typedef struct { type base; } type_u8;
typedef struct { type base; } type_u16;
typedef struct { type base; } type_u32;
typedef struct { type base; } type_u64;

typedef struct {
        type base;
        type *to;
} type_ptr;

typedef struct { type base; } type_void;
typedef struct { type base; } type_noreturn;

typedef struct {
        type base;
        type *rettype;
        const parameter_array *params;
} type_proc;

type_i32 *type_i32_alloc(void);
type_u32 *type_u32_alloc(void);
type_u8 *type_u8_alloc(void);
type_noreturn *type_noreturn_alloc(void);
type_ptr *type_ptr_alloc(type *to);

char *type_to_cstr(const type *t);
type_proc *type_proc_alloc(type *rettype,
                           const parameter_array *params);

int type_is_compat(const type *t1, const type *t2);

#endif // TYPES_H_INCLUDED
