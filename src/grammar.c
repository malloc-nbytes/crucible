#include "grammar.h"
#include "mem.h"
#include "visitor.h"

#include <forge/array.h>

static expr
init_expr_kind(expr_kind kind,
               void *(accept)(expr *, visitor *))
{
        expr e = {0};
        e.kind = kind;
        e.accept = accept;
        e.type = NULL;
        return e;
}

expr_identifier *
expr_identifier_alloc(const token *id)
{
        expr_identifier *e = (expr_identifier *)
                alloc(sizeof(expr_identifier));
        e->base = init_expr_kind(EXPR_KIND_IDENTIFIER, accept_expr_identifier);
        e->id = id;
        e->resolved = NULL;
        return e;
}

expr_integer_literal *
expr_integer_literal_alloc(const token *i)
{
        expr_integer_literal *e = (expr_integer_literal *)
                alloc(sizeof(expr_integer_literal));
        e->base = init_expr_kind(EXPR_KIND_INTEGER_LITERAL, accept_expr_integer_literal);
        e->i = i;
        return e;
}

expr_string_literal *
expr_string_literal_alloc(const token *s)
{
        expr_string_literal *e = (expr_string_literal *)
                alloc(sizeof(expr_string_literal));
        e->base = init_expr_kind(EXPR_KIND_STRING_LITERAL, accept_expr_string_literal);
        e->s = s;
        return e;
}

expr_mut *
expr_mut_alloc(expr        *lhs,
               const token *op,
               expr        *rhs)
{
        expr_mut *e    = (expr_mut *)alloc(sizeof(expr_mut));
        e->base        = init_expr_kind(EXPR_KIND_MUT, accept_expr_mut);
        e->lhs         = lhs;
        e->op          = op;
        e->rhs         = rhs;
        return e;
}

expr_bin *
expr_bin_alloc(expr        *lhs,
               const token *op,
               expr        *rhs)
{
        expr_bin *e    = (expr_bin *)alloc(sizeof(expr_bin));
        e->base        = init_expr_kind(EXPR_KIND_BINARY, accept_expr_bin);
        e->lhs         = lhs;
        e->op          = op;
        e->rhs         = rhs;
        return e;
}

expr_un *
expr_un_alloc(expr        *operand,
              const token *op)
{
        expr_un *e     = (expr_un *)alloc(sizeof(expr_un));
        e->base        = init_expr_kind(EXPR_KIND_UNARY, NULL);
        e->operand     = operand;
        e->op          = op;
        return e;
}

expr_proccall *
expr_proccall_alloc(expr       *lhs,
                    expr_array  args)
{
        expr_proccall *e = (expr_proccall *)alloc(sizeof(expr_proccall));
        e->base          = init_expr_kind(EXPR_KIND_PROCCALL, accept_expr_proccall);
        e->lhs           = lhs;
        e->args          = args;
        return e;
}

expr_brace_init *
expr_brace_init_alloc(token_array ids,
                      expr_array  exprs)
{
        expr_brace_init *e = (expr_brace_init *)alloc(sizeof(expr_brace_init));
        e->base            = init_expr_kind(EXPR_KIND_BRACE_INIT, accept_expr_brace_init);
        e->ids             = ids;
        e->exprs           = exprs;
        e->struct_id       = NULL; // to be resolved in lexer
        return e;
}

expr_namespace *
expr_namespace_alloc(const token *namespace,
                     const token *id)
{
        expr_namespace *e = (expr_namespace *)alloc(sizeof(expr_namespace));
        e->base           = init_expr_kind(EXPR_KIND_PROCCALL, accept_expr_namespace);
        e->namespace      = namespace;
        e->id             = id;
        return e;
}

stmt_let *
stmt_let_alloc(const token *id,
               type        *type,
               expr        *e)
{
        stmt_let *let    = (stmt_let *)alloc(sizeof(stmt_let));
        let->base.kind   = STMT_KIND_LET;
        let->base.accept = accept_stmt_let;
        let->id          = id;
        let->type        = type;
        let->e           = e;
        return let;
}

stmt_expr *
stmt_expr_alloc(expr *e)
{
        stmt_expr *expr   = (stmt_expr *)alloc(sizeof(stmt_expr));
        expr->base.kind   = STMT_KIND_EXPR;
        expr->base.accept = accept_stmt_expr;
        expr->e           = e;
        return expr;
}

stmt_proc *
stmt_proc_alloc(int              export,
                const token     *id,
                parameter_array  params,
                int              variadic,
                type            *type,
                stmt            *blk)
{
        stmt_proc *proc   = alloc(sizeof(stmt_proc));
        proc->base.kind   = STMT_KIND_PROC;
        proc->base.accept = accept_stmt_proc;
        proc->export      = export;
        proc->id          = id;
        proc->params      = params;
        proc->variadic    = variadic;
        proc->type        = type;
        proc->blk         = blk;
        proc->rsp         = 0;
        return proc;
}

stmt_extern_proc *
stmt_extern_proc_alloc(const token *id,
                       parameter_array params,
                       int variadic,
                       type *type)
{
        stmt_extern_proc *proc = alloc(sizeof(stmt_extern_proc));
        proc->base.kind        = STMT_KIND_EXTERN_PROC;
        proc->base.accept      = accept_stmt_extern_proc;
        proc->id               = id;
        proc->params           = params;
        proc->variadic         = variadic;
        proc->type             = type;
        return proc;
}

stmt_block *
stmt_block_alloc(stmt_array stmts)
{
        stmt_block *blk  = alloc(sizeof(stmt_block));
        blk->base.kind   = STMT_KIND_BLOCK;
        blk->base.accept = accept_stmt_block;
        blk->stmts       = stmts;
        return blk;
}

stmt_return *
stmt_return_alloc(expr *e)
{
        stmt_return *ret  = alloc(sizeof(stmt_return));
        ret->base.kind    = STMT_KIND_RETURN;
        ret->base.accept  = accept_stmt_return;
        ret->e            = e;
        return ret;
}

stmt_exit *
stmt_exit_alloc(expr *e)
{
        stmt_exit *ex    = alloc(sizeof(stmt_exit));
        ex->base.kind    = STMT_KIND_EXIT;
        ex->base.accept  = accept_stmt_exit;
        ex->e            = e;
        return ex;
}

stmt_if *
stmt_if_alloc(expr *e, stmt *then, stmt *else_)
{
        stmt_if *s      = alloc(sizeof(stmt_if));
        s->base.kind    = STMT_KIND_IF;
        s->base.accept  = accept_stmt_if;
        s->e            = e;
        s->then         = then;
        s->else_        = else_;
        return s;
}

stmt_while *
stmt_while_alloc(expr *e, stmt *body)
{
        stmt_while *s    = alloc(sizeof(stmt_while));
        s->base.kind     = STMT_KIND_WHILE;
        s->base.accept   = accept_stmt_while;
        s->e             = e;
        s->body          = body;
        s->asm_begin_lbl = NULL;
        s->asm_end_lbl   = NULL;
        return s;
}

stmt_for *
stmt_for_alloc(stmt *init,
               expr *e,
               expr *after,
               stmt *body)
{
        stmt_for *s      = alloc(sizeof(stmt_for));
        s->base.kind     = STMT_KIND_FOR;
        s->base.accept   = accept_stmt_for;
        s->init          = init;
        s->e             = e;
        s->after         = after;
        s->body          = body;
        s->asm_begin_lbl = NULL;
        s->asm_end_lbl   = NULL;
        return s;
}

stmt_break *
stmt_break_alloc(void)
{
        stmt_break *s      = alloc(sizeof(stmt_break));
        s->base.kind       = STMT_KIND_BREAK;
        s->base.accept     = accept_stmt_break;
        s->resolved_parent = NULL;
        return s;
}

stmt_continue *
stmt_continue_alloc(void)
{
        stmt_continue *s   = alloc(sizeof(stmt_continue));
        s->base.kind       = STMT_KIND_CONTINUE;
        s->base.accept     = accept_stmt_continue;
        s->resolved_parent = NULL;
        return s;
}

stmt_struct *
stmt_struct_alloc(const token     *id,
                  parameter_array  members)
{
        stmt_struct *s     = alloc(sizeof(stmt_struct));
        s->base.kind       = STMT_KIND_STRUCT;
        s->base.accept     = accept_stmt_struct;
        s->id              = id;
        s->members         = members;
        return s;
}

stmt_module *
stmt_module_alloc(const token *modname)
{
        stmt_module *s     = alloc(sizeof(stmt_module));
        s->base.kind       = STMT_KIND_MODULE;
        s->base.accept     = accept_stmt_module;
        s->modname         = modname;
        return s;
}

stmt_import *
stmt_import_alloc(char *filepath, int local)
{
        stmt_import *s     = alloc(sizeof(stmt_import));
        s->base.kind       = STMT_KIND_IMPORT;
        s->base.accept     = accept_stmt_import;
        s->filepath        = filepath;
        s->local           = local;
        return s;
}
