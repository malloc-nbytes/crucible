#include "types.h"
#include "mem.h"

#include <forge/cstr.h>
#include <forge/err.h>

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

type_noreturn *
type_noreturn_alloc(void)
{
        type_noreturn *t = (type_noreturn *)alloc(sizeof(type_noreturn));
        t->base.kind = TYPE_KIND_NORETURN;
        return t;
}

type_ptr *
type_ptr_alloc(type *to)
{
        type_ptr *t = (type_ptr *)alloc(sizeof(type_ptr));
        t->base.kind = TYPE_KIND_PTR;
        t->to = to;
        return t;
}

char *
type_to_cstr(const type *t)
{
        switch (t->kind) {
        case TYPE_KIND_I8: return "i8";
        case TYPE_KIND_I16: return "i16";
        case TYPE_KIND_I32: return "i32";
        case TYPE_KIND_I64: return "i64";
        case TYPE_KIND_U8: return "u8";
        case TYPE_KIND_U16: return "u16";
        case TYPE_KIND_U32: return "u32";
        case TYPE_KIND_U64: return "u64";
        case TYPE_KIND_PTR: {
                return forge_cstr_builder("ptr<", type_to_cstr(((type_ptr *)t)->to), ">", NULL);
        } break;
        case TYPE_KIND_VOID: return "void";
        case TYPE_KIND_NORETURN: return "!";
        default: forge_err_wargs("type_to_cstr(): unknown type `%d`", (int)t->kind);
        }

        return NULL; // unreachable
}
