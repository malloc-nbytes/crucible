#include "visitor.hpp"

visitor
visitor_create(void                             *context,
               visit_expr_int_sig                visit_expr_int,
               visit_expr_identifier_sig         visit_expr_identifier,
               visit_stmt_expr_sig               visit_stmt_expr,
               visit_stmt_let_sig                visit_stmt_let,
               visit_stmt_proc_sig               visit_stmt_proc)
{
        return visitor {
                .context               = context,
                .visit_expr_int        = visit_expr_int,
                .visit_expr_identifier = visit_expr_identifier,
                .visit_stmt_expr       = visit_stmt_expr,
                .visit_stmt_let        = visit_stmt_let,
                .visit_stmt_proc       = visit_stmt_proc,
        };
}

void *
accept_expr_int(expr *e, visitor *v)
{
        if (v->visit_expr_int)
                return v->visit_expr_int(v, (expr_int *)e);
        return NULL;
}

void *
accept_expr_identifier(expr *e, visitor *v)
{
        if (v->visit_expr_identifier)
                return v->visit_expr_identifier(v, (expr_identifier *)e);
        return NULL;
}

void *
accept_stmt_expr(stmt *s, visitor *v)
{
        if (v->visit_stmt_expr)
                return v->visit_stmt_expr(v, (stmt_expr *)s);
        return NULL;
}

void *
accept_stmt_let(stmt *s, visitor *v)
{
        if (v->visit_stmt_let)
                return v->visit_stmt_let(v, (stmt_let *)s);
        return NULL;
}

void *
accept_stmt_proc(stmt *s, visitor *v)
{
        if (v->visit_stmt_proc)
                return v->visit_stmt_proc(v, (stmt_proc *)s);
        return NULL;
}
