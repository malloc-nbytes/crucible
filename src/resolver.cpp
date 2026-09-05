#include "resolver.hpp"
#include "visitor.hpp"
#include "scope.hpp"

#include <cassert>
#include <cstdint>
#include <exception>
#include <format>
#include <iostream>
#include <optional>

typedef struct {
        scope<symbol *>         scope_;
        scope<type *>           types;
        std::optional<type *>   return_type;
        uint32_t                next_symbol_id;
} resolver_context;

struct semantic_analysis_error : public std::exception {};

struct identifier_not_defined_error : semantic_analysis_error {
        const token *id;

        identifier_not_defined_error(const token *id)
                : id(id) {}

        const char *
        what(void) const noexcept override
        {
                return format("%s: idenifier `%s' has not been defined",
                              location_to_string(id->loc),
                              id->lx).c_str();
        }
};

struct invalid_assignment_type_error : semantic_analysis_error {
        location loc;

        invalid_assignment_type_error(location loc)
                : loc(loc) {}

        const char *
        what(void) const noexcept override
        {
                return format("%s: expression is not assignable",
                              location_to_string(loc)).c_str();
        }
};

struct incompatible_types_error : semantic_analysis_error {
        location                loc;
        const type *const       t0;
        const type *const       t1;

        incompatible_types_error(location               loc,
                                 const type *const      t0,
                                 const type *const      t1)
                : loc(loc), t0(t0), t1(t1) {}

        const char *
        what(void) const noexcept override
        {
                return format("%s: types `%s' and `%s' are not compatible",
                              location_to_string(loc),
                              type_to_string(t0),
                              type_to_string(t1)).c_str();
        }
};

struct identifier_already_declared_error : semantic_analysis_error {
        const token *const id;

        identifier_already_declared_error(const token *const id)
                : id(id) {}

        const char *
        what(void) const noexcept override
        {
                return format("%s: identifier `%s' is already defined",
                                location_to_string(id->loc),
                                id->lx).c_str();
        }
};

static int
is_assignable(expr *e)
{
        if (e->k == EXPR_KIND_IDENTIFIER) {
                expr_identifier *id = (expr_identifier *)e;
                assert(id->sym);
                return id->sym->k == SYMBOL_KIND_LOCAL || id->sym->k == SYMBOL_KIND_PARAM;
        }
        if (e->k == EXPR_KIND_UNARY) {
                expr_unary *u = (expr_unary *)e;
                return u->op->k == TOKEN_KIND_ASTERISK;
        }
        return 0;
        //return e->k == EXPR_KIND_INDEX || e->k == EXPR_KIND_MEMBER;
}

static int
is_addressable(expr *e)
{
        if (e->k == EXPR_KIND_IDENTIFIER) {
                expr_identifier *id = (expr_identifier *)e;
                assert(id->sym);
                return id->sym->k == SYMBOL_KIND_LOCAL || id->sym->k == SYMBOL_KIND_PARAM;
        }
        if (e->k == EXPR_KIND_UNARY) {
                expr_unary *u = (expr_unary *)e;
                return u->op->k == TOKEN_KIND_ASTERISK;
        }
        return 0;
        //return e->k == EXPR_KIND_INDEX || e->k == EXPR_KIND_MEMBER;
}

static void
resolve_type(resolver_context *ctx, type *ty)
{
        // if (ty == TYPE_CUSTOM) {
        //         assert(0);
        // }
        std::printf("TODO: resolve_type: add complex types\n");
}

static symbol *
new_symbol(symbol_kind   kind,
           const token  *id,
           type         *ty,
           visitor      *v)
{
        return symbol_alloc(kind,
                            ((resolver_context *)v->context)->next_symbol_id++,
                            id->lx, ty, id->loc);
}

static void *
resolve_expr_int(visitor *v, expr_int *e)
{
        ((expr *)e)->ty = (type *)type_i32_alloc();
        return NULL;
}

static void *
resolve_expr_identifier(visitor *v, expr_identifier *e)
{
        resolver_context        *ctx;
        symbol                  *sym;

        ctx = (resolver_context *)v->context;

        if (!scope_contains<symbol *>(&ctx->scope_, e->id->lx))
                throw identifier_not_defined_error(e->id);

        sym = *scope_get<symbol *>(&ctx->scope_, e->id->lx);

        ((expr *)e)->ty = sym->ty;
        e->sym          = sym;

        return NULL;
}

static void *
resolve_expr_unary(visitor *v, expr_unary *e)
{
      return NULL;
}

static void *
resolve_expr_binary(visitor *v, expr_binary *e)
{
        resolver_context *ctx;

        ctx = (resolver_context *)v->context;

        e->lhs->accept(e->lhs, v);
        e->rhs->accept(e->lhs, v);

        if (type_is_assignment(e->op->k) && !is_assignable(e->lhs))
                throw invalid_assignment_type_error(e->lhs->loc);

        if (!type_check(e->lhs->ty, e->rhs->ty))
                throw incompatible_types_error(e->lhs->loc, e->lhs->ty, e->rhs->ty);

        return NULL;
}

static void *
resolve_stmt_expr(visitor *v, stmt_expr *s)
{
        s->e->accept(s->e, v);
        return NULL;
}

static void *
resolve_stmt_let(visitor *v, stmt_let *s)
{
        resolver_context *ctx;

        ctx = (resolver_context *)v->context;

        if (scope_contains<symbol *>(&ctx->scope_, s->id->lx))
                throw identifier_already_declared_error(s->id);

        resolve_type(ctx, s->ty);

        s->e->accept(s->e, v);

        if (!type_check(s->ty, s->e->ty))
                throw incompatible_types_error(s->base.loc, s->ty, s->e->ty);

        s->sym = new_symbol(SYMBOL_KIND_LOCAL, s->id, s->ty, v);

        scope_insert(&ctx->scope_, s->id->lx, s->sym);

        return NULL;
}

static void *
resolve_stmt_proc(visitor *v, stmt_proc *s)
{
        resolver_context *ctx;

        ctx = (resolver_context *)v->context;

        resolve_type(ctx, s->rty);

        for (size_t i = 0; i < s->params.size(); ++i) {
                parameter *p = &s->params.at(i);
                if (scope_contains<symbol *>(&ctx->scope_, p->id->lx))
                        throw identifier_already_declared_error(p->id);
                resolve_type(ctx, p->ty);
        }

        if (scope_contains<symbol *>(&ctx->scope_, s->id->lx))
                throw identifier_already_declared_error(s->id);

        s->sym = new_symbol(SYMBOL_KIND_PROC, s->id, s->rty, v);

        scope_push<symbol *>(&ctx->scope_);
        ctx->return_type = s->rty;

        for (size_t i = 0; i < s->params.size(); ++i) {
                parameter *p = &s->params.at(i);
                p->sym = new_symbol(SYMBOL_KIND_PARAM, p->id, p->ty, v);
                scope_insert(&ctx->scope_, p->id->lx, p->sym);
        }

        if (s->body != std::nullopt)
                s->body.value()->accept(s->body.value(), v);

        ctx->return_type = std::nullopt;
        scope_pop<symbol *>(&ctx->scope_);

        return NULL;
}

static void *
resolve_stmt_block(visitor *v, stmt_block *s)
{
        for (size_t i = 0; i < s->stmts.size(); ++i)
                s->stmts.at(i)->accept(s->stmts.at(i), v);
        return NULL;
}

void
resolve_ast(parser *p)
{
        resolver_context ctx = resolver_context {
                .scope_         = scope_create<symbol *>(),
                .types          = scope_create<type *>(),
                .return_type    = {},
                .next_symbol_id = 0,
        };

        visitor v = visitor_create((void *)&ctx,
                                   resolve_expr_int,
                                   resolve_expr_identifier,
                                   resolve_expr_unary,
                                   resolve_expr_binary,
                                   resolve_stmt_expr,
                                   resolve_stmt_let,
                                   resolve_stmt_proc,
                                   resolve_stmt_block);

        try {
                for (size_t i = 0; i < p->stmts.size(); ++i)
                        p->stmts.at(i)->accept(p->stmts.at(i), &v);
        } catch (const semantic_analysis_error &e) {
                std::fprintf(stderr, "%s\n", e.what());
                std::exit(1);
        }
}
