#include "visitor.h"
#include "mem.h"

visitor *
visitor_alloc(void                  *ctx,
              visit_expr_intlit_sig  visit_expr_intlit,
              visit_stmt_let_sig     visit_stmt_let)
{
        visitor *v = (visitor *)s_malloc(sizeof(visitor));
        v->ctx = ctx;
        v->visit_expr_intlit = visit_expr_intlit;
        v->visit_stmt_let = visit_stmt_let;
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
ACCEPT_IMPL(stmt, let);
