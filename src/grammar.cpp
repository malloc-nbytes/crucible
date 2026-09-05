#include "grammar.hpp"
#include "visitor.hpp"

expr_int *
expr_int_alloc(const token *const i)
{
        return new expr_int {
                .base = {
                        .k      = EXPR_KIND_INT,
                        .loc    = i->loc,
                        .ty     = NULL,
                        .accept = accept_expr_int,
                },
                .i = i,
        };
}

expr_identifier *
expr_identifier_alloc(const token *const id)
{
        return new expr_identifier {
                .base = {
                        .k      = EXPR_KIND_IDENTIFIER,
                        .loc    = id->loc,
                        .ty     = NULL,
                        .accept = accept_expr_identifier,
                },
                .id  = id,
                .sym = NULL,
        };
}

expr_unary *
expr_unary_alloc(const token    *op,
                 expr           *rhs)
{
        return new expr_unary {
                .base = {
                        .k      = EXPR_KIND_UNARY,
                        .loc    = op->loc,
                        .ty     = NULL,
                        .accept = accept_expr_unary,
                },
                .op  = op,
                .rhs = rhs,
        };
}

expr_binary *
expr_binary_alloc(expr        *lhs,
                  const token *op,
                  expr        *rhs)
{
        return new expr_binary {
                .base = {
                        .k = EXPR_KIND_BINARY,
                        .loc    = lhs->loc,
                        .ty     = NULL,
                        .accept = accept_expr_binary,
                },
                .lhs = lhs,
                .op  = op,
                .rhs = rhs,
        };
}

stmt_expr *
stmt_expr_alloc(expr *e)
{
        return new stmt_expr {
                .base = {
                        .k      = STMT_KIND_EXPR,
                        .loc    = e->loc,
                        .accept = accept_stmt_expr,
                },
                .e = e,
        };
}

stmt_let *
stmt_let_alloc(location                  loc,
               const token *const        id,
               type                     *ty,
               expr                     *e)
{
        return new stmt_let {
                .base = {
                        .k      = STMT_KIND_LET,
                        .loc    = loc,
                        .accept = accept_stmt_let,
                },
                .id  = id,
                .ty  = ty,
                .e   = e,
                .sym = NULL,
        };
}

stmt_proc *
stmt_proc_alloc(location                         loc,
                uint32_t                         bits,
                token                           *id,
                std::vector<procp>               params,
                type                            *rty,
                std::optional<stmt *>            body)
{
        return new stmt_proc {
                .base = {
                        .k      = STMT_KIND_PROC,
                        .loc    = loc,
                        .accept = accept_stmt_proc,
                },
                .bits   = bits,
                .id     = id,
                .params = params,
                .rty    = rty,
                .body   = body,
                .sym    = NULL,
        };
}

stmt_block *
stmt_block_alloc(location               loc,
                 std::vector<stmt *>    stmts)
{
        return new stmt_block {
                .base = {
                        .k      = STMT_KIND_BLOCK,
                        .loc    = loc,
                        .accept = accept_stmt_block,
                },
                .stmts = stmts,
        };
}
