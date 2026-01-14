#ifndef VISITOR_H_INCLUDED
#define VISITOR_H_INCLUDED

#include "grammar.h"

typedef void *(*visit_expr_intlit_sig)(visitor *v, expr_intlit *e);
typedef void *(*visit_expr_bin_sig)(visitor *v, expr_bin *e);
typedef void *(*visit_expr_un_sig)(visitor *v, expr_un *e);

typedef void *(*visit_stmt_let_sig)(visitor *v, stmt_let *s);

typedef struct visitor {
        void *ctx;
        visit_expr_intlit_sig visit_expr_intlit;
        visit_expr_bin_sig visit_expr_bin;
        visit_expr_un_sig visit_expr_un;
        visit_stmt_let_sig visit_stmt_let;
} visitor;

visitor *visitor_alloc(void *ctx,
                       visit_expr_intlit_sig visit_expr_intlit,
                       visit_expr_bin_sig visit_expr_bin,
                       visit_expr_un_sig visit_expr_un,
                       visit_stmt_let_sig visit_stmt_let);

void *accept_expr_intlit(expr *e, visitor *v);
void *accept_expr_bin(expr *e, visitor *v);
void *accept_expr_un(expr *e, visitor *v);

void *accept_stmt_let(stmt *s, visitor *v);

#endif // VISITOR_H_INCLUDED
