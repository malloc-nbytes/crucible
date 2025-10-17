#include "parser.h"
#include "lexer.h"
#include "kwds.h"
#include "mem.h"

#include <forge/array.h>
#include <forge/utils.h>
#include <forge/err.h>
#include <forge/str.h>

#include <assert.h>
#include <string.h>

#define LSP(l, p) lexer_peek(l, p) && lexer_peek(l, p)

typedef struct {
        lexer *l;
        int in_global;
        const char *module;
} parser_context;

static stmt *parse_stmt(parser_context *ctx);
static expr *parse_expr(parser_context *ctx);
parameter_array parse_parameters(parser_context *ctx, int *variadic);

token *
expect(parser_context *ctx, token_type ty)
{
        token *t = lexer_next(ctx->l);
        if (t->ty != ty) {
                forge_err_wargs("%sexpected token of type `%s` but got `%s`",
                                loc_err(t->loc), token_type_to_cstr(ty), t->lx);
        }
        return t;
}

token *
expect_or(parser_context *ctx,
          token_type      t0,
          token_type      t1)
{
        token *t = lexer_next(ctx->l);
        if (t->ty != t0 && t->ty != t1) {
                forge_err_wargs("%sexpected token of type `%s` or `%s` but got `%s`",
                                loc_err(t->loc), token_type_to_cstr(t0), token_type_to_cstr(t1), t->lx);
        }
        return t;
}

token *
expectkw(parser_context *ctx, const char *kw)
{
        token *t = lexer_next(ctx->l);
        if (t->ty != TOKEN_TYPE_KEYWORD) {
                forge_err_wargs("%sexpected token of type `%s` but got `%s`",
                                loc_err(t->loc), token_type_to_cstr(TOKEN_TYPE_KEYWORD), token_type_to_cstr(t->ty));
        }
        if (strcmp(t->lx, kw)) {
                forge_err_wargs("%sexpected keyword `%s` but got `%s`",
                                loc_err(t->loc), kw, t->lx);
        }
        return t;
}

static type *
parse_type(parser_context *ctx)
{
        const token *hd = lexer_next(ctx->l);
        const char *lx = hd->lx;
        type *ty = NULL;

        if (hd->ty == TOKEN_TYPE_LEFT_SQUARE) {
                type *inner = parse_type(ctx);
                int len = -1;

                if (LSP(ctx->l, 0)->ty == TOKEN_TYPE_SEMICOLON) {
                        lexer_discard(ctx->l); // ;
                        len = atoi(expect(ctx, TOKEN_TYPE_INTEGER_LITERAL)->lx);
                }
                ty = (type *)type_list_alloc(inner, len);
                (void)expect(ctx, TOKEN_TYPE_RIGHT_SQUARE);
                return ty;
        }

        if (!strcmp(lx, KWD_I8)) {
                ty = (type *)type_i8_alloc();
        } else if (!strcmp(lx, KWD_I16)) {
                forge_todo("i16");
        } else if (!strcmp(lx, KWD_I32)) {
                ty = (type *)type_i32_alloc();
        } else if (!strcmp(lx, KWD_I64)) {
                ty = (type *)type_i64_alloc();
        } else if (!strcmp(lx, KWD_U8)) {
                ty = (type *)type_u8_alloc();
        } else if (!strcmp(lx, KWD_U16)) {
                forge_todo("u16");
        } else if (!strcmp(lx, KWD_U32)) {
                ty = (type *)type_u32_alloc();
        } else if (!strcmp(lx, KWD_U64)) {
                forge_todo("u64");
        } else if (!strcmp(lx, KWD_VOID)) {
                ty = (type *)type_void_alloc();
        } else if (hd->ty == TOKEN_TYPE_BANG) {
                ty = (type *)type_noreturn_alloc();
        } else if (hd->ty == TOKEN_TYPE_KEYWORD && !strcmp(hd->lx, KWD_BOOL)) {
                ty = (type *)type_bool_alloc();
        } else if (hd->ty == TOKEN_TYPE_KEYWORD && !strcmp(hd->lx, KWD_SIZET)) {
                ty = (type *)type_sizet_alloc();
        } else if (hd->ty == TOKEN_TYPE_KEYWORD && !strcmp(hd->lx, KWD_PROC)) {
                (void)expect(ctx, TOKEN_TYPE_LEFT_PARENTHESIS);

                type       *rettype  = NULL;
                type_array  params   = dyn_array_empty(type_array);
                int         variadic = 0;

                while (LSP(ctx->l, 0)->ty != TOKEN_TYPE_RIGHT_PARENTHESIS) {
                        if (LSP(ctx->l, 0)->ty == TOKEN_TYPE_ELLIPSIS) {
                                lexer_discard(ctx->l); // ...
                                variadic = 1;
                                break;
                        }

                        type *ptype = parse_type(ctx);

                        if (ptype->kind == TYPE_KIND_VOID) {
                                if (params.len != 0) {
                                        forge_err_wargs("%s`void` can only be used when no parameters are expected",
                                                        loc_err(hd->loc));
                                }
                                free(ptype);
                                break;
                        }

                        dyn_array_append(params, ptype);

                        if (LSP(ctx->l, 0)->ty == TOKEN_TYPE_COMMA) {
                                lexer_discard(ctx->l);
                        } else {
                                break;
                        }
                }

                (void)expect(ctx, TOKEN_TYPE_RIGHT_PARENTHESIS);
                (void)expect(ctx, TOKEN_TYPE_COLON);

                rettype = parse_type(ctx);

                ty = (type *)type_procptr_alloc(params, rettype, variadic);
        } else if (hd->ty == TOKEN_TYPE_IDENTIFIER) {
                ty = (type *)type_custom_alloc(hd->lx);
        } else {
                forge_err_wargs("unknown type: `%s`", hd->lx);
        }

        // Handles all pointer types (ex: u8**).
        while (LSP(ctx->l, 0)->ty == TOKEN_TYPE_ASTERISK) {
                lexer_discard(ctx->l);
                ty = (type *)type_ptr_alloc(ty);
        }

        return ty;
}

// Note: This function expects the opening and
//       closing parenthesis `( )` and will consume those.
static expr_array
parse_comma_sep_exprs(parser_context *ctx)
{
        (void)expect(ctx, TOKEN_TYPE_LEFT_PARENTHESIS);
        expr_array ar = dyn_array_empty(expr_array);
        while (LSP(ctx->l, 0)->ty != TOKEN_TYPE_RIGHT_PARENTHESIS) {
                dyn_array_append(ar, parse_expr(ctx));
                if (LSP(ctx->l, 0)->ty == TOKEN_TYPE_COMMA) {
                        lexer_discard(ctx->l);
                } else {
                        break;
                }
        }
        (void)expect(ctx, TOKEN_TYPE_RIGHT_PARENTHESIS);
        return ar;
}

static parameter_array
parse_bracket_ids_and_types(parser_context *ctx)
{
        parameter_array members = dyn_array_empty(parameter_array);

        (void)expect(ctx, TOKEN_TYPE_LEFT_CURLY);

        while (LSP(ctx->l, 0)->ty != TOKEN_TYPE_RIGHT_CURLY) {
                const token *id = expect(ctx, TOKEN_TYPE_IDENTIFIER);
                (void)expect(ctx, TOKEN_TYPE_COLON);
                type *type = parse_type(ctx);

                dyn_array_append(members, ((parameter) {
                        .id = id,
                        .type = type,
                }));

                if (LSP(ctx->l, 0)->ty == TOKEN_TYPE_COMMA) {
                        lexer_discard(ctx->l);
                } else {
                        break;
                }
        }

        (void)expect(ctx, TOKEN_TYPE_RIGHT_CURLY);

        return members;
}

static expr_brace_init *
parse_brace_initializer(parser_context *ctx)
{
        (void)expect(ctx, TOKEN_TYPE_LEFT_CURLY);

        token_array ids = dyn_array_empty(token_array);
        expr_array exprs = dyn_array_empty(expr_array);

        while (LSP(ctx->l, 0)->ty != TOKEN_TYPE_RIGHT_CURLY) {
                (void)expect(ctx, TOKEN_TYPE_PERIOD);

                dyn_array_append(ids, (expect(ctx, TOKEN_TYPE_IDENTIFIER)));
                (void)expect(ctx, TOKEN_TYPE_EQUALS);
                dyn_array_append(exprs, (parse_expr(ctx)));

                if (LSP(ctx->l, 0)->ty == TOKEN_TYPE_COMMA) {
                        lexer_discard(ctx->l); // ,
                } else {
                        break;
                }
        }

        (void)expect(ctx, TOKEN_TYPE_RIGHT_CURLY);

        return expr_brace_init_alloc(ids, exprs);
}

static expr_arrayinit *
parse_arrayinit(parser_context *ctx)
{
        expr_array exprs = dyn_array_empty(expr_array);
        int zeroed = 1;

        (void)expect(ctx, TOKEN_TYPE_LEFT_SQUARE);
        while (LSP(ctx->l, 0)->ty != TOKEN_TYPE_RIGHT_SQUARE) {
                if (exprs.len > 1) zeroed = 0;

                expr *e = parse_expr(ctx);
                dyn_array_append(exprs, e);

                if (zeroed && e->kind == EXPR_KIND_INTEGER_LITERAL) {
                        if (strcmp(((expr_integer_literal *)e)->i->lx, "0")) {
                                zeroed = 0;
                        }
                }

                if (LSP(ctx->l, 0)->ty == TOKEN_TYPE_COMMA) {
                        lexer_discard(ctx->l); // ,
                } else {
                        break;
                }
        }
        (void)expect(ctx, TOKEN_TYPE_RIGHT_SQUARE);

        return expr_arrayinit_alloc(exprs, zeroed);
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
                        const token *i = lexer_next(ctx->l);

                        if (LSP(ctx->l, 0)->ty == TOKEN_TYPE_DOUBLE_COLON) {
                                lexer_discard(ctx->l); // ::
                                expr *right = parse_primary_expr(ctx);
                                left = (expr *)expr_namespace_alloc(i, right);
                        } else {
                                left = (expr *)expr_identifier_alloc(i);
                        }

                        left->loc = hd->loc;
                } break;
                case TOKEN_TYPE_INTEGER_LITERAL: {
                        const token *i = lexer_next(ctx->l);
                        left = (expr *)expr_integer_literal_alloc(i);
                        left->loc = hd->loc;
                } break;
                case TOKEN_TYPE_STRING_LITERAL: {
                        const token *s = lexer_next(ctx->l);
                        left = (expr *)expr_string_literal_alloc(s);
                        left->loc = hd->loc;
                } break;
                case TOKEN_TYPE_CHARACTER_LITERAL: {
                        const token *c = lexer_next(ctx->l);
                        left = (expr *)expr_character_literal_alloc(c);
                        left->loc = hd->loc;
                } break;
                case TOKEN_TYPE_LEFT_SQUARE: {
                        if (!left) {
                                left = (expr *)parse_arrayinit(ctx);
                        } else {
                                lexer_discard(ctx->l); // [
                                expr *idx = parse_expr(ctx);
                                (void)expect(ctx, TOKEN_TYPE_RIGHT_SQUARE);
                                left = (expr *)expr_index_alloc(left, idx);
                        }
                        left->loc = hd->loc;
                } break;
                case TOKEN_TYPE_LEFT_PARENTHESIS: {
                        // TODO: redo breace_initializer parsing
                        /* if (struct) {
                           left = (expr *)parse_brace_initializer(ctx);
                           left->loc = hd->loc;
                           }
                         */
                        if (left) {
                                // function call
                                expr_array args = parse_comma_sep_exprs(ctx);
                                left = (expr *)expr_proccall_alloc(left, args);
                        } else if (kwds_isty(lexer_peek(ctx->l, 1)->lx)) {
                                // TODO: somehow handle struct names and things
                                //       like `[i32]`.
                                lexer_discard(ctx->l); // (
                                type *ty = parse_type(ctx);
                                (void)expect(ctx, TOKEN_TYPE_RIGHT_PARENTHESIS);
                                left = (expr *)expr_cast_alloc(ty, parse_primary_expr(ctx));
                        } else {
                                // Math expression
                                lexer_discard(ctx->l); // (
                                left = parse_expr(ctx);
                                (void)expect(ctx, TOKEN_TYPE_RIGHT_PARENTHESIS);
                        }
                        left->loc = hd->loc;
                } break;
                case TOKEN_TYPE_LEFT_CURLY: {
                        assert(0);
                } break;
                case TOKEN_TYPE_KEYWORD: {
                        const token *kw = lexer_next(ctx->l);
                        if (!strcmp(kw->lx, KWD_TRUE) || !strcmp(kw->lx, KWD_FALSE)) {
                                left = (expr *)expr_bool_literal_alloc(kw);
                                left->loc = kw->loc;
                        } else if (!strcmp(kw->lx, KWD_NULL)) {
                                left = (expr *)expr_null_alloc();
                                left->loc = hd->loc;
                        } else if (!strcmp(kw->lx, KWD_CAST)) {
                                (void)expect(ctx, TOKEN_TYPE_LESSTHAN);
                                type *ty = parse_type(ctx);
                                (void)expect(ctx, TOKEN_TYPE_GREATERTHAN);
                                (void)expect(ctx, TOKEN_TYPE_LEFT_PARENTHESIS);
                                left = (expr *)expr_cast_alloc(ty, parse_expr(ctx));
                                (void)expect(ctx, TOKEN_TYPE_RIGHT_PARENTHESIS);
                                left->loc = hd->loc;
                        } else {
                                return left;
                        }
                } break;
                default: return left;
                }
        }

        assert(0 && "unreachable");
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
        token *cur = lexer_peek(ctx->l, 0);
        if (cur && (cur->ty == TOKEN_TYPE_MINUS
                    || cur->ty == TOKEN_TYPE_PLUS
                    || cur->ty == TOKEN_TYPE_BANG
                    || cur->ty == TOKEN_TYPE_ASTERISK
                    || cur->ty == TOKEN_TYPE_AMPERSAND)) {
                token *op = lexer_next(ctx->l);
                expr *lhs = (expr *)parse_unary_expr(ctx);
                ((expr *)lhs)->loc = op->loc;
                return (expr *)expr_un_alloc(op, lhs);
        }
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
                ((expr *)bin)->loc = lhs->loc;
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
                ((expr *)bin)->loc = lhs->loc;
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
                ((expr *)bin)->loc = lhs->loc;
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
                ((expr *)bin)->loc = lhs->loc;
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
parse_parameters(parser_context *ctx, int *variadic)
{
        *variadic = 0;

        (void)expect(ctx, TOKEN_TYPE_LEFT_PARENTHESIS);

        parameter_array ar = dyn_array_empty(parameter_array);

        if (!strcmp(lexer_peek(ctx->l, 0)->lx, KWD_VOID)) {
                lexer_discard(ctx->l); // void
                goto done;
        }

        while (LSP(ctx->l, 0)->ty != TOKEN_TYPE_RIGHT_PARENTHESIS) {
                token *id = lexer_peek(ctx->l, 0);
                if (id->ty != TOKEN_TYPE_IDENTIFIER) {
                        if (id->ty == TOKEN_TYPE_ELLIPSIS) {
                                lexer_discard(ctx->l); // ...
                                if (lexer_peek(ctx->l, 0)->ty != TOKEN_TYPE_RIGHT_PARENTHESIS) {
                                        forge_err("variadic parameter must come at the end of the parameter list");
                                }
                                *variadic = 1;
                                break;
                        } else {
                                forge_err_wargs("%sexpected either `)` or `...`", loc_err(id->loc));
                        }
                }

                id = expect(ctx, TOKEN_TYPE_IDENTIFIER);
                (void)expect(ctx, TOKEN_TYPE_COLON);
                type *type = parse_type(ctx);

                parameter param = (parameter) {
                        .id = id,
                        .type = type,
                        .resolved = NULL,
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
        int export = !strcmp(lexer_peek(ctx->l, 0)->lx, KWD_EXPORT);
        if (export) {
                lexer_discard(ctx->l); // export
        }
        lexer_discard(ctx->l); // proc

        token *id = expect(ctx, TOKEN_TYPE_IDENTIFIER);
        int variadic = 0;
        parameter_array params = parse_parameters(ctx, &variadic);
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

        return stmt_proc_alloc(export, id, params, variadic, ty, blk);
}

static stmt_extern_proc *
parse_stmt_extern(parser_context *ctx)
{
        int export = !strcmp(lexer_peek(ctx->l, 0)->lx, KWD_EXPORT);
        if (export) {
                lexer_discard(ctx->l); // export
        }

        (void)expectkw(ctx, KWD_EXTERN);
        (void)expectkw(ctx, KWD_PROC);
        token *id = expect(ctx, TOKEN_TYPE_IDENTIFIER);
        int variadic = 0;
        parameter_array params = parse_parameters(ctx, &variadic);
        (void)expect(ctx, TOKEN_TYPE_COLON);
        type *ty = parse_type(ctx);
        (void)expect(ctx, TOKEN_TYPE_SEMICOLON);
        return stmt_extern_proc_alloc(id, params, variadic, ty, export);
}

static stmt_return *
parse_stmt_return(parser_context *ctx)
{
        (void)expectkw(ctx, KWD_RETURN);
        expr *e = parse_expr(ctx);
        (void)expect(ctx, TOKEN_TYPE_SEMICOLON);
        return stmt_return_alloc(e);
}

static stmt_exit *
parse_stmt_exit(parser_context *ctx)
{
        (void)expectkw(ctx, KWD_EXIT);
        expr *e = parse_expr(ctx);
        (void)expect(ctx, TOKEN_TYPE_SEMICOLON);
        return stmt_exit_alloc(e);
}

static stmt *
parse_stmt_if(parser_context *ctx)
{
        lexer_discard(ctx->l); // if

        (void)expect(ctx, TOKEN_TYPE_LEFT_PARENTHESIS);
        expr *e     = parse_expr(ctx);
        (void)expect(ctx, TOKEN_TYPE_RIGHT_PARENTHESIS);

        stmt *then  = parse_stmt(ctx);
        stmt *else_ = NULL;

        token *t1 = lexer_peek(ctx->l, 0);
        token *t2 = lexer_peek(ctx->l, 1);

        int t1_else = t1 && t1->ty == TOKEN_TYPE_KEYWORD && !strcmp(t1->lx, KWD_ELSE);
        int t2_if   = t2 && t2->ty == TOKEN_TYPE_KEYWORD && !strcmp(t2->lx, KWD_IF);

        if (t1_else && t2_if) {
                lexer_discard(ctx->l); // else
                else_ = parse_stmt_if(ctx);
        } else if (t1_else) {
                lexer_discard(ctx->l); // else
                else_ = parse_stmt(ctx);
        }

        return (stmt *)stmt_if_alloc(e, then, else_);
}

static stmt_while *
parse_stmt_while(parser_context *ctx)
{
        lexer_discard(ctx->l); // while
        (void)expect(ctx, TOKEN_TYPE_LEFT_PARENTHESIS);
        expr *e = parse_expr(ctx);
        (void)expect(ctx, TOKEN_TYPE_RIGHT_PARENTHESIS);
        stmt *body = parse_stmt(ctx);
        return stmt_while_alloc(e, body);
}

static stmt_for *
parse_stmt_for(parser_context *ctx)
{
        stmt *init   = NULL;
        expr *e      = NULL;
        expr *after  = NULL;
        stmt *body   = NULL;

        lexer_discard(ctx->l); // for
        (void)expect(ctx, TOKEN_TYPE_LEFT_PARENTHESIS);
        init = parse_stmt(ctx);
        e = parse_expr(ctx);
        (void)expect(ctx, TOKEN_TYPE_SEMICOLON);
        after = parse_expr(ctx);
        (void)expect(ctx, TOKEN_TYPE_RIGHT_PARENTHESIS);
        body = parse_stmt(ctx);

        return stmt_for_alloc(init, e, after, body);
}

static stmt_break *
parse_stmt_break(parser_context *ctx)
{
        (void)expectkw(ctx, KWD_BREAK);
        (void)expect(ctx, TOKEN_TYPE_SEMICOLON);
        return stmt_break_alloc();
}

static stmt_continue *
parse_stmt_continue(parser_context *ctx)
{
        (void)expectkw(ctx, KWD_CONTINUE);
        (void)expect(ctx, TOKEN_TYPE_SEMICOLON);
        return stmt_continue_alloc();
}

static stmt_struct *
parse_stmt_struct(parser_context *ctx)
{
        (void)expectkw(ctx, KWD_STRUCT);
        const token *id = expect(ctx, TOKEN_TYPE_IDENTIFIER);
        parameter_array members = parse_bracket_ids_and_types(ctx);
        return stmt_struct_alloc(id, members);
}

static stmt_module *
parse_stmt_module(parser_context *ctx)
{
        (void)expectkw(ctx, KWD_MODULE);
        const token *modname = expect(ctx, TOKEN_TYPE_IDENTIFIER);
        (void)expectkw(ctx, KWD_WHERE);
        ctx->module = strdup(modname->lx);
        return stmt_module_alloc(modname);
}

static stmt_import *
parse_stmt_import(parser_context *ctx)
{
        char *filepath = NULL;
        int local = 0;

        lexer_discard(ctx->l); // import

        token *path = expect_or(ctx, TOKEN_TYPE_IDENTIFIER, TOKEN_TYPE_STRING_LITERAL);

        if (path->ty == TOKEN_TYPE_IDENTIFIER) {
                forge_str buf = forge_str_from(path->lx);

                while (LSP(ctx->l, 0)->ty != TOKEN_TYPE_SEMICOLON) {
                        token *next = expect_or(ctx, TOKEN_TYPE_IDENTIFIER, TOKEN_TYPE_PERIOD);
                        forge_str_concat(&buf, next->ty == TOKEN_TYPE_PERIOD ? "/" : next->lx);
                }

                forge_str_concat(&buf, ".cr");

                filepath = buf.data;
        } else {
                local = 1;
                filepath = strdup(path->lx);
        }

        (void)expect(ctx, TOKEN_TYPE_SEMICOLON);

        return stmt_import_alloc(filepath, local);
}

static stmt *
parse_stmt_export(parser_context *ctx)
{
        int ok = lexer_peek(ctx->l, 1) != NULL;

        if (ok && !strcmp(lexer_peek(ctx->l, 1)->lx, KWD_PROC)) {
                return (stmt *)parse_stmt_proc(ctx);
        } else if (ok && !strcmp(lexer_peek(ctx->l, 1)->lx, KWD_EXTERN)) {
                return (stmt *)parse_stmt_extern(ctx);
        }

        forge_err_wargs("%sinvalid use of keyword '%s'", loc_err(ctx->l->hd->loc), KWD_EXTERN);
        return NULL;
}

static stmt_embed *
parse_stmt_embed(parser_context *ctx)
{
        (void)expectkw(ctx, KWD_EMBED);
        (void)expect(ctx, TOKEN_TYPE_LEFT_CURLY);

        token_array lns = dyn_array_empty(token_array);
        while (LSP(ctx->l, 0)->ty != TOKEN_TYPE_RIGHT_CURLY) {
                dyn_array_append(lns, expect(ctx, TOKEN_TYPE_STRING_LITERAL));
                if (LSP(ctx->l, 0)->ty == TOKEN_TYPE_COMMA) {
                        lexer_discard(ctx->l); // ,
                } else {
                        break;
                }
        }
        (void)expect(ctx, TOKEN_TYPE_RIGHT_CURLY);
        return stmt_embed_alloc(lns);
}

static stmt_enum *
parse_stmt_enum(parser_context *ctx)
{
        token_array ids = dyn_array_empty(token_array);
        expr_array exprs = dyn_array_empty(expr_array);

        (void)expectkw(ctx, KWD_ENUM);
        const token *id = expect(ctx, TOKEN_TYPE_IDENTIFIER);
        (void)expect(ctx, TOKEN_TYPE_LEFT_CURLY);

        while (LSP(ctx->l, 0)->ty != TOKEN_TYPE_RIGHT_CURLY) {
                token *id = expect(ctx, TOKEN_TYPE_IDENTIFIER);
                (void)expect(ctx, TOKEN_TYPE_EQUALS);
                expr *e = parse_expr(ctx);
                if (LSP(ctx->l, 0)->ty == TOKEN_TYPE_COMMA) {
                        lexer_discard(ctx->l); // ,
                } else {
                        break;
                }
        }

        (void)expect(ctx, TOKEN_TYPE_RIGHT_CURLY);
        return stmt_enum_alloc(id, ids, exprs);
}

static stmt *
parse_keyword_stmt(parser_context *ctx)
{
        token *hd = lexer_peek(ctx->l, 0);

        if (!strcmp(hd->lx, KWD_LET)) {
                return (stmt *)parse_stmt_let(ctx);
        } else if (!strcmp(hd->lx, KWD_PROC)) {
                return (stmt *)parse_stmt_proc(ctx);
        } else if (!strcmp(hd->lx, KWD_EXPORT)) {
                return (stmt *)parse_stmt_export(ctx);
        } else if (!strcmp(hd->lx, KWD_RETURN)) {
                return (stmt *)parse_stmt_return(ctx);
        } else if (!strcmp(hd->lx, KWD_EXIT)) {
                return (stmt *)parse_stmt_exit(ctx);
        } else if (!strcmp(hd->lx, KWD_EXTERN)) {
                return (stmt *)parse_stmt_extern(ctx);
        } else if (!strcmp(hd->lx, KWD_IF)) {
                return (stmt *)parse_stmt_if(ctx);
        } else if (!strcmp(hd->lx, KWD_WHILE)) {
                return (stmt *)parse_stmt_while(ctx);
        } else if (!strcmp(hd->lx, KWD_FOR)) {
                return (stmt *)parse_stmt_for(ctx);
        } else if (!strcmp(hd->lx, KWD_BREAK)) {
                return (stmt *)parse_stmt_break(ctx);
        } else if (!strcmp(hd->lx, KWD_CONTINUE)) {
                return (stmt *)parse_stmt_continue(ctx);
        } else if (!strcmp(hd->lx, KWD_STRUCT)) {
                return (stmt *)parse_stmt_struct(ctx);
        } else if (!strcmp(hd->lx, KWD_MODULE)) {
                return (stmt *)parse_stmt_module(ctx);
        } else if (!strcmp(hd->lx, KWD_IMPORT)) {
                return (stmt *)parse_stmt_import(ctx);
        } else if (!strcmp(hd->lx, KWD_EMBED)) {
                return (stmt *)parse_stmt_embed(ctx);
        } else if (!strcmp(hd->lx, KWD_ENUM)) {
                return (stmt *)parse_stmt_enum(ctx);
        } else {
                assert(0);
        }

        assert(0 && "todo");
        return NULL; // unreachable
}

static stmt *
parse_stmt(parser_context *ctx)
{
        loc loc = lexer_peek(ctx->l, 0)->loc;
        stmt *s = NULL;

        if (kwds_iskw(lexer_peek(ctx->l, 0)->lx)) {
                s = parse_keyword_stmt(ctx);
                s->loc = loc;
                return s;
        } else if (LSP(ctx->l, 0)->ty == TOKEN_TYPE_LEFT_CURLY) {
                s = (stmt *)parse_stmt_block(ctx);
                s->loc = loc;
                return s;
        } else if (LSP(ctx->l, 0)->ty == TOKEN_TYPE_SEMICOLON) {
                lexer_discard(ctx->l); // ;
                return (stmt *)stmt_empty_alloc();
        }

        s = (stmt *)parse_stmt_expr(ctx);
        s->loc = loc;
        return s;
}

program *
parser_create_program(lexer *l)
{
        NOOP(parse_brace_initializer);

        parser_context ctx = (parser_context) {
                .l         = l,
                .in_global = 1,
                .module    = NULL,
        };

        program *p      = (program  *)alloc(sizeof(program));
        p->stmts        = dyn_array_empty(stmt_array);
        p->modname      = NULL;
        p->src_filepath = l->src_filepath;

        while (LSP(ctx.l, 0)->ty != TOKEN_TYPE_EOF) {
                dyn_array_append(p->stmts, parse_stmt(&ctx));
        }

        if (!ctx.module) {
                forge_err("a module name is required in each file");
        }

        p->modname = ctx.module;

        return p;
}
