#include "types.h"
#include "mem.h"

#include <forge/cstr.h>
#include <forge/str.h>
#include <forge/err.h>

#include <assert.h>
#include <stddef.h>

void type_get_types_from_proc(const type_proc  *proc,
                              type_array       *params,
                              type            **rettype);

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
        t->base.sz   = 1;
        return t;
}

type_i8 *
type_i8_alloc(void)
{
        type_i8 *t = (type_i8 *)alloc(sizeof(type_i8));
        t->base.kind = TYPE_KIND_I8;
        t->base.sz   = 1;
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
                int                    export,
                int                    extern_)
{
        type_proc *t = (type_proc *)alloc(sizeof(type_proc));
        t->base.kind = TYPE_KIND_PROC;
        t->base.sz   = 8;
        t->id        = id;
        t->rettype   = rettype;
        t->params    = params;
        t->variadic  = variadic;
        t->export    = export;
        t->extern_   = extern_;
        return t;
}

type_procptr *
type_procptr_alloc(type_array  param_types,
                   type       *rettype,
                   int         variadic)
{
        type_procptr *t = (type_procptr *)alloc(sizeof(type_procptr));
        t->base.kind    = TYPE_KIND_PROCPTR;
        t->base.sz      = 8;
        t->param_types  = param_types;
        t->rettype      = rettype;
        t->variadic     = variadic;
        return t;
}

type_unknown *
type_unknown_alloc(void)
{
        type_unknown *t = (type_unknown *)alloc(sizeof(type_unknown));
        t->base.kind    = TYPE_KIND_UNKNOWN;
        t->base.sz      = 0;
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

type_list *
type_list_alloc(type *elemty, int len)
{
        type_list *t = (type_list *)alloc(sizeof(type_list));
        t->base.kind  = TYPE_KIND_LIST;
        t->base.sz    = 8;
        t->elemty     = elemty;
        t->len        = len;
        return t;
}

type_bool *
type_bool_alloc(void)
{
        type_bool *t = (type_bool *)alloc(sizeof(type_bool));
        t->base.kind = TYPE_KIND_BOOL;
        t->base.sz   = 1;
        return t;
}

type_sizet *
type_sizet_alloc(void)
{
        type_sizet *t = (type_sizet *)alloc(sizeof(type_sizet));
        t->base.kind  = TYPE_KIND_SIZET;
        t->base.sz    = 8;
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
        case TYPE_KIND_LIST: {
                const type_list *ar = (const type_list *)t;
                char sz[32] = {0};
                sprintf(sz, "%d", ar->len);
                return forge_cstr_builder("<list [elemty=",
                                          type_to_cstr(ar->elemty),
                                          ", len=", sz,
                                          "]>", NULL);
        } break;
        case TYPE_KIND_BOOL:     return "bool";
        case TYPE_KIND_SIZET:    return "size_t";
        case TYPE_KIND_PROC: {
                type_array params = {0};
                type *rettype = NULL;
                type_get_types_from_proc((type_proc *)t, &params, &rettype);
                forge_str res = forge_str_from("<proc(");
                for (size_t i = 0; i < params.len; ++i) {
                        if (i != 0) forge_str_concat(&res, ", ");
                        forge_str_concat(&res, type_to_cstr(params.data[i]));
                }
                forge_str_concat(&res, "): ");
                forge_str_concat(&res, type_to_cstr(rettype));
                forge_str_append(&res, '>');
                return res.data;
        } break;
        case TYPE_KIND_PROCPTR: {
                type_procptr *proc = (type_procptr *)t;
                type_array params = proc->param_types;
                type *rettype = proc->rettype;
                forge_str res = forge_str_from("<procptr(");
                for (size_t i = 0; i < params.len; ++i) {
                        if (i != 0) forge_str_concat(&res, ", ");
                        forge_str_concat(&res, type_to_cstr(params.data[i]));
                }
                forge_str_concat(&res, "): ");
                forge_str_concat(&res, type_to_cstr(rettype));
                forge_str_append(&res, '>');
                return res.data;
        }
        default: {
                forge_err_wargs("type_to_cstr(): unknown type `%d`", (int)t->kind);
        } break;
        }

        return NULL; // unreachable
}

const char *
type_kind_to_cstr(type_kind t)
{
        switch (t) {
        case TYPE_KIND_I8:       return "i8";
        case TYPE_KIND_I16:      return "i16";
        case TYPE_KIND_I32:      return "i32";
        case TYPE_KIND_I64:      return "i64";
        case TYPE_KIND_U8:       return "u8";
        case TYPE_KIND_U16:      return "u16";
        case TYPE_KIND_U32:      return "u32";
        case TYPE_KIND_U64:      return "u64";
        case TYPE_KIND_NUMBER:   return "number";
        case TYPE_KIND_PTR:      return "ptr";
        case TYPE_KIND_VOID:     return "void";
        case TYPE_KIND_NORETURN: return "!";
        case TYPE_KIND_UNKNOWN:  return "<unknown>";
        case TYPE_KIND_STRUCT:   return "<struct>";
        case TYPE_KIND_LIST:     return "<list>";
        case TYPE_KIND_BOOL:     return "bool";
        case TYPE_KIND_SIZET:    return "size_t";
        case TYPE_KIND_PROC:     return "<proc>";
        case TYPE_KIND_PROCPTR:  return "<procptr>";
        default: {
                forge_err_wargs("type_to_cstr(): unknown type `%d`", (int)t);
        } break;
        }

        return NULL; // unreachable
}

void
type_get_types_from_proc(const type_proc  *proc,
                         type_array       *params,
                         type            **rettype)
{

        // Note: absolutely hellish workaround because
        //       this translation unit does not know
        //       what is inside of a parameter array.
        struct PARAM {
                const token *id;
                type *type;
                void *resolved;
        };

        struct ARRAY {
                struct PARAM *data;
                size_t len;
                size_t cap;
        };

        *params = dyn_array_empty(type_array);
        for (size_t i = 0; i < ((struct ARRAY *)proc->params)->len; ++i) {
                dyn_array_append(*params, ((struct ARRAY *)proc->params)->data[i].type);
        }
        *rettype = proc->rettype;
}

int
type_is_compat(type **t1, type **t2)
{
        assert(t1);
        assert(t2);

        assert(*t1);
        assert(*t2);

        type_kind t1kind = (*t1)->kind;
        type_kind t2kind = (*t2)->kind;

        /* assert(t1kind != TYPE_KIND_PROC */
        /*        && t2kind != TYPE_KIND_PROC */
        /*        && "proc type checking is unimplemented"); */

        if ((t1kind == TYPE_KIND_PROC || t2kind == TYPE_KIND_PROC)
            && (t1kind == TYPE_KIND_PROCPTR || t2kind == TYPE_KIND_PROCPTR)) {
                type_array  ar1  = dyn_array_empty(type_array);
                type_array  ar2  = dyn_array_empty(type_array);
                type       *ret1 = NULL;
                type       *ret2 = NULL;
                int         var1 = 0;
                int         var2 = 0;

                if (t1kind == TYPE_KIND_PROC) {
                        type_get_types_from_proc((type_proc *)(*t1), &ar1, &ret1);
                        var1 = ((type_proc *)(*t1))->variadic;
                } else if (t1kind == TYPE_KIND_PROCPTR) {
                        ar1 = ((type_procptr *)(*t1))->param_types;
                        ret1 = ((type_procptr *)(*t1))->rettype;
                        var1 = ((type_procptr *)(*t1))->variadic;
                }

                if (t2kind == TYPE_KIND_PROC) {
                        type_get_types_from_proc((type_proc *)(*t2), &ar2, &ret2);
                        var2 = ((type_proc *)(*t2))->variadic;
                } else if (t2kind == TYPE_KIND_PROCPTR) {
                        ar2 = ((type_procptr *)(*t2))->param_types;
                        ret2 = ((type_procptr *)(*t2))->rettype;
                        var2 = ((type_procptr *)(*t2))->variadic;
                }

                if (ar1.len != ar2.len) return 0;
                for (size_t i = 0; i < ar1.len; ++i) {
                        if (!type_is_compat(&ar1.data[i], &ar2.data[i])) {
                                return 0;
                        }
                }

                if (!type_is_compat(&ret1, &ret2)) return 0;
                return var1 == var2;
        }

        if (t1kind == TYPE_KIND_PTR && t2kind == TYPE_KIND_PTR) {
                return type_is_compat(&((type_ptr *)(*t1))->to, &((type_ptr *)(*t2))->to);
        }

        if (t1kind == TYPE_KIND_LIST && t2kind == TYPE_KIND_LIST) {
                type_list *ar1 = (type_list *)(*t1);
                type_list *ar2 = (type_list *)(*t2);
                return type_is_compat(&ar1->elemty, &ar2->elemty)
                        && ar1->len == ar2->len;
        }

        /* if ((t1kind == TYPE_KIND_ARRAY && t2kind == TYPE_KIND_PTR)) { */
        /*         return type_is_compat(&((type_array *)t1)->elemty, &((type_ptr *)t2)->to); */
        /* } */

        /* if ((t1kind == TYPE_KIND_PTR && t2kind == TYPE_KIND_ARRAY)) { */
        /*         return type_is_compat(&((type_ptr *)t1)->to, &((type_array *)t2)->elemty); */
        /* } */

        if (t1kind == TYPE_KIND_BOOL || t2kind == TYPE_KIND_BOOL) return 1;

        if (t1kind == TYPE_KIND_NUMBER
            && t2kind < TYPE_KIND_NUMBER) {
                (*t1) = (*t2);
        } else if (t1kind < TYPE_KIND_NUMBER
                   && t2kind == TYPE_KIND_NUMBER) {
                (*t2) = (*t1);
        }

        /* if (t1kind <= TYPE_KIND_NUMBER && t2kind <= TYPE_KIND_NUMBER) { */
        /*         return 1; */
        /* } */

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
        case TYPE_KIND_SIZET:    return 8;
        default: {
                forge_err_wargs("type_to_int(): unknown type `%d`", (int)t->kind);
        } break;
        }

        return 0; // unreachable
}

int
type_is_unsigned(const type *t)
{
        type_kind k = t->kind;
        return k == TYPE_KIND_U8
                || k == TYPE_KIND_U16
                || k == TYPE_KIND_U32
                || k == TYPE_KIND_U64;
}

type *
type_get_lowest(type *t)
{
        if (t->kind == TYPE_KIND_PTR) {
                return type_get_lowest(((type_ptr *)t)->to);
        } else if (t->kind == TYPE_KIND_LIST) {
                return type_get_lowest(((type_list *)t)->elemty);
        }
        return t;
}
