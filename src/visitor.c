#include "visitor.h"
#include "mem.h"

visitor *
visitor_alloc(void                  *ctx,
              visit_expr_intlit_sig  visit_expr_intlit,
              visit_expr_bin_sig     visit_expr_bin,
              visit_expr_un_sig      visit_expr_un,
              visit_expr_id_sig      visit_expr_id,
              visit_stmt_let_sig     visit_stmt_let,
              visit_stmt_proc_sig    visit_stmt_proc,
              visit_stmt_blk_sig     visit_stmt_blk,
              visit_stmt_return_sig  visit_stmt_return,
              visit_stmt_exit_sig    visit_stmt_exit)
{
        visitor *v           = (visitor *)s_malloc(sizeof(visitor));
        v->ctx               = ctx;
        v->visit_expr_intlit = visit_expr_intlit;
        v->visit_expr_bin    = visit_expr_bin;
        v->visit_expr_un     = visit_expr_un;
        v->visit_expr_id     = visit_expr_id;

        v->visit_stmt_let    = visit_stmt_let;
        v->visit_stmt_proc   = visit_stmt_proc;
        v->visit_stmt_blk    = visit_stmt_blk;
        v->visit_stmt_return = visit_stmt_return;
        v->visit_stmt_exit   = visit_stmt_exit;

        return v;
}

#define ACCEPT_IMPL(base, sub)                                          \
        void *                                                          \
        accept_##base##_##sub(base *k, visitor *v)                      \
        {                                                               \
                return v->visit_##base##_##sub                          \
                        ? v->visit_##base##_##sub(v, (base##_##sub *)k) \
                        : NULL;                                         \
        }


ACCEPT_IMPL(expr, intlit);
ACCEPT_IMPL(expr, bin);
ACCEPT_IMPL(expr, un);
ACCEPT_IMPL(expr, id);

ACCEPT_IMPL(stmt, let);
ACCEPT_IMPL(stmt, proc);
ACCEPT_IMPL(stmt, blk);
ACCEPT_IMPL(stmt, return);
ACCEPT_IMPL(stmt, exit);
