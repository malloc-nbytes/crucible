#include "grammar.h"
#include "mem.h"
#include "loc.h"
#include "visitor.h"

#include <assert.h>

#define SETBASE(g, k, a)                        \
        do {                                    \
                g->base.loc = (loc){0};         \
                g->base.kind = k;               \
                g->base.accept = a;             \
        } while (0)

expr_intlit *
expr_intlit_alloc(int i, arena *a)
{
        expr_intlit *e = (expr_intlit *)arena_alloc(a, sizeof(expr_intlit));
        e->i = i;
        SETBASE(e, EXPR_KIND_INTLIT, accept_expr_intlit);
        return e;
}

stmt_let *
stmt_let_alloc(strv   id,
               type  *type,
               expr  *e,
               arena *a)
{
        stmt_let *s = (stmt_let *)arena_alloc(a, sizeof(stmt_let));
        s->id = id;
        s->type = type;
        s->e = e;
        SETBASE(s, STMT_KIND_LET, accept_stmt_let);
        return s;
}
