#include "types.h"
#include "mem.h"

#include <assert.h>
#include <stddef.h>

type_void   *g_type_void   = NULL; // extern
type_i32    *g_type_i32    = NULL; // extern
type_u8     *g_type_u8     = NULL; // extern
type_u32    *g_type_u32    = NULL; // extern
type_number *g_type_number = NULL; // extern

typep_ar g_types = {0}; // extern

type_void *
type_void_alloc(void)
{
        type_void *t;

        t            = (type_void *)alloc(sizeof(type_void));
        t->base.kind = TYPE_KIND_VOID;

        return t;
}

type_i32 *
type_i32_alloc(void)
{
        type_i32 *t;

        t            = (type_i32 *)alloc(sizeof(type_i32));
        t->base.kind = TYPE_KIND_I32;

        return t;
}

type_u8 *
type_u8_alloc(void)
{
        type_u8 *t;

        t            = (type_u8 *)alloc(sizeof(type_u8));
        t->base.kind = TYPE_KIND_U8;

        return t;
}

type_u32 *
type_u32_alloc(void)
{
        type_u32 *t;

        t            = (type_u32 *)alloc(sizeof(type_u32));
        t->base.kind = TYPE_KIND_U32;

        return t;
}

type_number *
type_number_alloc(void)
{
        type_number *t;

        t            = (type_number *)alloc(sizeof(type_number));
        t->base.kind = TYPE_KIND_NUMBER;

        return t;
}

type_ptr  *
type_ptr_alloc(type *to)
{
        type_ptr *t;

        t            = (type_ptr *)alloc(sizeof(type_ptr));
        t->to        = to;
        t->base.kind = TYPE_KIND_PTR;

        return t;
}

void
init_types_translation_unit(void)
{
        g_type_void   = type_void_alloc();
        g_type_i32    = type_i32_alloc();
        g_type_u8     = type_u8_alloc();
        g_type_u32    = type_u32_alloc();
        g_type_number = type_number_alloc();
        g_types       = array_empty(typep_ar);

        size_t i = 0;

        array_append(g_types, (type *)g_type_void);   assert(i++ == TYPE_KIND_VOID);
        array_append(g_types, (type *)g_type_i32);    assert(i++ == TYPE_KIND_I32);
        array_append(g_types, (type *)g_type_u8);     assert(i++ == TYPE_KIND_U8);
        array_append(g_types, (type *)g_type_u32);    assert(i++ == TYPE_KIND_U32);
        array_append(g_types, (type *)g_type_number); assert(i++ == TYPE_KIND_NUMBER);
}
