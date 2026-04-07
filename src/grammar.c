#include "grammar.h"
#include "mem.h"

expr_ident
expr_ident_alloc(token *id)
{
        expr_ident *e;

        e            = (expr_ident *)alloc(sizeof(expr_ident));
        e->id        = id;
        e->base.kind = EXPR_KIND_IDENT;

        return e;
}

expr_intlit
expr_intlit_alloc(token *i)
{
        expr_intlit *e;

        e            = (expr_intlit *)alloc(sizeof(expr_intlit));
        e->i         = i;
        e->base.kind = EXPR_KIND_INTLIT;

        return e;
}

expr_strlit
expr_strlit_alloc(token *s)
{
        expr_strlit *e;

        e            = (expr_strlit *)alloc(sizeof(expr_strlit));
        e->s         = s;
        e->base.kind = EXPR_KIND_STRLIT;

        return e;
}

expr_bin
expr_bin_alloc(expr  *lhs,
               token *op,
               expr  *rhs)
{
        expr_bin *e;

        e            = (expr_bin *)alloc(sizeof(expr_bin));
        e->lhs       = lhs;
        e->op        = op;
        e->rhs       = rhs;
        e->base.kind = EXPR_KIND_BIN;

        return e;
}

expr_bin
expr_bin_alloc(token *prefix, expr *rhs)
{
        expr_un *e;

        e            = (expr_un *)alloc(sizeof(expr_un));
        e->prefix    = prefix;
        e->rhs       = rhs;
        e->base.kind = EXPR_KIND_UN;

        return e;
}
