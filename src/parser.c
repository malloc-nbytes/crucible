#include "parser.h"
#include "lexer.h"
#include "kwds.h"

#include <forge/array.h>
#include <forge/utils.h>
#include <forge/err.h>

#include <assert.h>
#include <string.h>

#define LSP(l, p) lexer_peek(l, p) && lexer_peek(l, p)

typedef struct {
        lexer *l;
        int in_global;
} parser_context;

static stmt *parse_stmt(parser_context *ctx);

token *
expect(parser_context *ctx, token_type ty)
{
        token *t = lexer_next(ctx->l);
        if (t->ty != ty) {
                forge_err_wargs("expected token of type `%s` but got `%s`",
                                token_type_to_cstr(ty), token_type_to_cstr(t->ty));
        }
        return t;
}

token *
expectkw(parser_context *ctx, const char *kw)
{
        token *t = lexer_next(ctx->l);
        if (t->ty != TOKEN_TYPE_KEYWORD) {
                forge_err_wargs("expected token of type `%s` but got `%s`",
                                token_type_to_cstr(TOKEN_TYPE_KEYWORD), token_type_to_cstr(t->ty));
        }
        if (strcmp(t->lx, kw)) {
                forge_err_wargs("expected keyword `%s` but got `%s`",
                                kw, t->lx);
        }
        return t;
}

static type *
parse_type(parser_context *ctx)
{
        token *hd = expect(ctx, TOKEN_TYPE_KEYWORD);
        const char *lx = hd->lx;

        if (!strcmp(lx, KWD_I8)
            || !strcmp(lx, KWD_I16)
            || !strcmp(lx, KWD_I32)
            || !strcmp(lx, KWD_I64)
            || !strcmp(lx, KWD_U8)
            || !strcmp(lx, KWD_U16)
            || !strcmp(lx, KWD_U32)
            || !strcmp(lx, KWD_U64)) {
                ;
        }
        forge_todo("types");

}

static expr *
parse_expr(parser_context *ctx)
{
        NOOP(ctx);
        forge_todo("parse_expr");
}

static stmt_let *
parse_stmt_let(parser_context *ctx)
{
        expectkw(ctx, KWD_LET);
        token *id = expect(ctx, TOKEN_TYPE_IDENTIFIER);
        (void)expect(ctx, TOKEN_TYPE_COLON);
        type *ty = parse_type(ctx);
        (void)expect(ctx, TOKEN_TYPE_EQUALS);
        expr *e = parse_expr(ctx);
        (void)expect(ctx, TOKEN_TYPE_SEMICOLON);
        return stmt_let_alloc(id, ty, e);
}

static stmt_expr *
parse_stmt_expr(parser_context *ctx)
{
        expr *e = parse_expr(ctx);
        (void)expect(ctx, TOKEN_TYPE_SEMICOLON);
        return stmt_expr_alloc(e);
}

parameter_array
parse_parameters(parser_context *ctx)
{
        (void)expect(ctx, TOKEN_TYPE_LEFT_PARENTHESIS);

        parameter_array ar = dyn_array_empty(parameter_array);

        if (!strcmp(lexer_peek(ctx->l, 0)->lx, KWD_VOID)) {
                (void)expect(ctx, TOKEN_TYPE_RIGHT_PARENTHESIS);
                goto done;
        }

        while (LSP(ctx->l, 0)->ty != TOKEN_TYPE_RIGHT_PARENTHESIS) {
                token *id = expect(ctx, TOKEN_TYPE_IDENTIFIER);
                (void)expect(ctx, TOKEN_TYPE_COLON);
                type *type = parse_type(ctx);

                parameter param = (parameter) {
                        .id = id,
                        .type = type,
                };
                dyn_array_append(ar, param);

                const token *t = lexer_peek(ctx->l, 0);

                if (lexer_peek(ctx->l, 0)->ty == TOKEN_TYPE_COMMA) {
                        lexer_discard(ctx->l);
                        if (t->ty == TOKEN_TYPE_RIGHT_PARENTHESIS) {
                                break;
                        }
                } else if (t->ty == TOKEN_TYPE_RIGHT_PARENTHESIS) {
                        break;
                } else {
                        forge_err("expected a comma or right parenthesis");
                }
        }

 done:
        (void)expect(ctx, TOKEN_TYPE_RIGHT_PARENTHESIS);
        return ar;
}

static stmt_block *
parse_stmt_block(parser_context *ctx)
{
        (void)expect(ctx, TOKEN_TYPE_LEFT_CURLY);
        stmt_array ar = dyn_array_empty(stmt_array);
        while (LSP(ctx->l, 0)->ty != TOKEN_TYPE_RIGHT_CURLY) {
                dyn_array_append(ar, parse_stmt(ctx));
        }
        (void)expect(ctx, TOKEN_TYPE_RIGHT_CURLY);
        return stmt_block_alloc(ar);
}

static stmt_proc *
parse_stmt_proc(parser_context *ctx)
{
        expectkw(ctx, KWD_PROC);
        token *id = expect(ctx, TOKEN_TYPE_IDENTIFIER);
        parameter_array params = parse_parameters(ctx);
        (void)expect(ctx, TOKEN_TYPE_COLON);
        type *ty = parse_type(ctx);
        stmt *blk = NULL;

        ctx->in_global = 0;

        if (lexer_peek(ctx->l, 0)->ty == TOKEN_TYPE_EQUALS) {
                lexer_discard(ctx->l);
                blk = parse_stmt(ctx);
        } else {
                blk = (stmt *)parse_stmt_block(ctx);
        }

        ctx->in_global = 1;

        return stmt_proc_alloc(id, params, ty, blk);
}

static stmt *
parse_keyword_stmt(parser_context *ctx)
{
        token *hd = lexer_peek(ctx->l, 0);

        if (!strcmp(hd->lx, KWD_LET)) {
                return (stmt *)parse_stmt_let(ctx);
        } else if (!strcmp(hd->lx, KWD_PROC)) {
                return (stmt *)parse_stmt_proc(ctx);
        }

        assert(0 && "todo");
        return NULL; // unreachable
}

static stmt *
parse_stmt(parser_context *ctx)
{
        if (kwds_iskw(lexer_peek(ctx->l, 0)->lx)) {
                return parse_keyword_stmt(ctx);
        }
        return (stmt *)parse_stmt_expr(ctx);
}

program
parser_create_program(lexer *l)
{
        parser_context ctx = (parser_context) {
                .l = l,
                .in_global = 1,
        };

        program p = (program) {
                .stmts = dyn_array_empty(stmt_array),
        };

        while (LSP(ctx.l, 0)->ty != TOKEN_TYPE_EOF) {
                dyn_array_append(p.stmts, parse_stmt(&ctx));
        }

        return p;
}
