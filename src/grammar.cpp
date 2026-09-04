#include "grammar.hpp"
#include "visitor.hpp"

expr_int *
expr_int_alloc(const token *const i)
{
        expr_int *e;

        e              = new expr_int;
        e->i           = i;
        e->base.k      = EXPR_KIND_INT;
        e->base.loc    = i->loc;
        e->base.ty     = NULL;
        e->base.accept = accept_expr_int;

        return e;
}

expr_identifier *
expr_identifier_alloc(const token *const id)
{
}

stmt_expr *
stmt_expr_alloc(expr *e)
{
}

stmt_let *
stmt_let_alloc(const token *const       id,
               type                    *ty,
               expr                    *e)
{
}

#undef IGNORE_VISITOR_FORWARD
