#include "ast.h"
#include "visitor.h"
#include "utils.h"
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
visit_stmt_let(visitor *v, stmt_let *s)
{
        ast_context *ctx = (ast_context *)v->ctx;

        spaces(ctx);
        printf("LET %s = ", strv_scstr(s->id));

        ++ctx->depth;
        s->e->accept(s->e, v);
        --ctx->depth;

        return NULL;
}

static void *
visit_expr_id(visitor *v, expr_id *e)
{
        ast_context *ctx = (ast_context *)v->ctx;
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
        NOOP(v, e);
        TODO("");
        return NULL;
}

static void *
visit_expr_intlit(visitor *v, expr_intlit *e)
{
        ast_context *ctx = (ast_context *)v->ctx;
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
                          visit_stmt_let);

        for (size_t i = 0; i < stmts.len; ++i) {
                stmts.data[i]->accept(stmts.data[i], v);
                putchar('\n');
        }
}
