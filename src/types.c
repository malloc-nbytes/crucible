#include "types.h"
#include "mem.h"

type_i32 *
type_i32_alloc(void)
{
        type_i32 *t = (type_i32 *)alloc(sizeof(type_i32));
        t->base.kind = TYPE_KIND_I32;
        return t;
}

type_u32 *
type_u32_alloc(void)
{
        type_u32 *t = (type_u32 *)alloc(sizeof(type_u32));
        t->base.kind = TYPE_KIND_U32;
        return t;
}

type_u8 *
type_u8_alloc(void)
{
        type_u8 *t = (type_u8 *)alloc(sizeof(type_u8));
        t->base.kind = TYPE_KIND_U8;
        return t;
}
