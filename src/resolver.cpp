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
resolve_type(type *ty)
{
        // if (ty == TYPE_CUSTOM) {
        //         assert(0);
        // }
        std::printf("TODO: resolve_type: add complex types\n");
}

static symbol *
new_symbol(symbol_kind   kind,
           token        *id,
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
        assert(0);
        return NULL;
}

static void *
resolve_expr_binary(visitor *v, expr_binary *e)
{
        assert(0);
        return NULL;
}

static void *
resolve_stmt_expr(visitor *v, stmt_expr *s)
{
        assert(0);
        return NULL;
}

static void *
resolve_stmt_let(visitor *v, stmt_let *s)
{
        assert(0);
        return NULL;
}

static void *
resolve_stmt_proc(visitor *v, stmt_proc *s)
{
        assert(0);
        return NULL;
}

static void *
resolve_stmt_block(visitor *v, stmt_block *s)
{
        assert(0);
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
