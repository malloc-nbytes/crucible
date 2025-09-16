#include "types.h"
#include "mem.h"

#include <forge/cstr.h>
#include <forge/err.h>

#include <assert.h>

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

type_void *
type_void_alloc(void)
{
        type_void *t = (type_void *)alloc(sizeof(type_void));
        t->base.kind = TYPE_KIND_VOID;
        return t;
}

type_proc *
type_proc_alloc(type                  *rettype,
                const parameter_array *params)
{
        type_proc *t = (type_proc *)alloc(sizeof(type_proc));
        t->base.kind = TYPE_KIND_PROC;
        t->rettype = rettype;
        t->params = params;
        return t;
}

type_unknown *
type_unknown_alloc(void)
{
        type_unknown *t = (type_unknown *)alloc(sizeof(type_unknown));
        t->base.kind = TYPE_KIND_UNKNOWN;
        return t;
}

char *
type_to_cstr(const type *t)
{
        if (!t) return "null"; // Handle NULL type pointer
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
        }
        case TYPE_KIND_VOID: return "void";
        case TYPE_KIND_NORETURN: return "!";
        case TYPE_KIND_UNKNOWN: return "<unknown>";
        default: {
                forge_err_wargs("type_to_cstr(): unknown type `%d`", (int)t->kind);
        } break;
        }

        return NULL; // unreachable
}

int
type_is_compat(const type *t1,
               const type *t2)
{
        assert(t1);
        assert(t2);

        assert(t1->kind != TYPE_KIND_PROC
               && t2->kind != TYPE_KIND_PROC
               && "proc type checking unimplemented");

        if (t1->kind == TYPE_KIND_PTR
            && t2->kind == TYPE_KIND_PTR) {
                return type_is_compat(((type_ptr *)t1)->to, ((type_ptr *)t2)->to);
        }

        return t1->kind == t2->kind;
}
