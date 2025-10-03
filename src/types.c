#include "types.h"
#include "mem.h"

#include <forge/cstr.h>
#include <forge/err.h>

#include <assert.h>
#include <stddef.h>

type_i32 *
type_i32_alloc(void)
{
        type_i32 *t = (type_i32 *)alloc(sizeof(type_i32));
        t->base.kind = TYPE_KIND_I32;
        t->base.sz   = 4;
        return t;
}

type_i64 *
type_i64_alloc(void)
{
        type_i64 *t = (type_i64 *)alloc(sizeof(type_i64));
        t->base.kind = TYPE_KIND_I64;
        t->base.sz   = 8;
        return t;
}

type_u32 *
type_u32_alloc(void)
{
        type_u32 *t = (type_u32 *)alloc(sizeof(type_u32));
        t->base.kind = TYPE_KIND_U32;
        t->base.sz   = 4;
        return t;
}

type_u8 *
type_u8_alloc(void)
{
        type_u8 *t = (type_u8 *)alloc(sizeof(type_u8));
        t->base.kind = TYPE_KIND_U8;
        t->base.sz   = 8;
        return t;
}

type_noreturn *
type_noreturn_alloc(void)
{
        type_noreturn *t = (type_noreturn *)alloc(sizeof(type_noreturn));
        t->base.kind = TYPE_KIND_NORETURN;
        t->base.sz   = 0;
        return t;
}

type_ptr *
type_ptr_alloc(type *to)
{
        type_ptr *t = (type_ptr *)alloc(sizeof(type_ptr));
        t->base.kind = TYPE_KIND_PTR;
        t->base.sz   = 8;
        t->to = to;
        return t;
}

type_void *
type_void_alloc(void)
{
        type_void *t = (type_void *)alloc(sizeof(type_void));
        t->base.kind = TYPE_KIND_VOID;
        t->base.sz   = 0;
        return t;
}

type_proc *
type_proc_alloc(const char            *id,
                type                  *rettype,
                const parameter_array *params,
                int                    variadic,
                int                    export)
{
        type_proc *t = (type_proc *)alloc(sizeof(type_proc));
        t->base.kind = TYPE_KIND_PROC;
        t->base.sz   = 8;
        t->id        = id;
        t->rettype   = rettype;
        t->params    = params;
        t->variadic  = variadic;
        t->export    = export;
        return t;
}

type_unknown *
type_unknown_alloc(void)
{
        type_unknown *t = (type_unknown *)alloc(sizeof(type_unknown));
        t->base.kind = TYPE_KIND_UNKNOWN;
        t->base.sz   = 0;
        return t;
}

type_number *
type_number_alloc(void)
{
        type_number *t = (type_number *)alloc(sizeof(type_number));
        t->base.kind = TYPE_KIND_NUMBER;
        t->base.sz   = 4;
        return t;
}

type_struct *
type_struct_alloc(const parameter_array *members, size_t sz)
{
        type_struct *t = (type_struct *)alloc(sizeof(type_struct));
        t->base.kind   = TYPE_KIND_STRUCT;
        t->base.sz     = sz;
        t->members     = members;

        return t;
}

type_custom *
type_custom_alloc(const token *struct_id)
{
        type_custom *t   = (type_custom *)alloc(sizeof(type_custom));
        t->base.kind     = TYPE_KIND_CUSTOM;
        t->struct_id     = struct_id;
        return t;
}

char *
type_to_cstr(const type *t)
{
        switch (t->kind) {
        case TYPE_KIND_I8:     return "i8";
        case TYPE_KIND_I16:    return "i16";
        case TYPE_KIND_I32:    return "i32";
        case TYPE_KIND_I64:    return "i64";
        case TYPE_KIND_U8:     return "u8";
        case TYPE_KIND_U16:    return "u16";
        case TYPE_KIND_U32:    return "u32";
        case TYPE_KIND_U64:    return "u64";
        case TYPE_KIND_NUMBER: return "number";
        case TYPE_KIND_PTR: {
                return forge_cstr_builder("ptr<", type_to_cstr(((type_ptr *)t)->to), ">", NULL);
        }
        case TYPE_KIND_VOID:     return "void";
        case TYPE_KIND_NORETURN: return "!";
        case TYPE_KIND_UNKNOWN:  return "<unknown>";
        case TYPE_KIND_STRUCT:   return "<struct>";
        case TYPE_KIND_CUSTOM:   return "<struct>";
        default: {
                forge_err_wargs("type_to_cstr(): unknown type `%d`", (int)t->kind);
        } break;
        }

        return NULL; // unreachable
}

int
type_is_compat(type **t1, type **t2)
{
        assert(t1);
        assert(t2);

        assert(*t1);
        assert(*t2);

        assert((*t1)->kind != TYPE_KIND_PROC
               && (*t2)->kind != TYPE_KIND_PROC
               && "proc type checking unimplemented");

        if ((*t1)->kind == TYPE_KIND_PTR
            && (*t2)->kind == TYPE_KIND_PTR) {
                return type_is_compat(&((type_ptr *)(*t1))->to, &((type_ptr *)(*t2))->to);
        }

        if ((*t1)->kind == TYPE_KIND_NUMBER
            && (*t2)->kind < TYPE_KIND_NUMBER) {
                (*t1) = (*t2);
        } else if ((*t1)->kind < TYPE_KIND_NUMBER
                   && (*t2)->kind == TYPE_KIND_NUMBER) {
                (*t2) = (*t1);
        }

        return (*t1)->kind == (*t2)->kind;
}

int
type_to_int(const type *t)
{
        assert(t);

        switch (t->kind) {
        case TYPE_KIND_I8:       return 1;
        case TYPE_KIND_I16:      return 2;
        case TYPE_KIND_I32:      return 4;
        case TYPE_KIND_I64:      return 8;
        case TYPE_KIND_U8:       return 1;
        case TYPE_KIND_U16:      return 2;
        case TYPE_KIND_U32:      return 4;
        case TYPE_KIND_U64:      return 8;
        case TYPE_KIND_NUMBER:   return 4;
        case TYPE_KIND_PTR:      return 8;
        case TYPE_KIND_VOID:     return 0;
        case TYPE_KIND_NORETURN: return 0;
        case TYPE_KIND_UNKNOWN:  return 0;
        case TYPE_KIND_PROC:     return 8;
        case TYPE_KIND_STRUCT:   return t->sz;
        case TYPE_KIND_CUSTOM:   return 0;
        default: {
                forge_err_wargs("type_to_int(): unknown type `%d`", (int)t->kind);
        } break;
        }

        return 0; // unreachable
}
