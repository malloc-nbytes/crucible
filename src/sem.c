#include "sem.h"
#include "visitor.h"
#include "mem.h"
#include "ds/smap.h"
#include "grammar.h"
#include "lexer.h"
#include "io.h"
#include "utils.h"

#include <forge/array.h>
#include <forge/utils.h>
#include <forge/err.h>
#include <forge/str.h>

#include <assert.h>
#include <string.h>
#include <stdarg.h>

void
pusherr(symtbl *tbl, loc loc, const char *fmt, ...)
{
        static char buf[512] = {0};
        const char *prefix = loc_err(loc);
        va_list args;

        snprintf(buf, sizeof(buf), "%s", prefix);

        size_t prefix_n = strlen(buf);
        va_start(args, fmt);
        vsnprintf(buf + prefix_n, sizeof(buf) - prefix_n, fmt, args);
        va_end(args);

        dyn_array_append(tbl->errs, strdup(buf));
}

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
sym_alloc(symtbl     *tbl,
          const char *id,
          type       *ty,
          int         extern_)
{
        sym *s          = (sym *)alloc(sizeof(sym));
        s->id           = id;
        s->ty           = ty;
        s->stack_offset = tbl->stack_offset + ty->sz;
        s->extern_      = extern_;
        s->modname      = tbl->modname;

        return s;
}

static void
coerce_integer_literal(symtbl    *tbl,
                       expr      *e,
                       type_kind  to)
{
        assert(e);
        assert(e->type);

        if (e->type->kind == to) return;

        if (e->type->kind <= TYPE_KIND_NUMBER) {
                // TODO: Write custom free() for each type
                free(e->type);
                e->type = (type *)type_sizet_alloc();
                return;
        }

        pusherr(tbl, e->loc, "could not coerce type `%s` to type `%s`",
                type_to_cstr(e->type), type_kind_to_cstr(to));
}

static type *
binop(symtbl      *tbl,
      expr        *lhs,
      const token *op,
      expr        *rhs)
{
        type *res = NULL;

        if (op->ty >= TOKEN_TYPE_BINOP_LEN || op->ty <= TOKEN_TYPE_OTHER_LEN) {
                forge_err_wargs("%sunsupported binary operator `%s`",
                                loc_err(op->loc), op->lx);
                (type *)type_unknown_alloc();
        }

        if (op->ty == TOKEN_TYPE_DOUBLE_EQUALS
            || op->ty == TOKEN_TYPE_BANG_EQUALS
            || op->ty == TOKEN_TYPE_GREATERTHAN
            || op->ty == TOKEN_TYPE_LESSTHAN
            || op->ty == TOKEN_TYPE_GREATERTHAN_EQUALS
            || op->ty == TOKEN_TYPE_LESSTHAN_EQUALS
            || op->ty == TOKEN_TYPE_DOUBLE_AMPERSAND
            || op->ty == TOKEN_TYPE_DOUBLE_PIPE) {
                res = (type *)type_bool_alloc();
        } else {
                res = lhs->type;
        }

        if (lhs->type->kind == TYPE_KIND_PTR && rhs->type->kind <= TYPE_KIND_NUMBER) {
                coerce_integer_literal(tbl, rhs, TYPE_KIND_SIZET);
                return lhs->type;
        }

        if (rhs->type->kind == TYPE_KIND_PTR && lhs->type->kind <= TYPE_KIND_NUMBER) {
                coerce_integer_literal(tbl, lhs, TYPE_KIND_SIZET);
                return rhs->type;
        }

        if (!type_is_compat(&lhs->type, &rhs->type)) {
                pusherr(tbl, lhs->loc,
                        "cannot perform binary operator `%s` on %s and %s",
                        op->lx,
                        type_to_cstr(lhs->type), type_to_cstr(rhs->type));
                return (type *)type_unknown_alloc();
        }

        assert(res);
        return res;
}

static void *
visit_expr_bin(visitor *v, expr_bin *e)
{
        symtbl *tbl = (symtbl *)v->context;

        e->lhs->accept(e->lhs, v);
        e->rhs->accept(e->rhs, v);

        ((expr *)e)->type = binop(tbl, e->lhs, e->op, e->rhs);

        return NULL;
}

static void *
visit_expr_identifier(visitor *v, expr_identifier *e)
{
        symtbl *tbl = (symtbl *)v->context;

        if (!sym_exists_in_scope(tbl, e->id->lx)) {
                pusherr(tbl, ((expr *)e)->loc, "variable `%s` is not defined", e->id->lx);
                ((expr *)e)->type = (type *)type_unknown_alloc();
                return NULL;
        } else {
                sym *sym = get_sym_from_scope(tbl, e->id->lx);
                ((expr *)e)->type = sym->ty;
                e->resolved = sym;
        }

        return NULL;
}

static void *
visit_expr_integer_literal(visitor *v, expr_integer_literal *e)
{
        symtbl *tbl = (symtbl *)v->context;
        if (tbl->expty) {
                ((expr *)e)->type = tbl->expty;
        } else {
                ((expr *)e)->type = (type *)type_number_alloc();
        }
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
        symtbl *tbl = (symtbl *)v->context;

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
        type *lhs_ty = ((expr *)e->lhs)->type;
        if (lhs_ty->kind != TYPE_KIND_PROC && lhs_ty->kind != TYPE_KIND_PROCPTR) {
                pusherr(tbl, e->lhs->loc, "cannot perform a procedure call on type %s", type_to_cstr(lhs_ty));
                ((expr *)e)->type = (type *)type_unknown_alloc();
                return NULL;
        }

        int export = 0;
        int variadic = 0;
        type_array params = dyn_array_empty(type_array);
        type *rettype = NULL;

        if (lhs_ty->kind == TYPE_KIND_PROC) {
                type_proc *proc_ty = (type_proc *)lhs_ty;
                export = proc_ty->export;
                variadic = proc_ty->variadic;
                type_get_types_from_proc(proc_ty, &params, &rettype);
        } else {
                type_procptr *proc_ty = (type_procptr *)lhs_ty;
                export = 0;
                variadic = proc_ty->variadic;
                params = proc_ty->param_types;
                rettype = proc_ty->rettype;
        }

        if (tbl->context_switch && !export) {
                pusherr(tbl, ((expr *)e)->loc,
                        "procedure `%s::%s()` is not marked as export",
                        tbl->modname, ((type_proc *)e->lhs->type)->id);
                ((expr *)e)->type = (type *)type_unknown_alloc();
                tbl->context_switch = 0;
                return NULL;
        }
        tbl->context_switch = 0;

        // Check number of arguments
        if (e->args.len != params.len) {
                if (e->args.len >= params.len && variadic) {
                        goto ok;
                }

                pusherr(tbl, ((expr *)e)->loc,
                        "procedure requires %zu arguments but %zu were given",
                        params.len, e->args.len);
        }
 ok:

        // This expression's return type is the return type
        // of the function that we are calling.
        ((expr *)e)->type = rettype;

        // Go through each argument in the procedure call.
        for (size_t i = 0; i < e->args.len; ++i) {
                expr *arg = e->args.data[i];

                /* arg->accept(arg, v); */
                if (!arg->type) arg->accept(arg, v);

                // Type check argument list
                if (i < params.len) {
                        type *expected = params.data[i];
                        type *got = arg->type;

                        assert(expected);
                        assert(got);

                        if (!type_is_compat(&got, &expected)) {
                                pusherr(tbl, arg->loc,
                                        "type mismatch, expected `%s` but the expression evaluates to `%s`",
                                        type_to_cstr(expected), type_to_cstr(got));
                        }
                }
        }

        return NULL;
}

static void *
visit_expr_mut(visitor *v, expr_mut *e)
{
        symtbl *tbl = (symtbl *)v->context;

        e->lhs->accept(e->lhs, v);
        e->rhs->accept(e->rhs, v);

        if (e->op->ty == TOKEN_TYPE_PLUS_EQUALS
            || e->op->ty == TOKEN_TYPE_MINUS_EQUALS
            || e->op->ty == TOKEN_TYPE_ASTERISK_EQUALS
            || e->op->ty == TOKEN_TYPE_FORWARDSLASH_EQUALS) {
                if ((e->lhs->type->kind == TYPE_KIND_LIST
                     || e->lhs->type->kind == TYPE_KIND_PTR)
                    && e->rhs->type->kind <= TYPE_KIND_NUMBER) {
                        coerce_integer_literal(tbl, e->rhs, TYPE_KIND_SIZET);
                } else if ((e->rhs->type->kind == TYPE_KIND_LIST
                            || e->rhs->type->kind == TYPE_KIND_PTR)
                           && e->lhs->type->kind <= TYPE_KIND_NUMBER) {
                        coerce_integer_literal(tbl, e->lhs, TYPE_KIND_SIZET);
                }

        }

        ((expr *)e)->type = e->lhs->type;

        return NULL;
}

static void *
visit_expr_brace_init(visitor *v, expr_brace_init *e)
{
        assert(e->struct_id);
        assert(e->ids.len == e->exprs.len);

        symtbl *tbl = (symtbl *)v->context;

        const char *struct_id = e->struct_id->lx;

        if (!sym_exists_in_scope(tbl, struct_id)) {
                pusherr(tbl, ((expr *)e)->loc, "struct `%s` is not defined", struct_id);
                return NULL;
        }

        const sym *struct_sym = get_sym_from_scope(tbl, struct_id);
        assert(struct_sym);

        // Should not be needed but doesn't hurt.
        if (struct_sym->ty->kind != TYPE_KIND_STRUCT) {
                pusherr(tbl, ((expr *)e)->loc,
                        "cannot use a brace initalizer on type `%s`",
                        type_to_cstr(struct_sym->ty));
                ((expr *)e)->type = (type *)type_unknown_alloc();
                return NULL;
        }

        type_struct *struct_ty = (type_struct *)struct_sym->ty;

        // Make sure members length matches the struct's members.
        if (e->ids.len != struct_ty->members->len) {
                pusherr(tbl, e->struct_id->loc,
                        "struct `%s` requires %zu members but %zu were supplied",
                        struct_id, struct_ty->members->len, e->ids.len);
                ((expr *)e)->type = (type *)type_unknown_alloc();
                return NULL;
        }

        // Verify members and their expressions.
        for (size_t i = 0; i < e->ids.len; ++i) {
                const char *got = e->ids.data[i]->lx;
                const char *expected = struct_ty->members->data[i].id->lx;
                if (strcmp(got, expected)) {
                        pusherr(tbl, e->ids.data[i]->loc,
                                "expected member ID `%s` but got `%s`",
                                expected, got);
                }
                e->exprs.data[i]->accept(e->exprs.data[i], v);
        }

        ((expr *)e)->type = struct_sym->ty;

        e->resolved_syms = (sym_array *)alloc(sizeof(sym_array));
        *e->resolved_syms = dyn_array_empty(sym_array);

        return NULL;
}

static void *
visit_expr_namespace(visitor *v, expr_namespace *e)
{
        symtbl *tbl = (symtbl *)v->context;
        symtbl *other = NULL;

        for (size_t i = 0; i < tbl->imports.len; ++i) {
                symtbl *t = tbl->imports.data[i];
                assert(t);
                if (!strcmp(e->namespace->lx, t->modname)) {
                        other = t;
                        break;
                }
        }

        if (!other) {
                pusherr(tbl, ((expr *)e)->loc, "module `%s` was not found", e->namespace->lx);
                ((expr *)e)->type = (type *)type_unknown_alloc();
                return NULL;
        }

        // Evaluate the arguments in *this* context!
        if (e->e->kind == EXPR_KIND_PROCCALL) {
                expr_proccall *pc = (expr_proccall *)e->e;
                for (size_t i = 0; i < pc->args.len; ++i) {
                        pc->args.data[i]->accept(pc->args.data[i], v);
                }
        }

        other->context_switch = 1;
        v->context = (void *)other;
        e->e->accept(e->e, v);

        other->context_switch = 0;
        v->context = (void *)tbl;

        if (other->errs.len > 0) {
                for (size_t i = 0; i < other->errs.len; ++i) {
                        fprintf(stderr, "%s\n", other->errs.data[i]);
                }
                exit(1);
        }

        ((expr *)e)->type = e->e->type;

        return NULL;
}

static void *
visit_expr_arrayinit(visitor *v, expr_arrayinit *e)
{
        symtbl *tbl = (symtbl *)v->context;
        type *elemty = NULL;

        // Reverse the order of the expressions for ASM generation.
        for (size_t i = 0; i < e->exprs.len/2; ++i) {
                expr *tmp = e->exprs.data[i];
                e->exprs.data[i] = e->exprs.data[e->exprs.len-i-1];
                e->exprs.data[e->exprs.len-i-1] = tmp;
        }

        // Evaluate all expressions and make sure all types are the same.
        for (size_t i = 0; i < e->exprs.len; ++i) {
                e->exprs.data[i]->accept(e->exprs.data[i], v);
                if (!elemty) {
                        elemty = e->exprs.data[i]->type;
                } else if (!type_is_compat(&elemty, &e->exprs.data[i]->type)) {
                        pusherr(tbl, e->exprs.data[i]->loc,
                                "type mismatch, expected `%s` but got `%s`",
                                type_to_cstr(elemty), type_to_cstr(e->exprs.data[i]->type));
                }

                /* if (tbl->proc.inproc) { */
                /*         tbl->proc.rsp += e->exprs.data[i]->type->sz; */
                /* } */
        }

        ((expr *)e)->type = (type *)type_list_alloc(elemty, (int)e->exprs.len);

        return NULL;
}

static void *
visit_expr_index(visitor *v, expr_index *e)
{
        symtbl *tbl = (symtbl *)v->context;

        e->lhs->accept(e->lhs, v);

        if (e->lhs->type->kind != TYPE_KIND_LIST
            && e->lhs->type->kind != TYPE_KIND_PTR) {
                pusherr(tbl, e->lhs->loc, "index operations are only permitted for lists and pointers");
                ((expr *)e)->type = (type *)type_unknown_alloc();
                return NULL;
        }

        e->idx->accept(e->idx, v);

        if (e->idx->type->kind > TYPE_KIND_NUMBER) {
                pusherr(tbl, e->idx->loc,
                        "array indexs can only be numbers, not `%s`",
                        type_to_cstr(e->idx->type));
        }

        coerce_integer_literal(tbl, e->idx, TYPE_KIND_SIZET);

        if (e->idx->type->sz != 8) {
                pusherr(tbl, e->idx->loc, "array indices are allowed only for size_t numbers");
        }

        if (e->lhs->type->kind == TYPE_KIND_LIST) {
                ((expr *)e)->type = ((type_list *)e->lhs->type)->elemty;
        } else {
                ((expr *)e)->type = ((type_ptr *)e->lhs->type)->to;
        }

        return NULL;
}

static void *
visit_expr_un(visitor *v, expr_un *e)
{
        // TODO: check op with rhs
        symtbl *tbl = (symtbl *)v->context;
        e->rhs->accept(e->rhs, v);

        // Check for address operator, assign to array type.
        if (e->op->ty == TOKEN_TYPE_AMPERSAND) {
                ((expr *)e)->type = (type *)type_ptr_alloc(e->rhs->type);
        } else if (e->op->ty == TOKEN_TYPE_ASTERISK) {
                if (e->rhs->type->kind == TYPE_KIND_PTR) {
                        ((expr *)e)->type = ((type_ptr *)e->rhs->type)->to;
                } else if (e->rhs->type->kind == TYPE_KIND_LIST) {
                        ((expr *)e)->type = ((type_list *)e->rhs->type)->elemty;
                } else {
                        pusherr(tbl, e->op->loc, "cannot dereference type of `%s`",
                                type_to_cstr(e->rhs->type));
                        goto bad;
                }
        } else if (e->op->ty == TOKEN_TYPE_BANG) {
                ((expr *)e)->type = (type *)type_bool_alloc();
        } else {
                ((expr *)e)->type = e->rhs->type;
        }
        return NULL;

 bad:
        ((expr *)(e))->type = (type *)type_unknown_alloc();
        return NULL;
}

static void *
visit_expr_character_literal(visitor *v, expr_character_literal *e)
{
        NOOP(v);
        ((expr *)e)->type = (type *)type_u8_alloc();
        return NULL;
}

static void *
visit_expr_cast(visitor *v, expr_cast *e)
{
        // TODO: make sure the types can be casted.
        e->rhs->accept(e->rhs, v);
        ((expr *)e)->type = e->to;
        return NULL;
}

static void *
visit_expr_bool_literal(visitor *v, expr_bool_literal *e)
{
        NOOP(v, e);
        ((expr *)e)->type = (type *)type_bool_alloc();
        return NULL;
}

static void *
visit_expr_null(visitor *v, expr_null *e)
{
        NOOP(v, e);

        ((expr *)e)->type = (type *)type_ptr_alloc(NULL);

        return NULL;
}

static void *
visit_stmt_let(visitor *v, stmt_let *s)
{
        symtbl *tbl = (symtbl *)v->context;

        // Check if the variable already exists
        if (sym_exists_in_scope(tbl, s->id->lx)) {
                pusherr(tbl, s->id->loc, "variable `%s` is already defined", s->id->lx);
                return NULL;
        }

        if (s->type->kind != TYPE_KIND_PTR) {
                // Doing this does *not* work for pointer
                // arithmetic i.e.,
                //   let p1: i32* = null;
                //   let p2: i32* = p1-1;
                tbl->expty = type_get_lowest(s->type);
                s->e->accept(s->e, v);
                tbl->expty = NULL;
        } else {
                s->e->accept(s->e, v);
        }

        sym *sym = sym_alloc(tbl, s->id->lx, s->type, 0);

        insert_sym_into_scope(tbl, sym);
        tbl->stack_offset += sym->ty->sz;

        if (s->type->kind == TYPE_KIND_PTR
            && s->e->type->kind == TYPE_KIND_PTR) {
                type_ptr *t1 = (type_ptr *)s->type;
                type_ptr *t2 = (type_ptr *)s->e->type;
                if (!t2->to) t2->to = t1->to;
        }

        // Check for a zeroed array initializer. Set appropriate
        // lengths if necessary.
        if (s->type->kind == TYPE_KIND_LIST
            && s->e->type->kind == TYPE_KIND_LIST) {
                type_list *let_ty   = (type_list *)s->type;
                type_list *e_ty     = (type_list *)s->e->type;
                expr_arrayinit *init = (expr_arrayinit *)s->e;

                if (init->zeroed && let_ty->len != -1/*array has length decl.*/) {
                        // We are zeroing the array, expand the array initializer
                        // to all zeros.
                        e_ty->len = let_ty->len;
                } else if (let_ty->len == -1) {
                        // Not zeroing, length not declared, set the declared
                        // length to the expression's length.
                        let_ty->len = e_ty->len;
                        init->zeroed = 0;
                } else {
                        // Not zeroing, length is declared, wait until
                        // typecheck to make sure they are compatible.
                        init->zeroed = 0;
                }

                ((expr_arrayinit *)s->e)->stack_offset_base = sym->stack_offset;
        }

        // Increase the procedures RSP register subtraction amount.
        if (tbl->proc.inproc) {
                if (sym->ty->kind == TYPE_KIND_LIST) {
                        // Add the values of all type sizes for arrays.
                        tbl->stack_offset += ((type_list *)sym->ty)->elemty->sz * ((type_list *)sym->ty)->len;
                        tbl->proc.rsp += ((type_list *)sym->ty)->elemty->sz * ((type_list *)sym->ty)->len;
                }
                tbl->proc.rsp += sym->ty->sz;
        }

        //coerce(&s->type, &s->e->type);

        // Typecheck the 'let' statement's given type
        // with the expression's type.
        // i.e.:
        //   let x: i32 = 1;
        //          ^^^   ^
        if (!type_is_compat(&s->type, &s->e->type)) {
                pusherr(tbl, s->id->loc,
                        "type mismatch, expected `%s` but the expression evaluates to `%s`",
                        type_to_cstr(s->type), type_to_cstr(s->e->type));
                return NULL;
        }

        s->resolved = sym;

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
        symtbl *tbl = (symtbl *)v->context;

        // Check if this procedure already exists.
        if (sym_exists_in_scope(tbl, s->id->lx)) {
                pusherr(tbl, s->id->loc, "procecure `%s` is already defined", s->id->lx);
                return NULL;
        }

        // Add procedure to the scope.
        type_proc *proc_ty = type_proc_alloc(s->id->lx, s->type, &s->params, s->variadic, s->export, 0);
        sym *proc_sym = sym_alloc(tbl, s->id->lx, (type *)proc_ty, 0);
        insert_sym_into_scope(tbl, proc_sym);

        // Add exported procedures to the export_syms table
        // for ASM generation 'extern' section.
        if (s->export) {
                dyn_array_append(tbl->export_syms, proc_sym);
        }

        // We are pushing scope here so that when this current
        // procedure is finished, the parameters are popped.
        push_scope(tbl);

        for (size_t i = 0; i < s->params.len; ++i) {
                if (sym_exists_in_scope(tbl, s->params.data[i].id->lx)) {
                        pusherr(tbl, s->params.data[i].id->loc,
                                "variable `%s` is already defined",
                                s->params.data[i].id->lx);
                        return NULL;
                }

                sym *param = sym_alloc(tbl, s->params.data[i].id->lx, s->params.data[i].type, 0);
                insert_sym_into_scope(tbl, param);
                tbl->stack_offset += param->ty->sz;
                s->params.data[i].resolved = param;

                // Consider the procedures parameter type sizes
                // to subtract from the RSP regsister in ASM generation.
                tbl->proc.rsp += s->params.data[i].type->sz;
        }

        // We are currently inside of a procedure, keep track
        // of the return type so that the result of any 'return'
        // statements can be typechecked with the type of the procedure.
        tbl->proc.inproc = 1;
        tbl->proc.type = proc_ty->rettype;

        // Procedure body.
        s->blk->accept(s->blk, v);

        // Make sure the last statement is an exit statement
        if (s->type->kind == TYPE_KIND_NORETURN) {
                if (s->blk->kind == STMT_KIND_BLOCK) {
                        stmt_block *blk = (stmt_block *)s->blk;
                        if (blk->stmts.len == 0 || blk->stmts.data[blk->stmts.len-1]->kind != STMT_KIND_EXIT) {
                                pusherr(tbl, ((stmt *)s)->loc,
                                        "last statement in `%s` does not contain an `exit` statement",
                                        s->id->lx);
                        }
                }
        }

        // The number of bytes to subtract from RSP
        // for the procedure's local variables.
        s->rsp = tbl->proc.rsp;
        tbl->proc.rsp = 0;

        // No longer in a procedure.
        tbl->proc.type = NULL;
        tbl->proc.inproc = 0;

        // Reset stack offset for other procedures.
        tbl->stack_offset = 0;

        // Remove parameters.
        pop_scope(tbl);

        return NULL;
}

static void *
visit_stmt_return(visitor *v, stmt_return *s)
{
        symtbl *tbl = (symtbl *)v->context;

        if (tbl->proc.inproc) {
                if (tbl->proc.type->kind == TYPE_KIND_NORETURN) {
                        pusherr(tbl, ((stmt *)s)->loc, "cannot return in procedure returning `!`");
                }
        }

        if (s->e) {
                s->e->accept(s->e, v);

                if (tbl->proc.inproc) {
                        if (!type_is_compat(&s->e->type, &tbl->proc.type)) {
                                pusherr(tbl, s->e->loc,
                                        "cannot return type `%s` in a procedure returning `%s`",
                                        type_to_cstr(s->e->type),
                                        type_to_cstr(tbl->proc.type));
                                return NULL;
                        }
                }
        }

        return NULL;
}

static void *
visit_stmt_exit(visitor *v, stmt_exit *s)
{
        if (s->e) {
                s->e->accept(s->e, v);
        }

        return NULL;
}

static void *
visit_stmt_extern_proc(visitor *v, stmt_extern_proc *s)
{
        symtbl *tbl = (symtbl *)v->context;

        if (sym_exists_in_scope(tbl, s->id->lx)) {
                pusherr(tbl, s->id->loc, "procecure `%s` is already defined", s->id->lx);
                return NULL;
        }

        type_proc *proc_ty = type_proc_alloc(s->id->lx, s->type, &s->params, s->variadic, s->export, /*extern=*/1);
        sym *proc_sym = sym_alloc(tbl, strdup(s->id->lx), (type *)proc_ty, 1);
        insert_sym_into_scope(tbl, proc_sym);

        if (s->export) {
                dyn_array_append(tbl->export_syms, proc_sym);
        }

        return NULL;
}

static void *
visit_stmt_if(visitor *v, stmt_if *s)
{
        s->e->accept(s->e, v);
        s->then->accept(s->then, v);

        if (s->else_) {
                s->else_->accept(s->else_, v);
        }

        return NULL;
}

static void *
visit_stmt_while(visitor *v, stmt_while *s)
{
        symtbl *tbl = (symtbl *)v->context;

        s->e->accept(s->e, v);
        tbl->loop = (void *)s;
        s->body->accept(s->body, v);
        tbl->loop = NULL;
        return NULL;
}

static void *
visit_stmt_for(visitor *v, stmt_for *s)
{
        symtbl *tbl = (symtbl *)v->context;

        push_scope(tbl);
        s->init->accept(s->init, v);
        s->e->accept(s->e, v);
        s->after->accept(s->after, v);

        tbl->loop = (void *)s;
        s->body->accept(s->body, v);

        pop_scope(tbl);
        tbl->loop = NULL;

        return NULL;
}

void *
visit_stmt_break(visitor *v, stmt_break *s)
{
        NOOP(v);

        symtbl *tbl = (symtbl *)v->context;

        if (!tbl->loop) {
                pusherr(tbl, ((stmt *)s)->loc, "cannot use `break` when outside of a loop");
                return NULL;
        }

        s->resolved_parent = tbl->loop;

        return NULL;
}

static void *
visit_stmt_continue(visitor *v, stmt_continue *s)
{
        NOOP(v);

        symtbl *tbl = (symtbl *)v->context;

        if (!tbl->loop) {
                pusherr(tbl, ((stmt *)s)->loc, "cannot use `continue` when outside of a loop");
                return NULL;
        }

        s->resolved_parent = tbl->loop;

        return NULL;
}

static void *
visit_stmt_struct(visitor *v, stmt_struct *s)
{
        symtbl *tbl = (symtbl *)v->context;

        // Check if this struct already exists.
        if (sym_exists_in_scope(tbl, s->id->lx)) {
                pusherr(tbl, s->id->loc, "struct `%s` is already defined", s->id->lx);
        }

        str_array member_names = dyn_array_empty(str_array);
        size_t sz = 0;
        for (size_t i = 0; i < s->members.len; ++i) {
                const parameter *p = &s->members.data[i];

                for (size_t j = 0; j < member_names.len; ++j) {
                        if (!strcmp(member_names.data[j], p->id->lx)) {
                                pusherr(tbl, p->id->loc, "the member of struct `%s` is already defined", p->id->lx);
                        }
                }

                sz += p->type->sz;
                s->members.data[i].resolved = sym_alloc(tbl, p->id->lx, p->type, 0);
                p->resolved->stack_offset = sz;

                dyn_array_append(member_names, p->id->lx);
        }

        dyn_array_free(member_names);

        if (sz == 0) {
                pusherr(tbl, s->id->loc, "struct `%s` has no members", s->id->lx);
        }

        type_struct *st_ty = type_struct_alloc(&s->members, sz);
        sym *sym = sym_alloc(tbl, s->id->lx, (type *)st_ty, 0);
        insert_sym_into_scope(tbl, sym);

        return NULL;
}

static void *
visit_stmt_module(visitor *v, stmt_module *s)
{
        NOOP(v, s);
        return NULL;
}

static void *
visit_stmt_import(visitor *v, stmt_import *s)
{
        symtbl *tbl = (symtbl *)v->context;

        for (size_t i = 0; i < s->filepaths.len; ++i) {
                char    *src        = read_file_from_searchpaths(&s->filepaths.data[i], &((stmt *)s)->loc);
                lexer    l          = lexer_create(src, s->filepaths.data[i]);
                program *p          = parser_create_program(&l);
                symtbl  *import_tbl = sem_analysis(p);

                dyn_array_append(tbl->imports, import_tbl);
                dyn_array_append(s->resolved_modnames, import_tbl->modname);
        }

        return NULL;
}

static void *
visit_stmt_embed(visitor *v, stmt_embed *s)
{
        symtbl *tbl = (symtbl *)v->context;

        for (size_t i = 0; i < s->lns.len; ++i) {
                const char *ln = s->lns.data[i]->lx;
                size_t ln_n = strlen(ln);
                forge_str name_buf = forge_str_create();

                for (size_t j = 0; ln[j]; ++j) {
                        size_t len = 0;
                        if (ln[j] == '{') {
                                ++len;
                                while (ln[j + len] != '}') {
                                        forge_str_append(&name_buf, ln[j+len]);
                                        ++len;
                                }

                                if (!sym_exists_in_scope(tbl, name_buf.data)) {
                                        pusherr(tbl, s->lns.data[i]->loc,
                                                "identifier `%s` is not defined",
                                                name_buf);
                                }

                                sym *sym = get_sym_from_scope(tbl, name_buf.data);

                                forge_str newln = forge_str_create();
                                for (size_t k = 0; k < ln_n-len-1; ++k) forge_str_append(&newln, ln[k]);
                                forge_str_concat(&newln, "[rbp-");
                                forge_str_concat(&newln, int_to_cstr(sym->stack_offset));
                                forge_str_concat(&newln, "]\n");
                                free(s->lns.data[i]->lx);
                                forge_str_destroy(&name_buf);
                                s->lns.data[i]->lx = newln.data;
                        }
                }
        }

        return NULL;
}

static void *
visit_stmt_empty(visitor *v, stmt_empty *s)
{
        NOOP(v, s);
        return NULL;
}

static void *
visit_stmt_enum(visitor *v, stmt_enum *s)
{
        NOOP(v, s);
        forge_todo("");
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
                visit_expr_mut,
                visit_expr_brace_init,
                visit_expr_namespace,
                visit_expr_arrayinit,
                visit_expr_index,
                visit_expr_un,
                visit_expr_character_literal,
                visit_expr_cast,
                visit_expr_bool_literal,
                visit_expr_null,

                visit_stmt_let,
                visit_stmt_expr,
                visit_stmt_block,
                visit_stmt_proc,
                visit_stmt_return,
                visit_stmt_exit,
                visit_stmt_extern_proc,
                visit_stmt_if,
                visit_stmt_while,
                visit_stmt_for,
                visit_stmt_break,
                visit_stmt_continue,
                visit_stmt_struct,
                visit_stmt_module,
                visit_stmt_import,
                visit_stmt_embed,
                visit_stmt_empty,
                visit_stmt_enum
        );
}

symtbl *
sem_analysis(program *p)
{
        symtbl *tbl         = (symtbl *)alloc(sizeof(symtbl));
        tbl->src_filepath   = p->src_filepath;
        tbl->modname        = p->modname;
        tbl->scope          = dyn_array_empty(smap_array);
        tbl->program        = p;
        tbl->proc.type      = NULL;
        tbl->proc.inproc    = 0;
        tbl->errs           = dyn_array_empty(str_array);
        tbl->stack_offset   = 0;
        tbl->loop           = NULL;
        tbl->imports.data   = NULL;
        tbl->imports.len    = 0;
        tbl->imports.cap    = 0;
        tbl->context_switch = 0;
        tbl->export_syms    = dyn_array_empty(sym_array);
        tbl->expty          = NULL;

        // Need to immediately add a scope for global scope.
        dyn_array_append(tbl->scope, smap_create(NULL));

        visitor *v = sem_visitor_alloc(tbl);

        for (size_t i = 0; i < p->stmts.len; ++i) {
                p->stmts.data[i]->accept(p->stmts.data[i], v);
        }

        if (tbl->errs.len > 0) {
                for (size_t i = 0; i < tbl->errs.len; ++i) {
                        fprintf(stderr, "%s\n", tbl->errs.data[i]);
                }
                exit(1);
        }

        return tbl;
}
