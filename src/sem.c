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
        // TODO: free() all symbols in popped scope.
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

static type *
binop(const expr  *lhs,
      const token *op,
      const expr  *rhs)
{
        if (op->ty != TOKEN_TYPE_PLUS
            && op->ty != TOKEN_TYPE_MINUS
            && op->ty != TOKEN_TYPE_ASTERISK
            && op->ty != TOKEN_TYPE_FORWARDSLASH) {
                forge_err_wargs("%sunsupported binary operator `%s`",
                                loc_err(op->loc), op->lx);
        }

        if (!type_is_compat(lhs->type, rhs->type)) {
                forge_err_wargs("%scannot perform binary operator `%s` on %s and %s",
                                loc_err(lhs->loc), op->lx, type_to_cstr(lhs->type), type_to_cstr(rhs->type));
        }

        return lhs->type;
}

static void *
visit_expr_bin(visitor *v, expr_bin *e)
{
        e->lhs->accept(e->lhs, v);
        e->rhs->accept(e->rhs, v);

        ((expr *)e)->type = binop(e->lhs, e->op, e->rhs);

        return NULL;
}

static void *
visit_expr_identifier(visitor *v, expr_identifier *e)
{
        symtbl *tbl = (symtbl *)v->context;

        if (!sym_exists_in_scope(tbl, e->id->lx)) {
                forge_err_wargs("%svariable `%s` is not defined", loc_err(((expr *)e)->loc), e->id->lx);
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
        // A procedure call has a left-hand-side expression
        // and parameters.
        // i.e.,
        //   sum(1, 2);
        //   ^^^ ^  ^
        // Doing a `proccall` operation `()` requires that the
        // left-hand-side expression must be of type `proc`.
        e->lhs->accept(e->lhs, v);

        // Let's assume that the left-hand-side it will always be a
        // a type of 'proc'.
        type *proc_rettype = ((expr *)e->lhs)->type;
        assert(proc_rettype->kind == TYPE_KIND_PROC);

        // Check number of arguments
        if (e->args.len != ((type_proc *)proc_rettype)->params->len) {
                forge_err_wargs("%sprocedure requires %zu arguments but %zu were given",
                                loc_err(((expr *)e)->loc), ((type_proc *)proc_rettype)->params->len, e->args.len);
        }

        // This expression's return type is the return type
        // of the function that we are calling.
        ((expr *)e)->type = ((type_proc *)proc_rettype)->rettype;

        // Go through each argument in the procedure call.
        for (size_t i = 0; i < e->args.len; ++i) {
                e->args.data[i]->accept(e->args.data[i], v);

                // Type check argument list
                const type *expected = ((type_proc *)proc_rettype)->params->data[i].type;
                const type *got = e->args.data[i]->type;
                if (!type_is_compat(got, expected)) {
                        forge_err_wargs("%stype mismatch, expected `%s` but the expression evaluates to `%s`",
                                        loc_err(e->args.data[i]->loc), type_to_cstr(expected), type_to_cstr(got));
                }
        }

        return NULL;
}

static void *
visit_stmt_let(visitor *v, stmt_let *s)
{
        symtbl *tbl = (symtbl *)v->context;

        // Check if the variable already exists
        if (sym_exists_in_scope(tbl, s->id->lx)) {
                forge_err_wargs("%svariable `%s` is already defined", loc_err(s->id->loc), s->id->lx);
        }

        insert_sym_into_scope(tbl, sym_alloc(s->id->lx, s->type));

        s->e->accept(s->e, v);

        // Typecheck the 'let' statement's given type
        // with the expression's type.
        // i.e.:
        //   let x: i32 = 1;
        //          ^^^   ^
        if (!type_is_compat(s->type, s->e->type)) {
                forge_err_wargs("%stype mismatch, expected `%s` but the expression evaluates to `%s`",
                                loc_err(s->id->loc), type_to_cstr(s->type), type_to_cstr(s->e->type));
        }

        return NULL;
}

static void *
visit_stmt_expr(visitor *v, stmt_expr *s)
{
        s->e->accept(s->e, v);
        return NULL;
}

static void *
visit_stmt_block(visitor *v, stmt_block *s)
{
        symtbl *tbl = (symtbl *)v->context;

        push_scope(tbl);

        // Iterate over all statements in the block.
        for (size_t i = 0; i < s->stmts.len; ++i) {
                stmt *stmt = s->stmts.data[i];
                stmt->accept(stmt, v);
        }

        pop_scope(tbl);

        return NULL;
}

static void *
visit_stmt_proc(visitor *v, stmt_proc *s)
{


        // Check if this procedure already exists.
        if (sym_exists_in_scope(tbl, s->id->lx)) {
                forge_err_wargs("%sprocecure `%s` is already defined", loc_err(s->id->loc), s->id->lx);
        }

        // Add procedure to the scope.
        type_proc *proc_ty = type_proc_alloc(s->type, &s->params);
        insert_sym_into_scope(tbl, sym_alloc(s->id->lx, (type *)proc_ty));

        // We are pushing scope here so that when this current
        // procedure is finished, the parameters are popped.
        push_scope(tbl);

        for (size_t i = 0; i < s->params.len; ++i) {
                if (sym_exists_in_scope(tbl, s->params.data[i].id->lx)) {
                        forge_err_wargs("%svariable `%s` is already defined",
                                        loc_err(s->params.data[i].id->loc), s->params.data[i].id->lx);
                }
                insert_sym_into_scope(tbl,
                                      sym_alloc(s->params.data[i].id->lx,
                                                s->params.data[i].type));
        }

        // We are currently inside of a procedure, keep track
        // of the return type so that the result of any 'return'
        // statements can be typechecked with the type of the procedure.
        tbl->proc.inproc = 1;
        tbl->proc.type = s->type;

        // Procedure body.
        s->blk->accept(s->blk, v);

        // No longer in a procedure.
        tbl->proc.type = NULL;
        tbl->proc.inproc = 0;

        // Remove parameters.
        pop_scope(tbl);

        return NULL;
}

static void *
visit_stmt_return(visitor *v, stmt_return *s)
{
        symtbl *tbl = (symtbl *)v->context;

        s->e->accept(s->e, v);

        if (tbl->proc.inproc) {
                if (!type_is_compat(s->e->type, tbl->proc.type)) {
                        forge_err_wargs("%scannot return type %s in a procedure returning %s",
                                        loc_err(s->e->loc),
                                        type_to_cstr(s->e->type),
                                        type_to_cstr(tbl->proc.type));

                }
        }

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
                .proc = {
                        .type = NULL,
                        .inproc = 0,
                },
        };

        // Need to immediately add a scope for global scope.
        dyn_array_append(tbl.scope, smap_create(NULL));

        visitor *v = sem_visitor_alloc(&tbl);

        for (size_t i = 0; i < p->stmts.len; ++i) {
                p->stmts.data[i]->accept(p->stmts.data[i], v);
        }

        return tbl;
}
