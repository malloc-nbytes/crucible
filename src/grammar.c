#include "grammar.h"
#include "mem.h"
#include "loc.h"
#include "visitor.h"

#include <assert.h>

#define SETBASE(g, k, a)                        \
        do {                                    \
                g->base.loc = (loc){0};         \
                g->base.kind = k;               \
                g->base.accept = a;             \
        } while (0)

expr_intlit *
expr_intlit_alloc(int i, arena *a)
{
        expr_intlit *e = (expr_intlit *)arena_alloc(a, sizeof(expr_intlit));
        e->i           = i;
        SETBASE(e, EXPR_KIND_INTLIT, accept_expr_intlit);
        return e;
}

expr_bin *
expr_bin_alloc(expr        *lhs,
               const token *op,
               expr        *rhs,
               arena       *a)
{
        expr_bin *e = (expr_bin *)arena_alloc(a, sizeof(expr_bin));
        e->lhs      = lhs;
        e->op       = op;
        e->rhs      = rhs;
        SETBASE(e, EXPR_KIND_BIN, accept_expr_bin);
        return e;
}

expr_un *
expr_un_alloc(const token *op, expr *rhs, arena *a)
{
        expr_un *e = (expr_un *)arena_alloc(a, sizeof(expr_un));
        e->op      = op;
        e->rhs     = rhs;
        SETBASE(e, EXPR_KIND_UN, accept_expr_un);
        return e;
}

expr_id *
expr_id_alloc(const token *i, arena *a)
{
        expr_id *e = (expr_id *)arena_alloc(a, sizeof(expr_id));
        e->i       = i;
        SETBASE(e, EXPR_KIND_ID, accept_expr_id);
        return e;
}

stmt_let *
stmt_let_alloc(const token *id,
               type        *type,
               expr        *e,
               arena       *a)
{
        stmt_let *s = (stmt_let *)arena_alloc(a, sizeof(stmt_let));
        s->id       = id;
        s->type     = type;
        s->e        = e;
        SETBASE(s, STMT_KIND_LET, accept_stmt_let);
        return s;
}
