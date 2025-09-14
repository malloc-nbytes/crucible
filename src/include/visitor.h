#ifndef VISITOR_H_INCLUDED
#define VISITOR_H_INCLUDED

#include "grammar.h"

typedef void *(*visit_expr_bin_sig)(visitor *v, expr_bin *e);
typedef void *(*visit_expr_identifier_sig)(visitor *v, expr_identifier *e);
typedef void *(*visit_expr_integer_literal_sig)(visitor *v, expr_integer_literal *e);

typedef void *(*visit_stmt_let_sig)(visitor *v, stmt_let *s);

typedef struct visitor {
        void *context;

        visit_expr_bin_sig             visit_expr_bin;
        visit_expr_identifier_sig      visit_expr_identifier;
        visit_expr_integer_literal_sig visit_expr_integer_literal;

        visit_stmt_let_sig             visit_stmt_let;
} visitor;

visitor *visitor_alloc(
        void *ctx,
        visit_expr_bin_sig             visit_expr_bin,
        visit_expr_identifier_sig      visit_expr_identifier,
        visit_expr_integer_literal_sig visit_expr_integer_literal,
        visit_stmt_let_sig             visit_stmt_let
);

void *accept_expr_bin(expr *e, visitor *v);
void *accept_expr_integer_literal(expr *e, visitor *v);
void *accept_expr_identifier(expr *e, visitor *v);

void *accept_stmt_let(stmt *s, visitor *v);

#endif // VISITOR_H_INCLUDED
