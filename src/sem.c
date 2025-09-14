#include "sem.h"
#include "visitor.h"
#include "mem.h"
#include "ds/smap.h"

#include <forge/array.h>
#include <forge/utils.h>
#include <forge/err.h>

#include <assert.h>
#include <string.h>

static void
push_scope(symtbl *tbl)
{
        dyn_array_append(tbl->scope, smap_create(NULL));
}

static void
pop_scope(symtbl *tbl)
{
        assert(tbl->scope.len > 0);
        --tbl->scope.len;
}

static int
proc_exists(const symtbl *tbl,
            const char   *id)
{
        for (size_t i = 0; i < tbl->procs.len; ++i) {
                if (!strcmp(id, tbl->procs.data[i]->id)) {
                        return 1;
                }
        }
        return 0;
}

static void
add_proc_to_scope(symtbl   *tbl,
                  sem_proc *proc)
{
        dyn_array_append(tbl->procs, proc);
}

static int
var_exists_in_scope(const symtbl *tbl,
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
insert_var_into_scope(symtbl *tbl, sem_var *sym)
{
        smap_insert(&tbl->scope.data[tbl->scope.len-1], sym->id, (void *)sym);
}

static sem_var *
sem_var_alloc(const char *id, const type *ty)
{
        sem_var *s = (sem_var *)alloc(sizeof(sem_var));
        s->id = id;
        s->ty = ty;
        return s;
}

static void *
visit_expr_bin(visitor *v, expr_bin *e)
{
        e->lhs->accept(e->lhs, v);
        e->rhs->accept(e->rhs, v);
        return NULL;
}

static void *
visit_expr_identifier(visitor *v, expr_identifier *e)
{
        if (!var_exists_in_scope((symtbl *)v->context, e->id->lx)
            && !proc_exists((symtbl *)v->context, e->id->lx)) {
                forge_err_wargs("%svariable `%s` is not defined", tokerr(e->id), e->id->lx);
        }

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
        e->lhs->accept(e->lhs, v);
        for (size_t i = 0; i < e->args.len; ++i) {
                e->args.data[i]->accept(e->args.data[i], v);
        }
        return NULL;
}

static void *
visit_stmt_let(visitor *v, stmt_let *s)
{
        if (var_exists_in_scope((symtbl *)v->context, s->id->lx)) {
                forge_err_wargs("%svariable `%s` is already defined", tokerr(s->id), s->id->lx);
        }
        insert_var_into_scope((symtbl *)v->context, sem_var_alloc(s->id->lx, s->type));

        s->e->accept(s->e, v);

        return NULL;
}

static void *
visit_stmt_expr(visitor *v, stmt_expr *s)
{
        return s->e->accept(s->e, v);
}

static void *
visit_stmt_block(visitor *v, stmt_block *s)
{
        push_scope((symtbl *)v->context);
        for (size_t i = 0; i < s->stmts.len; ++i) {
                s->stmts.data[i]->accept(s->stmts.data[i], v);
        }
        pop_scope((symtbl *)v->context);
        return NULL;
}

static void *
visit_stmt_proc(visitor *v, stmt_proc *s)
{
        if (proc_exists((symtbl *)v->context, s->id->lx)) {
                forge_err_wargs("%sprocecure `%s` is already defined", tokerr(s->id), s->id->lx);
        }

        sem_proc *proc = (sem_proc *)alloc(sizeof(sem_proc));
        proc->id = s->id->lx;
        proc->params = dyn_array_empty(sem_var_array);
        for (size_t i = 0; i < s->params.len; ++i) {
                dyn_array_append(proc->params,
                                 sem_var_alloc(s->params.data[i].id->lx,
                                               s->params.data[i].type));
        }
        proc->type = s->type;

        add_proc_to_scope((symtbl *)v->context, proc);
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
