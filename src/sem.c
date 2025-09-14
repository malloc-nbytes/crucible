#include "sem.h"
#include "visitor.h"
#include "mem.h"
#include "ds/smap.h"

#include <forge/array.h>
#include <forge/utils.h>
#include <forge/err.h>

static int
id_exists_in_scope(const symtbl *tbl,
                   const char   *id)
{
        for (int i = tbl->scope.len-1; i >= 0; --i) {
                if (smap_has(&tbl->scope.data[i], id)) {
                        return 1;
                }
        }
        return 0;
}

static void
insert_id_into_scope(symtbl *tbl, sym *sym)
{
        smap_insert(&tbl->scope.data[tbl->scope.len-1], sym->id, (void *)sym);
}

static sym *
sym_alloc(const char *id, const type *ty)
{
        sym *s = (sym *)alloc(sizeof(sym));
        s->id = id;
        s->ty = ty;
        return s;
}

static void *
visit_expr_bin(visitor *v, expr_bin *e)
{
        NOOP(v, e);
        return NULL;
}

static void *
visit_expr_identifier(visitor *v, expr_identifier *e)
{
        NOOP(v, e);
        return NULL;
}

static void *
visit_expr_integer_literal(visitor *v, expr_integer_literal *e)
{
        NOOP(v, e);
        return NULL;
}

static void *
visit_expr_string_literal(visitor *v, expr_string_literal *e)
{
        NOOP(v, e);
        return NULL;
}

static void *
visit_expr_proccall(visitor *v, expr_proccall *e)
{
        NOOP(v, e);
        return NULL;
}

static void *
visit_stmt_let(visitor *v, stmt_let *s)
{
        if (id_exists_in_scope((symtbl *)v->context, s->id->lx)) {
                forge_err_wargs("variable `%s` is already defined", s->id->lx);
        }
        insert_id_into_scope((symtbl *)v->context, sym_alloc(s->id->lx, s->type));

        return NULL;
}

static void *
visit_stmt_expr(visitor *v, stmt_expr *s)
{
        NOOP(v, s);
        return NULL;
}

static void *
visit_stmt_block(visitor *v, stmt_block *s)
{
        for (size_t i = 0; i < s->stmts.len; ++i) {
                s->stmts.data[i]->accept(s->stmts.data[i], v);
        }
        return NULL;
}

static void *
visit_stmt_proc(visitor *v, stmt_proc *s)
{
        return s->blk->accept(s->blk, v);
}

static void *
visit_stmt_return(visitor *v, stmt_return *s)
{
        NOOP(v, s);
        return NULL;
}

static void *
visit_stmt_exit(visitor *v, stmt_exit *s)
{
        NOOP(v, s);
        return NULL;
}

static visitor *
sem_visitor_alloc(symtbl *tbl)
{
        return visitor_alloc(
                (void *)tbl,
                visit_expr_bin,
                visit_expr_identifier,
                visit_expr_integer_literal,
                visit_expr_string_literal,
                visit_expr_proccall,
                visit_stmt_let,
                visit_stmt_expr,
                visit_stmt_block,
                visit_stmt_proc,
                visit_stmt_return,
                visit_stmt_exit
        );
}

symtbl
sem_analysis(program *p)
{
        symtbl tbl = (symtbl) {
                .scope = dyn_array_empty(smap_array),
        };
        dyn_array_append(tbl.scope, smap_create(NULL));

        visitor *v = sem_visitor_alloc(&tbl);

        for (size_t i = 0; i < p->stmts.len; ++i) {
                p->stmts.data[i]->accept(p->stmts.data[i], v);
        }

        return tbl;
}
