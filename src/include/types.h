#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include "lexer.h"

#include <forge/array.h>

#include <stddef.h>

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

        TYPE_KIND_SIZET,

        TYPE_KIND_NUMBER,

        TYPE_KIND_BOOL,

        TYPE_KIND_STR,
        TYPE_KIND_VOID, /*11*/
        TYPE_KIND_NORETURN,

        TYPE_KIND_PTR,
        TYPE_KIND_PROC,
        TYPE_KIND_PROCPTR,

        TYPE_KIND_LIST,

        TYPE_KIND_STRUCT,

        TYPE_KIND_UNKNOWN,
} type_kind;

typedef struct {
        type_kind kind;
        int sz;
} type;

DYN_ARRAY_TYPE(type *, type_array);

typedef struct { type base; } type_i8;
typedef struct { type base; } type_i16;
typedef struct { type base; } type_i32;
typedef struct { type base; } type_i64;

typedef struct { type base; } type_u8;
typedef struct { type base; } type_u16;
typedef struct { type base; } type_u32;
typedef struct { type base; } type_u64;

typedef struct { type base; } type_number;

typedef struct {
        type base;
        type *to;
} type_ptr;

typedef struct { type base; } type_void;
typedef struct { type base; } type_noreturn;

typedef struct {
        type base;
        const char *id;
        type *rettype;
        const parameter_array *params;
        int variadic;
        int export;
        int extern_;
} type_proc;

typedef struct {
        type base;
        type_array param_types;
        type *rettype;
        int variadic;
} type_procptr;

typedef struct {
        type base;
        type *elemty;
        int len;
} type_list;

typedef struct {
        type base;
        const parameter_array *members;
} type_struct;

typedef struct { type base; } type_unknown;
typedef struct { type base; } type_bool;
typedef struct { type base; } type_sizet;

type_i32 *type_i32_alloc(void);
type_i64 *type_i64_alloc(void);
type_u32 *type_u32_alloc(void);
type_u8 *type_u8_alloc(void);
type_i8 *type_i8_alloc(void);
type_number *type_number_alloc(void);
type_noreturn *type_noreturn_alloc(void);
type_ptr *type_ptr_alloc(type *to);
type_void *type_void_alloc(void);
type_struct *type_struct_alloc(const parameter_array *members, size_t sz);
type_unknown *type_unknown_alloc(void);
type_list *type_list_alloc(type *elemty, int len);
type_bool *type_bool_alloc(void);
type_sizet *type_sizet_alloc(void);

type_proc *type_proc_alloc(
        const char *id,
        type *rettype,
        const parameter_array *params,
        int variadic,
        int export,
        int extern_
);
type_procptr *type_procptr_alloc(type_array param_types, type *rettype, int variadic);

char *type_to_cstr(const type *t);
int type_is_compat(type **t1, type **t2);
int type_to_int(const type *t);
const char *type_kind_to_cstr(type_kind t);
int type_is_unsigned(const type *t);
type *type_get_lowest(type *t);

void type_get_types_from_proc(const type_proc *proc, type_array *params, type **rettype);

#endif // TYPES_H_INCLUDED
