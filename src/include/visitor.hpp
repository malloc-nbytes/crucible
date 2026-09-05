#ifndef VISITOR_H_INCLUDED
#define VISITOR_H_INCLUDED

#include "grammar.hpp"

typedef void *(*visit_expr_int_sig)(visitor *, expr_int *);
typedef void *(*visit_expr_identifier_sig)(visitor *, expr_identifier *);
typedef void *(*visit_expr_unary_sig)(visitor *, expr_unary *);
typedef void *(*visit_expr_binary_sig)(visitor *, expr_binary *);

typedef void *(*visit_stmt_expr_sig)(visitor *, stmt_expr *);
typedef void *(*visit_stmt_let_sig)(visitor *, stmt_let *);
typedef void *(*visit_stmt_proc_sig)(visitor *, stmt_proc *);
typedef void *(*visit_stmt_block_sig)(visitor *, stmt_block *);

typedef struct visitor {
        void                            *context;
        visit_expr_int_sig               visit_expr_int;
        visit_expr_identifier_sig        visit_expr_identifier;
        visit_expr_unary_sig             visit_expr_unary;
        visit_expr_binary_sig            visit_expr_binary;

        visit_stmt_expr_sig     visit_stmt_expr;
        visit_stmt_let_sig      visit_stmt_let;
        visit_stmt_proc_sig     visit_stmt_proc;
        visit_stmt_block_sig    visit_stmt_block;
} visitor;

visitor visitor_create(void                             *context,
                       visit_expr_int_sig                visit_expr_int,
                       visit_expr_identifier_sig         visit_expr_identifier,
                       visit_expr_unary_sig              visit_expr_unary,
                       visit_expr_binary_sig             visit_expr_binary,
                       visit_stmt_expr_sig               visit_stmt_expr,
                       visit_stmt_let_sig                visit_stmt_let,
                       visit_stmt_proc_sig               visit_stmt_proc,
                       visit_stmt_block_sig              visit_stmt_block);

void    *accept_expr_int(expr *e, visitor *v);
void    *accept_expr_identifier(expr *e, visitor *v);
void    *accept_expr_unary(expr *e, visitor *v);
void    *accept_expr_binary(expr *e, visitor *v);

void    *accept_stmt_expr(stmt *s, visitor *v);
void    *accept_stmt_let(stmt *s, visitor *v);
void    *accept_stmt_proc(stmt *s, visitor *v);
void    *accept_stmt_block(stmt *s, visitor *v);

#endif // VISITOR_H_INCLUDED
