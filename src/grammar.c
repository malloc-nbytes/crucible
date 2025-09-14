#include "grammar.h"
#include "mem.h"
#include "visitor.h"

expr_identifier *
expr_identifier_alloc(const token *id)
{
        expr_identifier *e = (expr_identifier *)
                alloc(sizeof(expr_identifier));
        e->base.kind = EXPR_KIND_IDENTIFIER;
        e->base.accept = accept_expr_identifier;
        e->id = id;
        return e;
}

expr_integer_literal *
expr_integer_literal_alloc(const token *i)
{
        expr_integer_literal *e = (expr_integer_literal *)
                alloc(sizeof(expr_integer_literal));
        e->base.kind = EXPR_KIND_INTEGER_LITERAL;
        e->base.accept = accept_expr_integer_literal;
        e->i = i;
        return e;
}

expr_string_literal *
expr_string_literal_alloc(const token *s)
{
        expr_string_literal *e = (expr_string_literal *)
                alloc(sizeof(expr_string_literal));
        e->base.kind = EXPR_KIND_STRING_LITERAL;
        e->base.accept = accept_expr_string_literal;
        e->s = s;
        return e;
}

expr_mut *
expr_mut_alloc(expr        *lhs,
               const token *op,
               expr        *rhs)
{
        expr_mut *e    = (expr_mut *)alloc(sizeof(expr_mut));
        e->base.kind   = EXPR_KIND_MUT;
        e->base.accept = NULL;
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
        e->base.kind   = EXPR_KIND_BINARY;
        e->base.accept = accept_expr_bin;
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
        e->base.kind   = EXPR_KIND_UNARY;
        e->base.accept = NULL;
        e->operand     = operand;
        e->op          = op;
        return e;
}

expr_proccall *
expr_proccall_alloc(expr       *lhs,
                    expr_array  args)
{
        expr_proccall *e = (expr_proccall *)alloc(sizeof(expr_proccall));
        e->base.kind     = EXPR_KIND_PROCCALL;
        e->base.accept   = accept_expr_proccall;
        e->lhs           = lhs;
        e->args          = args;
        return e;
}

stmt_let *
stmt_let_alloc(const token *id,
               const type  *type,
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
                type            *type,
                stmt            *blk)
{
        stmt_proc *proc   = alloc(sizeof(stmt_proc));
        proc->base.kind   = STMT_KIND_PROC;
        proc->base.accept = accept_stmt_proc;
        proc->export      = export;
        proc->id          = id;
        proc->params      = params;
        proc->type        = type;
        proc->blk         = blk;
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
