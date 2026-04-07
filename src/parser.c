#include "parser.h"
#include "err.h"
#include "kw.h"
#include "types.h"

#include <assert.h>
#include <string.h>

#define LSP(l, i) lexer_peek(l, i) && lexer_peek(l, i)

static token *
expect(parser_context *ctx,
       token_kind      k)
{
        token *hd;

        if (!(hd = lexer_peek(ctx->l, 0))) {
                ctx->err.msg = str_from_fmt("expected `%s' but got nothing",
                                            token_kind_cstr(k));
                return NULL;
        }

        if (hd->kind != k) {
                ctx->err.msg = str_from_fmt("expected `%s' but got `%s'",
                                            token_kind_cstr(k),
                                            token_kind_cstr(hd->kind));
                return NULL;
        }

        return hd;
}

static token *
expectkw(parser_context *ctx,
         const char     *kw)
{
        token *hd;

        if (!(hd = lexer_peek(ctx->l, 0))) {
                ctx->err.msg = str_from_fmt("expected keyword `%s' but got nothing", kw);
                return NULL;
        }

        if (hd->kind != TK_KW || strcmp(sv_cstr(hd->lx), kw) != 0) {
                ctx->err.msg = str_from_fmt("expected keyword `%s' but got `%s'",
                                            kw, token_kind_cstr(hd->kind));
                return NULL;
        }
}

static type *
parse_type(parser_context *ctx)
{
        assert(ctx && 0);
        return NULL;
}

static stmt_proc *
parse_stmt_proc(parser_context *ctx)
{
        assert(ctx && 0);
        return NULL;
}

static stmt *
parse_stmt_kw(parser_context *ctx)
{
        const token *hd;
        const char  *lx;

        hd = lexer_peek(ctx->l, 0);
        lx = sv_cstr(hd->lx);

        if (!strcmp(lx, KW_PROC))
                return (stmt *)parse_stmt_proc(ctx);
        if (!strcmp(lx, KW_LET))
                return NULL;

        ctx->err.msg = str_from("invalid keyword statement");
        ctx->err.loc = hd->loc;

        return NULL;
}

static stmt *
parse_stmt(parser_context *ctx)
{
        const token *hd = lexer_peek(ctx->l, 0);

        switch (hd->kind) {
        case TK_KW: return parse_stmt_kw(ctx);
        default:    assert(0);
        }

        ctx->err.msg = str_from("invalid statement");
        ctx->err.loc = hd->loc;

        return NULL;
}

parser_context
parser_parse(lexer *l)
{
        parser_context ctx;

        ctx = (parser_context) {
                .l       = l,
                .global  = 1,
                .stmts   = array_empty(stmtp_ar),
                .err     = {
                        .msg = {0},
                        .loc = {0},
                },
        };

        while (LSP(ctx.l, 0)->kind != TK_EOF) {
                stmt *s = parse_stmt(&ctx);

                if (ctx.err.msg.len == 0)
                        array_append(ctx.stmts, s);
                else
                        fatal("%s%s", loc_cstr(ctx.err.loc), ctx.err.msg.chars);
        }

        return ctx;
}
