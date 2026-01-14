#include "parse.h"
#include "err.h"
#include "kwd.h"
#include "cstr.h"
#include "mem.h"

#include <assert.h>
#include <stdio.h>

#define SP(l, n) lexer_peek(l, n) && lexer_peek(l, n)

typedef struct {
        lexer *l;
        arena a;
        err err;
} parse_context;

static token *
expectkw(parse_context *ctx,
         const char    *kwd)
{
        token *hd;

        if (!(hd = lexer_next(ctx->l))) {
                ctx->err = err_create("out of tokens", (loc){.r=0,.c=0});
                return NULL;
        }

        if (hd->k != TK_KWD || strv_cmp2(hd->lx, kwd)) {
                ctx->err = err_create(format("expected keyword `%s' but got `%s'",
                                             kwd, strv_scstr(hd->lx)),
                                      hd->loc);
                return NULL;
        }

        return hd;
}

static token *
expect(parse_context *ctx, token_kind k)
{
        token *hd;

        if (!(hd = lexer_next(ctx->l))) {
                ctx->err = err_create("out of tokens", (loc){.r=0,.c=0});
                return NULL;
        }

        if (hd->k != k) {
                ctx->err = err_create(format("expected `%s' but got `%s'",
                                             tk_to_cstr(k), strv_scstr(hd->lx)),
                                      hd->loc);
                return NULL;
        }

        return hd;
}

static expr *
parse_primary_expr(parse_context *ctx)
{
        expr *left;

        left = NULL;

        while (1) {
                const token *hd = lexer_peek(ctx->l, 0);
                if (!hd) return left;

                switch (hd->k) {
                case TK_ID: {
                        const token *i = lexer_next(ctx->l);
                        left = (expr *)expr_id_alloc(i, &ctx->a);
                } break;
                case TK_INTLIT: {
                        const token *i = lexer_next(ctx->l);
                        left = (expr *)expr_intlit_alloc(atoi(strv_scstr(i->lx)), &ctx->a);
                } break;
                default: return left;
                }

                left->loc = hd->loc;
        }

        // unreachable
        return left;
}

static expr *
parse_member_expr(parse_context *ctx)
{
        return parse_primary_expr(ctx);
}

static expr *
parse_unary_expr(parse_context *ctx)
{
        token *cur = lexer_peek(ctx->l, 0);
        if (cur && (cur->k == TK_MINUS
                    || cur->k == TK_PLUS
                    || cur->k == TK_BANG
                    || cur->k == TK_ASTERISK
                    || cur->k == TK_AMPERSAND)) {
                token *op = lexer_next(ctx->l);
                expr *rhs = (expr *)parse_unary_expr(ctx);
                ((expr *)rhs)->loc = op->loc;
                return (expr *)expr_un_alloc(op, rhs, &ctx->a);
        }
        return parse_member_expr(ctx);
}

static expr *
parse_multiplicitate_expr(parse_context *ctx)
{
        expr *lhs = parse_unary_expr(ctx);
        token *cur = lexer_peek(ctx->l, 0);
        while (cur && (cur->k == TK_ASTERISK
                       || cur->k == TK_FORWARDSLASH
                       || cur->k == TK_PERCENT)) {
                token *op = lexer_next(ctx->l);
                expr *rhs = parse_unary_expr(ctx);
                expr_bin *bin = expr_bin_alloc(lhs, op, rhs, &ctx->a);
                ((expr *)bin)->loc = lhs->loc;
                lhs = (expr *)bin;
                cur = lexer_peek(ctx->l, 0);
        }
        return lhs;
}

static expr *
parse_additive_expr(parse_context *ctx)
{
        expr *lhs = parse_multiplicitate_expr(ctx);
        token *cur = lexer_peek(ctx->l, 0);
        while (cur && (cur->k == TK_PLUS
                       || cur->k == TK_MINUS)) {
                token *op = lexer_next(ctx->l);
                expr *rhs = parse_multiplicitate_expr(ctx);
                expr_bin *bin = expr_bin_alloc(lhs, op, rhs, &ctx->a);
                ((expr *)bin)->loc = lhs->loc;
                lhs = (expr *)bin;
                cur = lexer_peek(ctx->l, 0);
        }
        return lhs;
}

static expr *
parse_equalitative_expr(parse_context *ctx)
{
        expr *lhs = parse_additive_expr(ctx);
        token *cur = lexer_peek(ctx->l, 0);
        while (cur && (cur->k == TK_DOUBLE_EQ
                       || cur->k == TK_GREATERTHAN_EQ
                       || cur->k == TK_GREATERTHAN
                       || cur->k == TK_LESSTHAN_EQ
                       || cur->k == TK_LESSTHAN
                       || cur->k == TK_BANG_EQ)) {
                token *op = lexer_next(ctx->l);
                expr *rhs = parse_additive_expr(ctx);
                expr_bin *bin = expr_bin_alloc(lhs, op, rhs, &ctx->a);
                ((expr *)bin)->loc = lhs->loc;
                lhs = (expr *)bin;
                cur = lexer_peek(ctx->l, 0);
        }
        return lhs;
}

static expr *
parse_logical_expr(parse_context *ctx)
{
        expr *lhs = parse_equalitative_expr(ctx);
        token *cur = lexer_peek(ctx->l, 0);
        while (cur && (cur->k == TK_DOUBLE_AMPERSAND
                       || cur->k == TK_DOUBLE_PIPE)) {
                token *op = lexer_next(ctx->l);
                expr *rhs = parse_equalitative_expr(ctx);
                expr_bin *bin = expr_bin_alloc(lhs, op, rhs, &ctx->a);
                ((expr *)bin)->loc = lhs->loc;
                lhs = (expr *)bin;
                cur = lexer_peek(ctx->l, 0);
        }
        return lhs;
}

static expr *
parse_assignment_expr(parse_context *ctx)
{
        expr *lhs = parse_logical_expr(ctx);

        token *cur = lexer_peek(ctx->l, 0);
        if (!cur) return lhs;

        switch (cur->k) {
        case TK_PLUS_EQ:
        case TK_MINUS_EQ:
        case TK_ASTERISK_EQ:
        case TK_FORWARDSLASH_EQ:
        case TK_PERCENT_EQ:
        case TK_AMPERSAND_EQ:
        case TK_PIPE_EQ:
        case TK_UPTICK_EQ:
        case TK_EQ: {
                const token *op = lexer_next(ctx->l);
                expr *rhs = parse_assignment_expr(ctx);
                return (expr *)expr_bin_alloc(lhs, op, rhs, &ctx->a);
        }
        default:
                return lhs;
        }
}

static expr *
parse_expr(parse_context *ctx)
{
        return parse_assignment_expr(ctx);
}

static type *
parse_type(parse_context *ctx)
{
        const token *hd;
        strv         lx;
        type        *ty;

        hd = lexer_next(ctx->l);
        lx = hd->lx;
        ty = NULL;

        /* if (hd->ty == TOKEN_TYPE_LEFT_SQUARE) { */
        /*         type *inner = parse_type(ctx); */
        /*         int len = -1; */

        /*         if (LSP(ctx->l, 0)->ty == TOKEN_TYPE_SEMICOLON) { */
        /*                 lexer_discard(ctx->l); // ; */
        /*                 len = atoi(expect(ctx, TOKEN_TYPE_INTEGER_LITERAL)->lx); */
        /*         } */
        /*         ty = (type *)type_list_alloc(inner, len); */
        /*         (void)expect(ctx, TOKEN_TYPE_RIGHT_SQUARE); */
        /*         return ty; */
        /* } */

        if (!strv_cmp2(lx, TY_I32)) {
                ty = (type *)type_i32_alloc(&ctx->a);
        } else {
                assert(0);
        }

        // Handles all pointer types (ex: u8**).
        /* while (LSP(ctx->l, 0)->ty == TOKEN_TYPE_ASTERISK) { */
        /*         lexer_discard(ctx->l); */
        /*         ty = (type *)type_ptr_alloc(ty); */
        /* } */

        return ty;
}

static stmt_let *
parse_stmt_let(parse_context *ctx)
{
        token *idt;
        strv   id;
        type  *type;
        expr  *e;

        if (!expectkw(ctx, KWD_LET))
                return NULL;

        if (!(idt = expect(ctx, TK_ID)))
                return NULL;

        id = idt->lx;

        if (!expect(ctx, TK_COLON))
                return NULL;

        if (!(type = parse_type(ctx)))
                return NULL;

        if (!expect(ctx, TK_EQ))
                return NULL;

        if (!(e = parse_expr(ctx)))
                return NULL;

        if (!expect(ctx, TK_SEMI))
                return NULL;

        return stmt_let_alloc(id, type, e, &ctx->a);
}

static stmt *
parse_kwd_stmt(parse_context *ctx)
{
        strv kwd;

        kwd = ctx->l->hd->lx;

        if (!strv_cmp2(kwd, KWD_LET))
                return (stmt *)parse_stmt_let(ctx);

        assert(0);
        return NULL;
}

static stmt *
parse_stmt(parse_context *ctx)
{
        token *hd;

        if (!(hd = lexer_peek(ctx->l, 0))) {
                ctx->err = err_create("out of tokens", (loc){.r=0,.c=0});
                return NULL;
        }

        if (hd->k == TK_KWD)
                return parse_kwd_stmt(ctx);

        assert(0);
        return NULL;
}

stmt_array
parse(lexer *l)
{
        parse_context ctx;
        stmt_array    stmts;

        stmts = array_empty(stmt_array);
        ctx   = (parse_context) {
                .l   = l,
                .a   = {0},
                .err = {0},
        };

        arena_init(&ctx.a, ARENA_DEFAULT_ALLOC_SIZE);

        while (SP(ctx.l, 0)->k != TK_EOF) {
                stmt *s = parse_stmt(&ctx);
                if (s) array_append(stmts, s);
                else   break;
        }

        if (ctx.err.msg) {
                err_print(ctx.err);
                exit(1);
        }

        return stmts;
}
