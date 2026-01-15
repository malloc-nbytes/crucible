#include "parse.h"
#include "kwd.h"
#include "utils.h"
#include "mem.h"
#include "ast.h"
#include "glconf.h"

#include <assert.h>
#include <stdio.h>

#define SP(l, n) lexer_peek(l, n) && lexer_peek(l, n)

static stmt *parse_stmt(parse_context *ctx);

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
                        left           = (expr *)expr_id_alloc(i, &ctx->a);
                } break;
                case TK_INTLIT: {
                        const token *i   = lexer_next(ctx->l);
                        int          val = atoi(strv_scstr(i->lx));
                        left             = (expr *)expr_intlit_alloc(val, &ctx->a);
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
                token *op          = lexer_next(ctx->l);
                expr  *rhs         = (expr *)parse_unary_expr(ctx);
                ((expr *)rhs)->loc = op->loc;
                return (expr *)expr_un_alloc(op, rhs, &ctx->a);
        }
        return parse_member_expr(ctx);
}

static expr *
parse_multiplicitate_expr(parse_context *ctx)
{
        expr  *lhs = parse_unary_expr(ctx);
        token *cur = lexer_peek(ctx->l, 0);
        while (cur && (cur->k == TK_ASTERISK
                       || cur->k == TK_FORWARDSLASH
                       || cur->k == TK_PERCENT)) {
                token    *op       = lexer_next(ctx->l);
                expr     *rhs      = parse_unary_expr(ctx);
                expr_bin *bin      = expr_bin_alloc(lhs, op, rhs, &ctx->a);
                ((expr *)bin)->loc = lhs->loc;
                lhs                = (expr *)bin;
                cur                = lexer_peek(ctx->l, 0);
        }
        return lhs;
}

static expr *
parse_additive_expr(parse_context *ctx)
{
        expr  *lhs = parse_multiplicitate_expr(ctx);
        token *cur = lexer_peek(ctx->l, 0);
        while (cur && (cur->k == TK_PLUS
                       || cur->k == TK_MINUS)) {
                token    *op       = lexer_next(ctx->l);
                expr     *rhs      = parse_multiplicitate_expr(ctx);
                expr_bin *bin      = expr_bin_alloc(lhs, op, rhs, &ctx->a);
                ((expr *)bin)->loc = lhs->loc;
                lhs                = (expr *)bin;
                cur                = lexer_peek(ctx->l, 0);
        }
        return lhs;
}

static expr *
parse_equalitative_expr(parse_context *ctx)
{
        expr  *lhs = parse_additive_expr(ctx);
        token *cur = lexer_peek(ctx->l, 0);
        while (cur && (cur->k == TK_DOUBLE_EQ
                       || cur->k == TK_GREATERTHAN_EQ
                       || cur->k == TK_GREATERTHAN
                       || cur->k == TK_LESSTHAN_EQ
                       || cur->k == TK_LESSTHAN
                       || cur->k == TK_BANG_EQ)) {
                token    *op       = lexer_next(ctx->l);
                expr     *rhs      = parse_additive_expr(ctx);
                expr_bin *bin      = expr_bin_alloc(lhs, op, rhs, &ctx->a);
                ((expr *)bin)->loc = lhs->loc;
                lhs                = (expr *)bin;
                cur                = lexer_peek(ctx->l, 0);
        }
        return lhs;
}

static expr *
parse_logical_expr(parse_context *ctx)
{
        expr  *lhs = parse_equalitative_expr(ctx);
        token *cur = lexer_peek(ctx->l, 0);
        while (cur && (cur->k == TK_DOUBLE_AMPERSAND
                       || cur->k == TK_DOUBLE_PIPE)) {
                token    *op       = lexer_next(ctx->l);
                expr     *rhs      = parse_equalitative_expr(ctx);
                expr_bin *bin      = expr_bin_alloc(lhs, op, rhs, &ctx->a);
                ((expr *)bin)->loc = lhs->loc;
                lhs                = (expr *)bin;
                cur                = lexer_peek(ctx->l, 0);
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
                const token *op  = lexer_next(ctx->l);
                expr        *rhs = parse_assignment_expr(ctx);
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

        if (!strv_cmp2(lx, TY_I32))
                ty = (type *)type_i32(&ctx->t_ctx);
        else if (!strv_cmp2(lx, TY_VOID))
                ty = (type *)type_void(&ctx->t_ctx);
        else if (hd->k == TK_BANG)
                ty = (type *)type_never(&ctx->t_ctx);
        else {
                ctx->err = err_create(format("illegal type `%s'", strv_scstr(lx)), hd->loc);
                return NULL;
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
        token *id;
        type  *type;
        expr  *e;

        if (!expectkw(ctx, KWD_LET))
                return NULL;

        if (!(id = expect(ctx, TK_ID)))
                return NULL;

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

static int
parse_proc_params(parse_context *ctx,
                  idty_array    *params)
{
        *params = array_empty(idty_array);

        if (!expect(ctx, TK_LPAREN))
                goto bad;

        while (SP(ctx->l, 0)->k != TK_RPAREN) {
                const token *id;
                type        *type;

                if (!(id = expect(ctx, TK_ID)))
                        goto bad;

                if (!expect(ctx, TK_COLON))
                        goto bad;

                if (!(type = parse_type(ctx)))
                        goto bad;

                array_append(*params, idty_alloc(id, type, &ctx->a));

                if (SP(ctx->l, 0)->k == TK_COMMA)
                        (void)expect(ctx, TK_COMMA);
                else
                        break;
        }

        if (!expect(ctx, TK_RPAREN))
                goto bad;

        return 1;
 bad:
        array_free(*params);
        return 0;
}

static stmt_blk *
parse_stmt_blk(parse_context *ctx)
{
        stmt_array stmts;

        stmts = array_empty(stmt_array);

        if (!expect(ctx, TK_LBRACK))
                goto bad;

        while (SP(ctx->l, 0)->k != TK_RBRACK) {
                stmt *s;

                if (!(s = parse_stmt(ctx)))
                        goto bad;

                array_append(stmts, s);
        }

        if (!expect(ctx, TK_RBRACK))
                goto bad;

        return stmt_blk_alloc(stmts, &ctx->a);
 bad:
        array_free(stmts);
        return NULL;
}

static stmt_proc *
parse_stmt_proc(parse_context *ctx)
{
        const token *id;
        idty_array   params;
        type        *rtype;
        stmt        *blk;

        if (!expectkw(ctx, KWD_PROC))
                return NULL;

        if (!(id = expect(ctx, TK_ID)))
                return NULL;

        if (!parse_proc_params(ctx, &params))
                return NULL;

        if (!expect(ctx, TK_COLON))
                return NULL;

        if (!(rtype = parse_type(ctx)))
                return NULL;

        if (SP(ctx->l, 0)->k == TK_EQ) {
                (void)expect(ctx, TK_EQ);
                if (!(blk = parse_stmt(ctx)))
                        return NULL;
        }

        else if (!(blk = (stmt *)parse_stmt_blk(ctx)))
                return NULL;

        return stmt_proc_alloc(id, params, rtype, blk, &ctx->a);
}

static stmt_return *
parse_stmt_return(parse_context *ctx)
{
        expr *e;

        if (!expectkw(ctx, KWD_RETURN))
                return NULL;

        if (!(e = parse_expr(ctx)))
                return NULL;

        if (!expect(ctx, TK_SEMI))
                return NULL;

        return stmt_return_alloc(e, &ctx->a);
}

static stmt_exit *
parse_stmt_exit(parse_context *ctx)
{
        expr *e;

        if (!expectkw(ctx, KWD_EXIT))
                return NULL;

        if (!(e = parse_expr(ctx)))
                return NULL;

        if (!expect(ctx, TK_SEMI))
                return NULL;

        return stmt_exit_alloc(e, &ctx->a);
}

static stmt *
parse_kwd_stmt(parse_context *ctx)
{
        token *hd;
        strv   kwd;

        assert((hd = lexer_peek(ctx->l, 0)));

        kwd = hd->lx;

        if (!strv_cmp2(kwd, KWD_LET))
                return (stmt *)parse_stmt_let(ctx);
        else if (!strv_cmp2(kwd, KWD_PROC))
                return (stmt *)parse_stmt_proc(ctx);
        else if (!strv_cmp2(kwd, KWD_RETURN))
                return (stmt *)parse_stmt_return(ctx);
        else if (!strv_cmp2(kwd, KWD_EXIT))
                return (stmt *)parse_stmt_exit(ctx);

        ctx->err = err_create(format("illegal keyword starting at `%s'",
                                     strv_scstr(kwd)), hd->loc);
        return NULL;
}

static stmt *
parse_stmt(parse_context *ctx)
{
        token *hd;

        assert((hd = lexer_peek(ctx->l, 0)));

        if (hd->k == TK_KWD)
                return parse_kwd_stmt(ctx);

        if (hd->k == TK_LBRACK)
                return (stmt *)parse_stmt_blk(ctx);

        ctx->err = err_create(format("illegal statement starting at `%s'",
                                     strv_scstr(hd->lx)), hd->loc);
        return NULL;
}

parse_context
parse(lexer *l)
{
        parse_context ctx;
        stmt_array    stmts;

        stmts = array_empty(stmt_array);
        ctx   = (parse_context) {
                .l     = l,
                .a     = {0},
                .err   = {0},
                .t_ctx = {0},
        };

        arena_init(&ctx.a, ARENA_DEFAULT_ALLOC_SIZE);
        type_context_init(&ctx.t_ctx, &ctx.a);

        while (SP(ctx.l, 0)->k != TK_EOF) {
                stmt *s = parse_stmt(&ctx);
                if (s) array_append(stmts, s);
                else   break;
        }

        if (ctx.err.msg) {
                err_print(ctx.err);
                lexer_free(l);
                arena_free(&ctx.a);
                array_free(stmts);
                exit(1);
        }

        if (g_glconf.flags & FT_SHOW_AST)
                ast_dump(stmts);

        return ctx;
}
