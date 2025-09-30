#include "sem.h"
#include "visitor.h"
#include "mem.h"
#include "ds/smap.h"

#include <forge/array.h>
#include <forge/utils.h>
#include <forge/err.h>

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
        s->stack_offset = tbl->stack_offset + type_to_int(ty);
        s->extern_      = extern_;

        return s;
}

static type *
binop(symtbl      *tbl,
      expr        *lhs,
      const token *op,
      expr        *rhs)
{
        if (op->ty >= TOKEN_TYPE_BINOP_LEN || op->ty <= TOKEN_TYPE_OTHER_LEN) {
                forge_err_wargs("%sunsupported binary operator `%s`",
                                loc_err(op->loc), op->lx);
        }

        if (!type_is_compat(&lhs->type, &rhs->type)) {
                pusherr(tbl, lhs->loc,
                        "cannot perform binary operator `%s` on %s and %s",
                        op->lx,
                        type_to_cstr(lhs->type), type_to_cstr(rhs->type));
        }

        return lhs->type;
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
        NOOP(v);
        ((expr *)e)->type = (type *)type_number_alloc();
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
        if (lhs_ty->kind != TYPE_KIND_PROC) {
                pusherr(tbl, e->lhs->loc, "cannot perform a procedure call on type %s", type_to_cstr(lhs_ty));
                ((expr *)e)->type = (type *)type_unknown_alloc();
                return NULL;
        }

        type_proc *proc_ty = (type_proc *)lhs_ty;

        // Check number of arguments
        if (e->args.len != proc_ty->params->len) {
                if (e->args.len >= proc_ty->params->len && proc_ty->variadic) {
                        goto ok;
                }

                pusherr(tbl, ((expr *)e)->loc,
                        "procedure requires %zu arguments but %zu were given",
                        ((type_proc *)proc_ty)->params->len, e->args.len);
        }
 ok:

        // This expression's return type is the return type
        // of the function that we are calling.
        ((expr *)e)->type = proc_ty->rettype;

        // Go through each argument in the procedure call.
        for (size_t i = 0; i < e->args.len; ++i) {
                expr *arg = e->args.data[i];
                arg->accept(arg, v);

                // Type check argument list
                if (i < proc_ty->params->len && proc_ty->variadic) {
                        type *expected = proc_ty->params->data[i].type;
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
        e->lhs->accept(e->lhs, v);
        e->rhs->accept(e->rhs, v);

        // TODO: support other assignment operators
        if (e->op->ty != TOKEN_TYPE_EQUALS) {
                forge_err("only direct assignment is supported `=`");
        }

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

                if (!type_is_compat(&e->exprs.data[i]->type, &struct_ty->members->data[i].type)) {
                        pusherr(tbl, e->exprs.data[i]->loc, "expected type `%s` but got type `%s`",
                                type_to_cstr(struct_ty->members->data[i].type),
                                type_to_cstr(e->exprs.data[i]->type));
                }
        }

        ((expr *)e)->type = struct_sym->ty;

        e->resolved_syms = (sym_array *)alloc(sizeof(sym_array));
        *e->resolved_syms = dyn_array_empty(sym_array);

        return NULL;
}

static void *
visit_expr_member(visitor *v, expr_member *e)
{
        symtbl *tbl = (symtbl *)v->context;

        e->lhs->accept(e->lhs, v);

        if (e->lhs->type->kind != TYPE_KIND_STRUCT) {
                pusherr(tbl, e->lhs->loc,
                        "the left-hand-side expression evaluates to type `%s`, but type `<struct>` is needed",
                        type_to_cstr(e->lhs->type));
                return NULL;
        }

        const type_struct *stty = (type_struct *)e->lhs->type;

        sym *member_sym = NULL;
        for (size_t i = 0; i < stty->members->len; ++i) {
                if (!strcmp(stty->members->data[i].id->lx, e->member->lx)) {
                        member_sym = stty->members->data[i].resolved;
                        break;
                }
        }
        if (!member_sym) {
                pusherr(tbl, e->member->loc, "struct `%s` has no member named `%s`",
                        stty->id->lx, e->member->lx);
                return NULL;
        }

        ((expr *)e)->type = member_sym->ty;
        e->resolved_member = member_sym;

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

        s->e->accept(s->e, v);

        sym *sym = sym_alloc(tbl, s->id->lx, s->type, 0);

        // Structs need a bit more information to compute.
        if (s->type->kind == TYPE_KIND_CUSTOM
            && s->e->type->kind == TYPE_KIND_STRUCT) {
                free(s->type); // free temporary `custom` type.
                s->type = s->e->type;

                // No need for type checking for casting, done earlier.
                expr_brace_init *br = (expr_brace_init *)s->e;
                assert(br->resolved_syms); // alloc'd in sem.c:visit_expr_brace_init

                const char *st_id = br->struct_id->lx;
                assert(st_id);

                sym->ty = get_sym_from_scope(tbl, st_id)->ty;

                type_struct *stty = (type_struct *)sym->ty;
                assert(stty);

                // Append resolved symbols and align each symbol with the
                // procedure's stack offset (for RSP).
                for (size_t i = 0; i < br->ids.len; ++i) {
                        assert(stty->members->data[i].resolved);
                        dyn_array_append(*br->resolved_syms, stty->members->data[i].resolved);

                        // Make symbols align with the stack. Originally, the symbols are
                        // just offsets from each other, but now we update it to align
                        // with the current global stack offset.
                        br->resolved_syms->data[i]->stack_offset += tbl->stack_offset;
                }
        }

        insert_sym_into_scope(tbl, sym);
        tbl->stack_offset += type_to_int(sym->ty);

        // Increase the procedures RSP register subtraction amount.
        if (tbl->proc.inproc) {
                tbl->proc.rsp += type_to_int(sym->ty);
        }

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
        type_proc *proc_ty = type_proc_alloc(s->type, &s->params, s->variadic);
        insert_sym_into_scope(tbl, sym_alloc(tbl, s->id->lx, (type *)proc_ty, 0));

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
                tbl->stack_offset += type_to_int(param->ty);
                s->params.data[i].resolved = param;
        }

        // We are currently inside of a procedure, keep track
        // of the return type so that the result of any 'return'
        // statements can be typechecked with the type of the procedure.
        tbl->proc.inproc = 1;
        tbl->proc.type = proc_ty->rettype;

        // Procedure body.
        s->blk->accept(s->blk, v);

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

        type_proc *proc_ty = type_proc_alloc(s->type, &s->params, s->variadic);
        insert_sym_into_scope(tbl, sym_alloc(tbl, s->id->lx, (type *)proc_ty, 1));

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
                parameter *p = &s->members.data[i];

                for (size_t j = 0; j < member_names.len; ++j) {
                        if (!strcmp(member_names.data[j], p->id->lx)) {
                                pusherr(tbl, p->id->loc, "the member of struct `%s` is already defined", p->id->lx);
                        }
                }

                // TODO: better error handling
                if (p->type->kind == TYPE_KIND_CUSTOM) {
                        type_custom *cust = (type_custom *)p->type;
                        p->type = get_sym_from_scope(tbl, cust->struct_id->lx)->ty;
                        assert(p->type);
                }

                sz += type_to_int(p->type);
                s->members.data[i].resolved = sym_alloc(tbl, p->id->lx, p->type, 0);
                p->resolved->stack_offset = sz;

                dyn_array_append(member_names, p->id->lx);
        }

        dyn_array_free(member_names);

        if (sz == 0) {
                pusherr(tbl, s->id->loc, "struct `%s` has no members", s->id->lx);
        }

        type_struct *st_ty = type_struct_alloc(&s->members, s->id, sz);
        sym *sym = sym_alloc(tbl, s->id->lx, (type *)st_ty, 0);
        insert_sym_into_scope(tbl, sym);

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
                visit_expr_mut,
                visit_expr_brace_init,
                visit_expr_member,
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
                visit_stmt_struct
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
                .errs = dyn_array_empty(str_array),
                .stack_offset = 0,
                .loop = NULL,
        };

        // Need to immediately add a scope for global scope.
        dyn_array_append(tbl.scope, smap_create(NULL));

        visitor *v = sem_visitor_alloc(&tbl);

        for (size_t i = 0; i < p->stmts.len; ++i) {
                p->stmts.data[i]->accept(p->stmts.data[i], v);
        }

        return tbl;
}
