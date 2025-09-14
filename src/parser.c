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
static expr *parse_expr(parser_context *ctx);

token *
expect(parser_context *ctx, token_type ty)
{
        token *t = lexer_next(ctx->l);
        if (t->ty != ty) {
                forge_err_wargs("%sexpected token of type `%s` but got `%s`",
                                tokerr(t), token_type_to_cstr(ty), t->lx);
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
        const token *hd = lexer_next(ctx->l);
        const char *lx = hd->lx;
        type *ty = NULL;

        if (!strcmp(lx, KWD_I8)) {
                forge_todo("i8");
        } else if (!strcmp(lx, KWD_I16)) {
                forge_todo("i16");
        } else if (!strcmp(lx, KWD_I32)) {
                return (type *)type_i32_alloc();
        } else if (!strcmp(lx, KWD_I64)) {
                forge_todo("i64");
        } else if (!strcmp(lx, KWD_U8)) {
                ty = (type *)type_u8_alloc();
        } else if (!strcmp(lx, KWD_U16)) {
                forge_todo("u16");
        } else if (!strcmp(lx, KWD_U32)) {
                ty = (type *)type_u32_alloc();
        } else if (!strcmp(lx, KWD_U64)) {
                forge_todo("u64");
        } else if (hd->ty == TOKEN_TYPE_BANG) {
                ty = (type *)type_noreturn_alloc();
        } else {
                forge_err_wargs("could not parse type at `%s`", lx);
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
                        left = (expr *)expr_identifier_alloc(i);
                } break;
                case TOKEN_TYPE_INTEGER_LITERAL: {
                        const token *i = lexer_next(ctx->l);
                        left = (expr *)expr_integer_literal_alloc(i);
                } break;
                case TOKEN_TYPE_STRING_LITERAL: {
                        const token *s = lexer_next(ctx->l);
                        left = (expr *)expr_string_literal_alloc(s);
                } break;
                case TOKEN_TYPE_LEFT_PARENTHESIS: {

                        if (left) {
                                // function call
                                expr_array args = parse_comma_sep_exprs(ctx);
                                left = (expr *)expr_proccall_alloc(left, args);
                        } else {
                                // Math expression
                                lexer_discard(ctx->l); // (
                                left = parse_expr(ctx);
                                (void)expect(ctx, TOKEN_TYPE_RIGHT_PARENTHESIS);
                        }
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
                lexer_discard(ctx->l); // void
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
        int export = !strcmp(lexer_peek(ctx->l, 0)->lx, KWD_EXPORT);
        if (export) {
                lexer_discard(ctx->l); // export
        }
        lexer_discard(ctx->l); // proc

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

        return stmt_proc_alloc(export, id, params, ty, blk);
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
parse_keyword_stmt(parser_context *ctx)
{
        token *hd = lexer_peek(ctx->l, 0);

        if (!strcmp(hd->lx, KWD_LET)) {
                return (stmt *)parse_stmt_let(ctx);
        } else if (!strcmp(hd->lx, KWD_PROC) || !strcmp(hd->lx, KWD_EXPORT)) {
                return (stmt *)parse_stmt_proc(ctx);
        } else if (!strcmp(hd->lx, KWD_RETURN)) {
                return (stmt *)parse_stmt_return(ctx);
        } else if (!strcmp(hd->lx, KWD_EXIT)) {
                return (stmt *)parse_stmt_exit(ctx);
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

static void dump_expr(const expr *e);
static void dump_stmt(const stmt *s);

static void
dump_expr_integer_literal(const expr_integer_literal *e)
{
        printf("\"integer\": \"%s\"\n", e->i->lx);
}

static void
dump_expr_binary(expr_bin *e)
{
        printf("\"binary\": {\n");
        printf("\"lhs\": {\n");
        dump_expr(e->lhs);
        printf("},");
        printf("\"op\": \"%s\",\n", e->op->lx);
        printf("\"rhs\": {\n");
        dump_expr(e->rhs);
        printf("}\n");
        printf("}\n");
}

static void
dump_expr_identifier(expr_identifier *e)
{
        printf("\"id\": \"%s\"\n", e->id->lx);
}

static void
dump_expr_string_literal(expr_string_literal *e)
{
        printf("\"string\": \"%s\"\n", e->s->lx);
}

static void
dump_expr_proccall(expr_proccall *e)
{
        printf("\"proccall\": {\n");
        printf("\"lhs\": {\n");
        dump_expr(e->lhs);
        printf("},\n");
        printf("\"args\": [\n");
        for (size_t i = 0; i < e->args.len; ++i) {
                if (i != 0) putchar(',');
                printf("{\n");
                dump_expr(e->args.data[i]);
                printf("}\n");
        }
        printf("]\n");
        printf("}\n");
}

static void
dump_expr(const expr *e)
{
        printf("\"expr\": {\n");
        switch (e->kind) {
        case EXPR_KIND_INTEGER_LITERAL: {
                dump_expr_integer_literal((expr_integer_literal *)e);
        } break;
        case EXPR_KIND_STRING_LITERAL: {
                dump_expr_string_literal((expr_string_literal *)e);
        } break;
        case EXPR_KIND_IDENTIFIER: {
                dump_expr_identifier((expr_identifier *)e);
        } break;
        case EXPR_KIND_BINARY: {
                dump_expr_binary((expr_bin *)e);
        } break;
        case EXPR_KIND_PROCCALL: {
                dump_expr_proccall((expr_proccall *)e);
        } break;
        default: forge_err_wargs("dump_expr(): unknown expression `%d`", (int)e->kind);
        }
        printf("}\n");
}

static void
dump_stmt_let(const stmt_let *s)
{
        printf("\"let\": {");
        printf("\"id\": \"%s\",\n", s->id->lx);
        printf("\"type\": \"%s\",\n", type_to_cstr(s->type));
        dump_expr(s->e);
        printf("}");
}

static void
dump_stmt_proc(stmt_proc *s)
{
        printf("\"proc\": {\n");
        printf("\"export\": \"%d\",", s->export);
        printf("\"type\": \"%s\",", type_to_cstr(s->type));
        printf("\"id\": \"%s\",\n", s->id->lx);
        printf("\"parameters\": [\n");

        for (size_t i = 0; i < s->params.len; ++i) {
                printf("{\n");
                printf("\"id\": \"%s\",\n", s->params.data[i].id->lx);
                printf("\"type\": \"%s\"\n", type_to_cstr(s->params.data[i].type));
                printf("}");
                if (i != s->params.len - 1) {
                        putchar(',');
                }
                printf("\n");
        }

        printf("],\n");
        printf("\"body\": ");
        dump_stmt(s->blk);
        printf("}\n");
}

static void
dump_stmt_block(stmt_block *s)
{
        printf("\"blk\": [\n");
        for (size_t i = 0; i < s->stmts.len; ++i) {
                if (i != 0) putchar(',');
                dump_stmt(s->stmts.data[i]);
        }
        printf("]\n");
}

static void
dump_stmt_return(stmt_return *s)
{
        printf("\"return\": {\n");
        dump_expr(s->e);
        printf("}\n");
}

static void
dump_stmt_expr(stmt_expr *s)
{
        dump_expr(s->e);
}

static void
dump_stmt_exit(stmt_expr *s)
{
        printf("\"exit\": {\n");
        if (s->e) {
                dump_expr(s->e);
        }
        printf("}\n");
}

static void
dump_stmt(const stmt *s)
{
        printf("{\n");
        switch (s->kind) {
        case STMT_KIND_LET: {
                dump_stmt_let((stmt_let *)s);
        } break;
        case STMT_KIND_PROC: {
                dump_stmt_proc((stmt_proc *)s);
        } break;
        case STMT_KIND_BLOCK: {
                dump_stmt_block((stmt_block *)s);
        } break;
        case STMT_KIND_RETURN: {
                dump_stmt_return((stmt_return *)s);
        } break;
        case STMT_KIND_EXPR: {
                dump_stmt_expr((stmt_expr *)s);
        } break;
        case STMT_KIND_EXIT: {
                dump_stmt_exit((stmt_expr *)s);
        } break;
        default:
                forge_err_wargs("dump_stmt(): unknown statement `%d`", (int)s->kind);
        }
        printf("}\n");
}

void
parser_dump_program(const program *p)
{
        printf("{\n\"program\": [\n");
        for (size_t i = 0; i < p->stmts.len; ++i) {
                if (i != 0) printf(",");
                dump_stmt(p->stmts.data[i]);
        }
        printf("]\n}");
}

