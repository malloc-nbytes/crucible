#include "visitor.h"
#include "mem.h"

#include <forge/utils.h>

visitor *
visitor_alloc(void                           *ctx,
              visit_expr_bin_sig              visit_expr_bin,
              visit_expr_identifier_sig       visit_expr_identifier,
              visit_expr_integer_literal_sig  visit_expr_integer_literal,
              visit_expr_string_literal_sig   visit_expr_string_literal,
              visit_expr_proccall_sig         visit_expr_proccall,
              visit_stmt_let_sig              visit_stmt_let,
              visit_stmt_expr_sig             visit_stmt_expr,
              visit_stmt_block_sig            visit_stmt_block,
              visit_stmt_proc_sig             visit_stmt_proc,
              visit_stmt_return_sig           visit_stmt_return,
              visit_stmt_exit_sig             visit_stmt_exit,
              visit_stmt_extern_proc_sig      visit_stmt_extern_proc,
              visit_stmt_if_sig               visit_stmt_if) {

        visitor *v = (visitor *)alloc(sizeof(visitor));

        v->context = ctx;

        v->visit_expr_bin             = visit_expr_bin;
        v->visit_expr_identifier      = visit_expr_identifier;
        v->visit_expr_integer_literal = visit_expr_integer_literal;
        v->visit_expr_string_literal  = visit_expr_string_literal;
        v->visit_expr_proccall        = visit_expr_proccall;

        v->visit_stmt_let             = visit_stmt_let;
        v->visit_stmt_expr            = visit_stmt_expr;
        v->visit_stmt_block           = visit_stmt_block;
        v->visit_stmt_proc            = visit_stmt_proc;
        v->visit_stmt_return          = visit_stmt_return;
        v->visit_stmt_exit            = visit_stmt_exit;
        v->visit_stmt_extern_proc     = visit_stmt_extern_proc;
        v->visit_stmt_if              = visit_stmt_if;

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

void *
accept_expr_string_literal(expr *e, visitor *v)
{
        if (v->visit_expr_string_literal) {
                return v->visit_expr_string_literal(v, (expr_string_literal *)e);
        }
        return NULL;
}

void *
accept_expr_proccall(expr *e, visitor *v)
{
        if (v->visit_expr_proccall) {
                return v->visit_expr_proccall(v, (expr_proccall *)e);
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

void *
accept_stmt_expr(stmt *s, visitor *v)
{
        if (v->visit_stmt_expr) {
                return v->visit_stmt_expr(v, (stmt_expr *)s);
        }
        return NULL;
}

void *
accept_stmt_block(stmt *s, visitor *v)
{
        if (v->visit_stmt_block) {
                return v->visit_stmt_block(v, (stmt_block *)s);
        }
        return NULL;
}

void *
accept_stmt_proc(stmt *s, visitor *v)
{
        if (v->visit_stmt_proc) {
                return v->visit_stmt_proc(v, (stmt_proc *)s);
        }
        return NULL;
}

void *
accept_stmt_return(stmt *s, visitor *v)
{
        if (v->visit_stmt_return) {
                return v->visit_stmt_return(v, (stmt_return *)s);
        }
        return NULL;
}

void *
accept_stmt_exit(stmt *s, visitor *v)
{
        if (v->visit_stmt_exit) {
                return v->visit_stmt_exit(v, (stmt_exit *)s);
        }
        return NULL;
}

void *
accept_stmt_extern_proc(stmt *s, visitor *v)
{
        if (v->visit_stmt_extern_proc) {
                return v->visit_stmt_extern_proc(v, (stmt_extern_proc *)s);
        }
        return NULL;
}

void *
accept_stmt_if(stmt *s, visitor *v)
{
        if (v->visit_stmt_if) {
                return v->visit_stmt_if(v, (stmt_if *)s);
        }
        return NULL;
}
