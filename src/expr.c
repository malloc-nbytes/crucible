#include "grammar.h"
#include "mem.h"

expr_ident *
expr_ident_alloc(token *id)
{
        expr_ident *e;

        e            = (expr_ident *)alloc(sizeof(expr_ident));
        e->id        = id;
        e->base.kind = EXPR_KIND_IDENT;
        e->base.loc  = (loc){0};

        return e;
}

expr_intlit *
expr_intlit_alloc(token *i)
{
        expr_intlit *e;

        e            = (expr_intlit *)alloc(sizeof(expr_intlit));
        e->i         = i;
        e->base.kind = EXPR_KIND_INTLIT;
        e->base.loc  = (loc){0};

        return e;
}

expr_strlit *
expr_strlit_alloc(token *s)
{
        expr_strlit *e;

        e            = (expr_strlit *)alloc(sizeof(expr_strlit));
        e->s         = s;
        e->base.kind = EXPR_KIND_STRLIT;
        e->base.loc  = (loc){0};

        return e;
}

expr_bin *
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
        e->base.loc  = (loc){0};

        return e;
}

expr_un *
expr_un_alloc(token *prefix, expr *rhs)
{
        expr_un *e;

        e            = (expr_un *)alloc(sizeof(expr_un));
        e->prefix    = prefix;
        e->e         = rhs;
        e->base.kind = EXPR_KIND_UN;
        e->base.loc  = (loc){0};

        return e;
}

