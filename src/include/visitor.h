#ifndef VISITOR_H_INCLUDED
#define VISITOR_H_INCLUDED

#include "grammar.h"

typedef struct visitor {
        void *context;

        void *(*visit_expr_bin)(visitor *v, expr_bin *e);
        void *(*visit_expr_identifier)(visitor *v, expr_identifier *e);
        void *(*visit_expr_integer_literal)(visitor *v, expr_integer_literal *e);

        void *(*visit_stmt_let)(visitor *v, stmt_let *s);
} visitor;

void *accept_expr_bin(expr *e, visitor *v);
void *accept_expr_integer_literal(expr *e, visitor *v);
void *accept_expr_identifier(expr *e, visitor *v);

void *accept_stmt_let(stmt *s, visitor *v);

#endif // VISITOR_H_INCLUDED
