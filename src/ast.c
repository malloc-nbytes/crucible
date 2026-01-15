#include "ast.h"
#include "visitor.h"
#include "utils.h"
#include "type.h"
#include "ds/strv.h"

#include <assert.h>
#include <stdio.h>
#include <stddef.h>

typedef struct {
        size_t depth;
} ast_context;

static void
spaces(ast_context *ctx)
{
        for (size_t i = 0; i < ctx->depth; ++i)
                printf("  ");
}

static void *
visit_stmt_exit(visitor *v, stmt_exit *s)
{
        ast_context *ctx = (ast_context *)v->ctx;

        spaces(ctx);

        printf("EXIT ");
        s->e->accept(s->e, v);
        putchar('\n');

        return NULL;
}

static void *
visit_stmt_return(visitor *v, stmt_return *s)
{
        ast_context *ctx = (ast_context *)v->ctx;

        spaces(ctx);

        printf("RETURN ");
        s->e->accept(s->e, v);
        putchar('\n');

        return NULL;
}

static void *
visit_stmt_let(visitor *v, stmt_let *s)
{
        ast_context *ctx = (ast_context *)v->ctx;

        spaces(ctx);
        printf("LET %s: %s<%d> = ", strv_scstr(s->id->lx), type_to_cstr(s->type), s->type->id);

        ++ctx->depth;
        s->e->accept(s->e, v);
        --ctx->depth;

        printf("\n");

        return NULL;
}

static void *
visit_stmt_proc(visitor *v, stmt_proc *s)
{
        ast_context *ctx = (ast_context *)v->ctx;

        spaces(ctx);

        printf("PROC %s(", strv_scstr(s->id->lx));

        for (size_t i = 0; i < s->params.len; ++i) {
                if (i != 0)
                        printf(", ");

                const char *id;
                const type *type;
                const char *t;

                id   = strv_scstr(s->params.data[i]->id->lx);
                type = s->params.data[i]->type;
                t    = type_to_cstr(type);

                printf("%s: %s<%d>", id, t, type->id);
        }

        printf("): %s<%d>", type_to_cstr(s->rtype), s->rtype->id);

        if (s->blk->kind == STMT_KIND_BLK)
                putchar('\n');
        else
                printf(" = ");

        s->blk->accept(s->blk, v);

        return NULL;
}

static void *
visit_stmt_blk(visitor *v, stmt_blk *s)
{
        ast_context *ctx = (ast_context *)v->ctx;

        spaces(ctx);
        printf("{\n");

        ++ctx->depth;
        for (size_t i = 0; i < s->stmts.len; ++i) {
                s->stmts.data[i]->accept(s->stmts.data[i], v);
        }
        --ctx->depth;

        spaces(ctx);
        printf("}\n");

        return NULL;
}

static void *
visit_expr_id(visitor *v, expr_id *e)
{
        NOOP(v);
        printf("ID(%s)", strv_scstr(e->i->lx));
        return NULL;
}

static void *
visit_expr_un(visitor *v, expr_un *e)
{
        NOOP(v, e);
        TODO("");
        return NULL;
}

static void *
visit_expr_bin(visitor *v, expr_bin *e)
{
        putchar('(');
        e->lhs->accept(e->lhs, v);
        printf(" %s ", strv_scstr(e->op->lx));
        e->rhs->accept(e->rhs, v);
        putchar(')');
        return NULL;
}

static void *
visit_expr_intlit(visitor *v, expr_intlit *e)
{
        NOOP(v);
        printf("INT(%d)", e->i);
        return NULL;
}

void
ast_dump(stmt_array stmts)
{
        ast_context  ctx;
        visitor     *v;

        ctx = (ast_context) {
                .depth = 0,
        };

        v = visitor_alloc((void *)&ctx,
                          visit_expr_intlit,
                          visit_expr_bin,
                          visit_expr_un,
                          visit_expr_id,
                          visit_stmt_let,
                          visit_stmt_proc,
                          visit_stmt_blk,
                          visit_stmt_return,
                          visit_stmt_exit);

        printf("=== DUMP OF AST ===\n");

        for (size_t i = 0; i < stmts.len; ++i) {
                stmts.data[i]->accept(stmts.data[i], v);
        }
}
