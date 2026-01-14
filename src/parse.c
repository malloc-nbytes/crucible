#include "parse.h"
#include "err.h"
#include "kwd.h"
#include "cstr.h"

#include <assert.h>

#define SP(l, n) lexer_peek(l, n) && lexer_peek(l, n)

typedef struct {
        lexer *l;
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

static stmt_let *
parse_stmt_let(parse_context *ctx)
{
        if (!expectkw(ctx, KWD_LET))
                return NULL;
}

static stmt *
parse_kwd_stmt(parse_context *ctx)
{
        strv kwd;

        kwd = ctx->l->hd->lx;

        if (!strv_cmp2(kwd, KWD_LET)) {
                return (stmt *)parse_stmt_let(ctx);
        }

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
        stmt_array stmts;

        ctx = (parse_context) {
                .l = l,
                .err = {0},
        };

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
