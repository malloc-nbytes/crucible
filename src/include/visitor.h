#ifndef VISITOR_H_INCLUDED
#define VISITOR_H_INCLUDED

#include "grammar.h"

typedef void *(*visit_expr_bin_sig)(visitor *v, expr_bin *e);
typedef void *(*visit_expr_identifier_sig)(visitor *v, expr_identifier *e);
typedef void *(*visit_expr_integer_literal_sig)(visitor *v, expr_integer_literal *e);
typedef void *(*visit_expr_string_literal_sig)(visitor *v, expr_string_literal *e);
typedef void *(*visit_expr_proccall_sig)(visitor *v, expr_proccall *e);
typedef void *(*visit_expr_mut_sig)(visitor *v, expr_mut *e);
typedef void *(*visit_expr_brace_init_sig)(visitor *v, expr_brace_init *e);
typedef void *(*visit_expr_namespace_sig)(visitor *v, expr_namespace *e);

typedef void *(*visit_stmt_let_sig)(visitor *v, stmt_let *s);
typedef void *(*visit_stmt_expr_sig)(visitor *v, stmt_expr *s);
typedef void *(*visit_stmt_block_sig)(visitor *v, stmt_block *s);
typedef void *(*visit_stmt_proc_sig)(visitor *v, stmt_proc *s);
typedef void *(*visit_stmt_return_sig)(visitor *v, stmt_return *s);
typedef void *(*visit_stmt_exit_sig)(visitor *v, stmt_exit *s);
typedef void *(*visit_stmt_extern_proc_sig)(visitor *v, stmt_extern_proc *s);
typedef void *(*visit_stmt_if_sig)(visitor *v, stmt_if *s);
typedef void *(*visit_stmt_while_sig)(visitor *v, stmt_while *s);
typedef void *(*visit_stmt_for_sig)(visitor *v, stmt_for *s);
typedef void *(*visit_stmt_break_sig)(visitor *v, stmt_break *s);
typedef void *(*visit_stmt_continue_sig)(visitor *v, stmt_continue *s);
typedef void *(*visit_stmt_struct_sig)(visitor *v, stmt_struct *s);
typedef void *(*visit_stmt_module_sig)(visitor *v, stmt_module *s);
typedef void *(*visit_stmt_import_sig)(visitor *v, stmt_import *s);

typedef struct visitor {
        void *context;

        visit_expr_bin_sig             visit_expr_bin;
        visit_expr_identifier_sig      visit_expr_identifier;
        visit_expr_integer_literal_sig visit_expr_integer_literal;
        visit_expr_string_literal_sig  visit_expr_string_literal;
        visit_expr_proccall_sig        visit_expr_proccall;
        visit_expr_mut_sig             visit_expr_mut;
        visit_expr_brace_init_sig      visit_expr_brace_init;
        visit_expr_namespace_sig       visit_expr_namespace;

        visit_stmt_let_sig             visit_stmt_let;
        visit_stmt_expr_sig            visit_stmt_expr;
        visit_stmt_block_sig           visit_stmt_block;
        visit_stmt_proc_sig            visit_stmt_proc;
        visit_stmt_return_sig          visit_stmt_return;
        visit_stmt_exit_sig            visit_stmt_exit;
        visit_stmt_extern_proc_sig     visit_stmt_extern_proc;
        visit_stmt_if_sig              visit_stmt_if;
        visit_stmt_while_sig           visit_stmt_while;
        visit_stmt_for_sig             visit_stmt_for;
        visit_stmt_break_sig           visit_stmt_break;
        visit_stmt_continue_sig        visit_stmt_continue;
        visit_stmt_struct_sig          visit_stmt_struct;
        visit_stmt_module_sig          visit_stmt_module;
        visit_stmt_import_sig          visit_stmt_import;
} visitor;

visitor *visitor_alloc(
        void *ctx,
        visit_expr_bin_sig             visit_expr_bin,
        visit_expr_identifier_sig      visit_expr_identifier,
        visit_expr_integer_literal_sig visit_expr_integer_literal,
        visit_expr_string_literal_sig  visit_expr_string_literal,
        visit_expr_proccall_sig        visit_expr_proccall,
        visit_expr_mut_sig             visit_expr_mut,
        visit_expr_brace_init_sig      visit_expr_brace_init,
        visit_expr_namespace_sig       visit_expr_namespace,
        visit_stmt_let_sig             visit_stmt_let,
        visit_stmt_expr_sig            visit_stmt_expr,
        visit_stmt_block_sig           visit_stmt_block,
        visit_stmt_proc_sig            visit_stmt_proc,
        visit_stmt_return_sig          visit_stmt_return,
        visit_stmt_exit_sig            visit_stmt_exit,
        visit_stmt_extern_proc_sig     visit_stmt_extern_proc,
        visit_stmt_if_sig              visit_stmt_if,
        visit_stmt_while_sig           visit_stmt_while,
        visit_stmt_for_sig             visit_stmt_for,
        visit_stmt_break_sig           visit_stmt_break,
        visit_stmt_continue_sig        visit_stmt_continue,
        visit_stmt_struct_sig          visit_stmt_struct,
        visit_stmt_module_sig          visit_stmt_module,
        visit_stmt_import_sig          visit_stmt_import
);

void *accept_expr_bin(expr *e, visitor *v);
void *accept_expr_integer_literal(expr *e, visitor *v);
void *accept_expr_identifier(expr *e, visitor *v);
void *accept_expr_string_literal(expr *e, visitor *v);
void *accept_expr_proccall(expr *e, visitor *v);
void *accept_expr_mut(expr *e, visitor *v);
void *accept_expr_brace_init(expr *e, visitor *v);
void *accept_expr_namespace(expr *e, visitor *v);

void *accept_stmt_let(stmt *s, visitor *v);
void *accept_stmt_expr(stmt *s, visitor *v);
void *accept_stmt_block(stmt *s, visitor *v);
void *accept_stmt_proc(stmt *s, visitor *v);
void *accept_stmt_return(stmt *s, visitor *v);
void *accept_stmt_exit(stmt *s, visitor *v);
void *accept_stmt_extern_proc(stmt *s, visitor *v);
void *accept_stmt_if(stmt *s, visitor *v);
void *accept_stmt_while(stmt *s, visitor *v);
void *accept_stmt_for(stmt *s, visitor *v);
void *accept_stmt_break(stmt *s, visitor *v);
void *accept_stmt_continue(stmt *s, visitor *v);
void *accept_stmt_struct(stmt *s, visitor *v);
void *accept_stmt_module(stmt *s, visitor *v);
void *accept_stmt_import(stmt *s, visitor *v);

#endif // VISITOR_H_INCLUDED
