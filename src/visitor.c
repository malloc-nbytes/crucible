#include "visitor.h"

// EXPRESSION VISITORS
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

// STATEMENT VISITORS
void *
accept_stmt_let(stmt *s, visitor *v)
{
        if (v->visit_stmt_let) {
                return v->visit_stmt_let(v, (stmt_let *)s);
        }
        return NULL;
}
