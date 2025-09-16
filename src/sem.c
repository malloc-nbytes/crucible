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
sym_exists_in_scope(const symtbl *tbl,
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
insert_sym_into_scope(symtbl *tbl, sym *sym)
{
        smap_insert(&tbl->scope.data[tbl->scope.len-1], sym->id, (void *)sym);
}

static sym *
get_sym_from_scope(symtbl *tbl, const char *id)
{
        for (int i = tbl->scope.len-1; i >= 0; --i) {
                sym *sym = NULL;
                if ((sym = smap_get(&tbl->scope.data[i], id)) != NULL) {
                        return sym;
                }
        }

        forge_err_wargs("get_sym_from_scope(): could not find variable %s", id);
        return NULL; // unreachable
}

static sym *
sym_alloc(const char *id, type *ty)
{
        sym *s = (sym *)alloc(sizeof(sym));
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
        symtbl *tbl = (symtbl *)v->context;
        if (!sym_exists_in_scope(tbl, e->id->lx)) {
                forge_err_wargs("%svariable `%s` is not defined", tokerr(e->id), e->id->lx);
        }

        sym *sym = get_sym_from_scope(tbl, e->id->lx);
        ((expr *)e)->type = sym->ty;

        return NULL;
}

static void *
visit_expr_integer_literal(visitor *v, expr_integer_literal *e)
{
        NOOP(v);
        ((expr *)e)->type = (type *)type_i32_alloc();
        return NULL;
}

static void *
visit_expr_string_literal(visitor *v, expr_string_literal *e)
{
        NOOP(v);
        ((expr *)e)->type = (type *)type_ptr_alloc((type *)type_u8_alloc());
        return NULL;
}

static void *
visit_expr_proccall(visitor *v, expr_proccall *e)
{
        e->lhs->accept(e->lhs, v);

        type *proc_rettype = ((expr *)e->lhs)->type;
        assert(proc_rettype->kind == TYPE_KIND_PROC);

        if (e->args.len != ((type_proc *)proc_rettype)->params->len) {
                forge_err_wargs("error: procedure requires %zu arguments but %zu were given",
                                ((type_proc *)proc_rettype)->params->len, e->args.len);
        }

        ((expr *)e)->type = ((type_proc *)proc_rettype)->rettype;
        for (size_t i = 0; i < e->args.len; ++i) {
                e->args.data[i]->accept(e->args.data[i], v);

                const type *expected = ((type_proc *)proc_rettype)->params->data[i].type;
                const type *got = e->args.data[i]->type;
                if (!type_is_compat(got, expected)) {
                        forge_err_wargs("error: type mismatch, expected `%s` but the expression evaluates to `%s`",
                                        type_to_cstr(expected), type_to_cstr(got));
                }
        }

        return NULL;
}

static void *
visit_stmt_let(visitor *v, stmt_let *s)
{
        if (sym_exists_in_scope((symtbl *)v->context, s->id->lx)) {
                forge_err_wargs("%svariable `%s` is already defined", tokerr(s->id), s->id->lx);
        }
        insert_sym_into_scope((symtbl *)v->context, sym_alloc(s->id->lx, s->type));

        s->e->accept(s->e, v);

        if (!type_is_compat(s->type, s->e->type)) {
                forge_err_wargs("%stype mismatch, expected `%s` but the expression evaluates to `%s`",
                                tokerr(s->id), type_to_cstr(s->type), type_to_cstr(s->e->type));
        }

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
        symtbl *tbl = (symtbl *)v->context;

        if (sym_exists_in_scope(tbl, s->id->lx)) {
                forge_err_wargs("%sprocecure `%s` is already defined", tokerr(s->id), s->id->lx);
        }

        type_proc *proc_ty = type_proc_alloc(s->type, &s->params);
        insert_sym_into_scope(tbl, sym_alloc(s->id->lx, (type *)proc_ty));

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
