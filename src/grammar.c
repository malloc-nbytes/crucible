#include "grammar.h"
#include "mem.h"

stmt_let *
stmt_let_alloc(const token *id,
               const type  *type,
               expr        *e)
{
        stmt_let *let  = (stmt_let *)alloc(sizeof(stmt_let));
        let->base.kind = STMT_KIND_LET;
        let->id        = id;
        let->type      = type;
        let->e         = e;
        return let;
}

stmt_expr *
stmt_expr_alloc(expr *e)
{
        stmt_expr *expr = (stmt_expr *)alloc(sizeof(stmt_expr));
        expr->base.kind = STMT_KIND_EXPR;
        expr->e         = e;
        return expr;
}

stmt_proc *
stmt_proc_alloc(const token     *id,
                parameter_array  params,
                type            *type,
                stmt            *blk)
{
        stmt_proc *proc = alloc(sizeof(stmt_proc));
        proc->base.kind = STMT_KIND_PROC;
        proc->id        = id;
        proc->params    = params;
        proc->type      = type;
        proc->blk       = blk;
        return proc;
}
