#include "visitor.h"
#include "mem.h"

#include <forge/utils.h>

visitor *
visitor_alloc(void                     *ctx,
        visit_expr_bin_sig              visit_expr_bin,
        visit_expr_identifier_sig       visit_expr_identifier,
        visit_expr_integer_literal_sig  visit_expr_integer_literal,
        visit_stmt_let_sig              visit_stmt_let) {

        visitor *v = (visitor *)alloc(sizeof(visitor));

        v->context = ctx;

        v->visit_expr_bin             = visit_expr_bin;
        v->visit_expr_identifier      = visit_expr_identifier;
        v->visit_expr_integer_literal = visit_expr_integer_literal;
        v->visit_stmt_let             = visit_stmt_let;

        return v;
}

// EXPRESSION ACCEPTORS

void *
accept_expr_bin(expr *e, visitor *v)
{
        if (v->visit_expr_bin) {
                return v->visit_expr_bin(v, (expr_bin *)e);
        }
        return NULL;
}

void *
accept_expr_integer_literal(expr *e, visitor *v)
{
        if (v->visit_expr_integer_literal) {
                return v->visit_expr_integer_literal(v, (expr_integer_literal *)e);
        }
        return NULL;
}

void *
accept_expr_identifier(expr *e, visitor *v)
{
        if (v->visit_expr_identifier) {
                return v->visit_expr_identifier(v, (expr_identifier *)e);
        }
        return NULL;
}

// STATEMENT ACCEPTORS

void *
accept_stmt_let(stmt *s, visitor *v)
{
        if (v->visit_stmt_let) {
                return v->visit_stmt_let(v, (stmt_let *)s);
        }
        return NULL;
}

// EXPRESSION VISITORS

void *
visitor_expr_bin(visitor *v, expr_bin *e)
{
        e->lhs->accept(e->lhs, v);
        return e->rhs->accept(e->rhs, v);
}

void *
visitor_expr_integer_literal(visitor *v, expr_integer_literal *e)
{
        NOOP(v, e);
        return NULL;
}

void *
visitor_expr_identifier(visitor *v, expr_identifier *e)
{
        NOOP(v, e);
        return NULL;
}

// STATEMENT VISITORS

void *
visit_stmt_let(visitor *v, stmt_let *s)
{
        return s->e->accept(s->e, v);
}
