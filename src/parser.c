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
        const token *hd = expect(ctx, TOKEN_TYPE_KEYWORD);
        const char *lx = hd->lx;

        if (!strcmp(lx, KWD_I8)) {
                forge_todo("i8");
        } else if (!strcmp(lx, KWD_I16)) {
                forge_todo("i16");
        } else if (!strcmp(lx, KWD_I32)) {
                return (type *)type_i32_alloc();
        } else if (!strcmp(lx, KWD_I64)) {
                forge_todo("i64");
        } else if (!strcmp(lx, KWD_U8)) {
                return (type *)type_u8_alloc();
        } else if (!strcmp(lx, KWD_U16)) {
                forge_todo("u16");
        } else if (!strcmp(lx, KWD_U32)) {
                return (type *)type_u32_alloc();
        } else if (!strcmp(lx, KWD_U64)) {
                forge_todo("u64");
        }
        forge_err_wargs("could not parse type: %s", hd->lx);

}

expr *
parse_primary_expr(parser_context *ctx)
{
        expr *left = NULL;

        while (1) {
                token *hd = lexer_peek(ctx->l, 0);
                if (!hd) return left;

                switch (hd->ty) {
                case TOKEN_TYPE_IDENTIFIER: {
                        forge_todo("identifers");
                } break;
                case TOKEN_TYPE_INTEGER_LITERAL: {
                        const token *i = lexer_next(ctx->l);
                        left = (expr *)expr_integer_literal_alloc(i);
                } break;
                case TOKEN_TYPE_STRING_LITERAL: {
                        forge_todo("strings");
                } break;
                default: return left;
                }
        }

        return left; // unreachable
}

expr *
parse_member_expr(parser_context *ctx)
{
        return parse_primary_expr(ctx);
}

expr *
parse_unary_expr(parser_context *ctx)
{
        /* token *cur = lexer_peek(ctx->l, 0); */
        /* if (cur && (cur->ty == TOKEN_TYPE_MINUS */
        /*             || cur->ty == TOKEN_TYPE_PLUS */
        /*             || cur->ty == TOKEN_TYPE_BANG */
        /*             || cur->ty == TOKEN_TYPE_ASTERISK */
        /*             || cur->ty == TOKEN_TYPE_AMPERSAND)) { */
        /*         token *op = lexer_next(ctx->l); */
        /*         expr *operand = parse_unary_expr(ctx); */
        /*         return (expr *)expr_un_alloc(operand, op); */
        /* } */
        return parse_member_expr(ctx);
}

static expr *
parse_multiplicitate_expr(parser_context *ctx)
{
        expr *lhs = parse_unary_expr(ctx);
        token *cur = lexer_peek(ctx->l, 0);
        while (cur && (cur->ty == TOKEN_TYPE_ASTERISK
                       || cur->ty == TOKEN_TYPE_FORWARDSLASH
                       || cur->ty == TOKEN_TYPE_PERCENT)) {
                token *op = lexer_next(ctx->l);
                expr *rhs = parse_unary_expr(ctx);
                expr_bin *bin = expr_bin_alloc(lhs, op, rhs);
                lhs = (expr *)bin;
                cur = lexer_peek(ctx->l, 0);
        }
        return lhs;
}

static expr *
parse_additive_expr(parser_context *ctx)
{
        expr *lhs = parse_multiplicitate_expr(ctx);
        token *cur = lexer_peek(ctx->l, 0);
        while (cur && (cur->ty == TOKEN_TYPE_PLUS
                       || cur->ty == TOKEN_TYPE_MINUS)) {
                token *op = lexer_next(ctx->l);
                expr *rhs = parse_multiplicitate_expr(ctx);
                expr_bin *bin = expr_bin_alloc(lhs, op, rhs);
                lhs = (expr *)bin;
                cur = lexer_peek(ctx->l, 0);
        }
        return lhs;
}

static expr *
parse_equalitative_expr(parser_context *ctx)
{
        expr *lhs = parse_additive_expr(ctx);
        token *cur = lexer_peek(ctx->l, 0);
        while (cur && (cur->ty == TOKEN_TYPE_DOUBLE_EQUALS
                       || cur->ty == TOKEN_TYPE_GREATERTHAN_EQUALS
                       || cur->ty == TOKEN_TYPE_GREATERTHAN
                       || cur->ty == TOKEN_TYPE_LESSTHAN_EQUALS
                       || cur->ty == TOKEN_TYPE_LESSTHAN
                       || cur->ty == TOKEN_TYPE_BANG_EQUALS)) {
                token *op = lexer_next(ctx->l);
                expr *rhs = parse_additive_expr(ctx);
                expr_bin *bin = expr_bin_alloc(lhs, op, rhs);
                lhs = (expr *)bin;
                cur = lexer_peek(ctx->l, 0);
        }
        return lhs;
}

static expr *
parse_logical_expr(parser_context *ctx)
{
        expr *lhs = parse_equalitative_expr(ctx);
        token *cur = lexer_peek(ctx->l, 0);
        while (cur && (cur->ty == TOKEN_TYPE_DOUBLE_AMPERSAND
                       || cur->ty == TOKEN_TYPE_DOUBLE_PIPE)) {
                token *op = lexer_next(ctx->l);
                expr *rhs = parse_equalitative_expr(ctx);
                expr_bin *bin = expr_bin_alloc(lhs, op, rhs);
                lhs = (expr *)bin;
                cur = lexer_peek(ctx->l, 0);
        }
        return lhs;
}

static expr *
parse_assignment_expr(parser_context *ctx)
{
        expr *lhs = parse_logical_expr(ctx);

        token *cur = lexer_peek(ctx->l, 0);
        if (!cur) return lhs;

        switch (cur->ty) {
        case TOKEN_TYPE_EQUALS:
        case TOKEN_TYPE_PLUS_EQUALS:
        case TOKEN_TYPE_MINUS_EQUALS:
        case TOKEN_TYPE_ASTERISK_EQUALS:
        case TOKEN_TYPE_FORWARDSLASH_EQUALS:
        case TOKEN_TYPE_PERCENT_EQUALS:
        case TOKEN_TYPE_AMPERSAND_EQUALS:
        case TOKEN_TYPE_PIPE_EQUALS:
        case TOKEN_TYPE_UPTICK_EQUALS: {
                const token *op = lexer_next(ctx->l);
                expr *rhs = parse_assignment_expr(ctx);
                return (expr *)expr_mut_alloc(lhs, op, rhs);
        }
        default:
                return lhs;
        }
}

static expr *
parse_expr(parser_context *ctx)
{
        return parse_assignment_expr(ctx);
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
