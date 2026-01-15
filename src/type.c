#include "type.h"
#include "ds/array.h"

#include <assert.h>

void
type_context_init(type_context *ctx, arena *a)
{
    ctx->arena     = a;
    ctx->next_id   = 0;
    ctx->t_void    = NULL;
    ctx->t_i32     = NULL;
    ctx->t_never   = NULL;
    ctx->ptr_types = array_empty(type_array);
}

static type *
type_alloc(type_context *ctx, type_kind kind)
{
        type *t  = arena_alloc(ctx->arena, sizeof(type));
        t->kind  = kind;
        t->id    = ctx->next_id++;
        t->size  = 0;
        t->align = 0;
        return t;
}

type *
type_void(type_context *ctx)
{
        if (ctx->t_void)
                return ctx->t_void;

        type *t  = type_alloc(ctx, TYPE_KIND_VOID);
        t->size  = 0;
        t->align = 1;

        ctx->t_void = t;
        return t;
}

type *
type_i32(type_context *ctx)
{
        if (ctx->t_i32)
                return ctx->t_i32;

        type *t  = type_alloc(ctx, TYPE_KIND_I32);
        t->size  = 4;
        t->align = 4;

        ctx->t_i32 = t;
        return t;
}

type *
type_never(type_context *ctx)
{
        if (ctx->t_never)
                return ctx->t_never;

        type *t  = type_alloc(ctx, TYPE_KIND_NEVER);
        t->size  = 0;
        t->align = 1;

        ctx->t_never = t;
        return t;
}

type *
type_ptr(type_context *ctx, type *to)
{
        // look for existing ptr(to)
        for (size_t i = 0; i < ctx->ptr_types.len; ++i) {
                type *t = ctx->ptr_types.data[i];
                if (t->ptr.to == to)
                        return t;
        }

        // create new
        type *t   = type_alloc(ctx, TYPE_KIND_PTR);
        t->ptr.to = to;
        t->size   = 8;          // assume 64-bit target
        t->align  = 8;

        array_append(ctx->ptr_types, t);
        return t;
}

const char *
type_to_cstr(const type *t)
{
        switch (t->kind) {
        case TYPE_KIND_VOID:  return "void";
        case TYPE_KIND_I32:   return "i32";
        case TYPE_KIND_NEVER: return "never";
        case TYPE_KIND_PTR:   return "ptr";
        }
        return "<unknown>";
}
