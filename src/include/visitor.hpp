#ifndef VISITOR_H_INCLUDED
#define VISITOR_H_INCLUDED

#include "grammar.hpp"

typedef void *(*visit_expr_int_sig)(visitor *, expr_int *);
typedef void *(*visit_expr_identifier_sig)(visitor *, expr_identifier *);

typedef void *(*visit_stmt_expr_sig)(visitor *, stmt_expr *);
typedef void *(*visit_stmt_let_sig)(visitor *, stmt_let *);

typedef struct visitor {
        void                            *context;
        visit_expr_int_sig               visit_expr_int;
        visit_expr_identifier_sig        visit_expr_identifier;

        visit_stmt_expr_sig     visit_stmt_expr;
        visit_stmt_let_sig      visit_stmt_let;
} visitor;

visitor visitor_create(void                             *context,
                       visit_expr_int_sig                visit_expr_int,
                       visit_expr_identifier_sig         visit_expr_identifier,
                       visit_stmt_expr_sig               visit_stmt_expr,
                       visit_stmt_let_sig                visit_stmt_let);

void    *accept_expr_int(expr *e, visitor *v);
void    *accept_expr_identifier(expr *e, visitor *v);
void    *accept_stmt_expr(stmt *s, visitor *v);
void    *accept_stmt_let(stmt *s, visitor *v);

#endif // VISITOR_H_INCLUDED
