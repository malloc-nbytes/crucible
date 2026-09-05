#include "parser.hpp"
#include "keyword.hpp"

#include <cassert>
#include <exception>
#include <format>
#include <iostream>
#include <string>
#include <utility>

static stmt *
parse_stmt(parser *p);

static stmt_block *
parse_stmt_block(parser *p);

struct parser_error : public std::exception {
        std::string message;

        parser_error(std::string message)
                : message(std::move(message)) {}

        const char *
        what(void) const noexcept override
        {
                return message.c_str();
        }
};

struct expect_error : parser_error {
        token_kind              exp;
        const token *const      got;

        expect_error(token_kind         exp,
                     const token *const got)
                : parser_error(std::format("{}: expected `{}' but got `{}'",
                                            location_to_string(got->loc),
                                            token_kind_to_cstring(exp),
                                            token_kind_to_cstring(got->k))),
                  exp(exp), got(got) {}
};

struct expected_expression_error : parser_error {
        const token *const got;

        expected_expression_error(const token *const got)
                : parser_error(std::format("{}: expected expression but got `{}'",
                                            location_to_string(got->loc),
                                            token_kind_to_cstring(got->k))),
                  got(got) {}
};

struct zero_param_proc_error : parser_error {
        location loc;

        zero_param_proc_error(location loc)
                : parser_error(std::format("{}: procedures with no parameters need to be `void'",
                                            location_to_string(loc))),
                  loc(std::move(loc)) {}
};

struct expect_keyword_error : parser_error {
        std::string kw;
        const token *const got;

        expect_keyword_error(std::string        kw,
                             const token *const got)
                : parser_error(std::format("{}: expected keyword `{}' but got `{}'",
                                            location_to_string(got->loc),
                                            kw, token_kind_to_cstring(got->k))),
                  kw(std::move(kw)), got(got) {}
};

struct out_of_tokens_error : public parser_error {
        out_of_tokens_error()
                : parser_error("ran out of tokens") {}
};

struct illegal_keyword_placement : public parser_error {
        const token *const kw;

        illegal_keyword_placement(const token *const kw)
                : parser_error(std::format("{}: illegal keyword `{}'",
                                            location_to_string(kw->loc),
                                            kw->lx)),
                  kw(kw) {}
};

static void
cursor_check(const parser *p)
{
        if (p->l->cursor >= p->l->ts.size())
                throw out_of_tokens_error();
}

static token *
next(parser *p)
{
        cursor_check(p);
        return p->l->ts[p->l->cursor++];
}

static void
discard(parser *p)
{
        cursor_check(p);
        ++p->l->cursor;
}

static const token *
peek(parser *p)
{
        cursor_check(p);
        return p->l->ts[p->l->cursor];
}

static token *
expect(parser *p, token_kind k)
{
        token *hd = next(p);

        if (hd->k != k)
                throw expect_error(k, hd);

        return hd;
}

static token *
expectkw(parser *p, const char *kw)
{
        token *hd = expect(p, TOKEN_KIND_KEYWORD);
        if (hd->lx != kw)
                throw expect_keyword_error(kw, hd);
        return hd;
}

static expr *
parse_primary_expr(parser *p)
{
        const token *hd = peek(p);

        switch (hd->k) {
        case TOKEN_KIND_INTEGER_LITERAL:
                discard(p);
                return (expr *)expr_int_alloc(hd);
        case TOKEN_KIND_IDENTIFIER:
                discard(p);
                return (expr *)expr_identifier_alloc(hd);
        default:
                throw expected_expression_error(hd);
        }
}

static expr *
parse_unary_expr(parser *p)
{
        return parse_primary_expr(p);
}

static expr *
parse_multiplicative_expr(parser *p)
{
        expr        *lhs;
        const token *cur;

        lhs = parse_unary_expr(p);
        cur = peek(p);

        while (cur && (cur->k == TOKEN_KIND_ASTERISK
                        || cur->k == TOKEN_KIND_FORWARD_SLASH)) {
                token *op        = next(p);
                expr *rhs        = parse_unary_expr(p);
                expr_binary *bin = expr_binary_alloc(lhs, op, rhs);
                bin->base.loc    = lhs->loc;
                lhs              = (expr *)bin;
                cur              = peek(p);
        }

        return lhs;
}

static expr *
parse_additive_expr(parser *p)
{
        expr        *lhs;
        const token *cur;

        lhs = parse_multiplicative_expr(p);
        cur = peek(p);

        while (cur && (cur->k == TOKEN_KIND_PLUS
                        || cur->k == TOKEN_KIND_HYPHEN)) {
                token *op        = next(p);
                expr *rhs        = parse_multiplicative_expr(p);
                expr_binary *bin = expr_binary_alloc(lhs, op, rhs);
                bin->base.loc    = lhs->loc;
                lhs              = (expr *)bin;
                cur              = peek(p);
        }

        return lhs;
}

static expr *
parse_equalitive_expr(parser *p)
{
        expr        *lhs;
        const token *cur;

        lhs = parse_additive_expr(p);
        cur = peek(p);

        while (cur && (cur->k == TOKEN_KIND_DOUBLE_EQUALS
                        || cur->k == TOKEN_KIND_BANG_EQUALS
                        || cur->k == TOKEN_KIND_GREATERTHAN_EQUALS
                        || cur->k == TOKEN_KIND_LESSTHAN_EQUALS
                        || cur->k == TOKEN_KIND_LESSTHAN
                        || cur->k == TOKEN_KIND_GREATERTHAN)) {
                token *op        = next(p);
                expr *rhs        = parse_additive_expr(p);
                expr_binary *bin = expr_binary_alloc(lhs, op, rhs);
                bin->base.loc    = lhs->loc;
                lhs              = (expr *)bin;
                cur              = peek(p);
        }

        return lhs;
}

static expr *
parse_logical_expr(parser *p)
{
        expr        *lhs;
        const token *cur;

        lhs = parse_equalitive_expr(p);
        cur = peek(p);

        while (cur && (cur->k == TOKEN_KIND_DOUBLE_PIPE
                        || cur->k == TOKEN_KIND_DOUBLE_AMPERSAND)) {
                token *op        = next(p);
                expr *rhs        = parse_equalitive_expr(p);
                expr_binary *bin = expr_binary_alloc(lhs, op, rhs);
                bin->base.loc    = lhs->loc;
                lhs              = (expr *)bin;
                cur              = peek(p);
        }

        return lhs;
}

static expr *
parse_assignment_expr(parser *p)
{
        expr *lhs;
        const token *cur;

        lhs = parse_logical_expr(p);

        cur = peek(p);
        if (cur->k == TOKEN_KIND_EQUALS) {
                token   *op  = next(p);
                expr    *rhs = parse_assignment_expr(p);
                return (expr *)expr_binary_alloc(lhs, op, rhs);
        }

        return lhs;
}

static expr *
parse_expr(parser *p)
{
        return parse_assignment_expr(p);
}

static type *
parse_type(parser *p)
{
        token   *hd;
        type    *base;

        hd   = expect(p, TOKEN_KIND_TYPE);
        base = NULL;

        if (hd->lx == TYPE_VOID)
                base = (type *)type_void_alloc();
        else if (hd->lx == TYPE_I32)
                base = (type *)type_i32_alloc();
        else if (hd->lx == TYPE_U8)
                base = (type *)type_u8_alloc();
        else
                std::unreachable();

        while (peek(p)->k == TOKEN_KIND_ASTERISK) {
                discard(p);
                base = (type *)type_ptr_alloc(base);
        }

        return base;
}

static stmt_expr *
parse_stmt_expr(parser *p)
{
        expr *e;

        e = parse_expr(p);

        expect(p, TOKEN_KIND_SEMICOLON);

        return stmt_expr_alloc(e);
}

static stmt_block *
parse_stmt_block(parser *p)
{
        location                loc;
        std::vector<stmt *>     stmts;

        loc   = expect(p, TOKEN_KIND_L_CURLY)->loc;
        stmts = std::vector<stmt *>();

        while (peek(p)->k != TOKEN_KIND_R_CURLY
               && peek(p)->k != TOKEN_KIND_EOF)
                stmts.push_back(parse_stmt(p));

        expect(p, TOKEN_KIND_R_CURLY);

        return stmt_block_alloc(loc, stmts);
}

static std::vector<parameter>
parse_proc_params(parser *p, uint32_t *bits)
{
        std::vector<parameter>  params;
        location                loc;

        params = std::vector<parameter>();
        loc    = expect(p, TOKEN_KIND_L_PAREN)->loc;

        if (peek(p)->k == TOKEN_KIND_TYPE && peek(p)->lx == TYPE_VOID) {
                discard(p);
                expect(p, TOKEN_KIND_R_PAREN);
                return params;
        }

        if (peek(p)->k == TOKEN_KIND_R_PAREN)
                throw zero_param_proc_error(loc);

        while (peek(p)->k != TOKEN_KIND_R_PAREN) {
                token   *id;
                type    *ty;

                id = expect(p, TOKEN_KIND_IDENTIFIER);

                expect(p, TOKEN_KIND_COLON);

                ty = parse_type(p);

                params.push_back(parameter { .id = id, .ty = ty, .sym = NULL });

                if (peek(p)->k == TOKEN_KIND_ELIPSIS) {
                        discard(p);
                        *bits |= PROC_VARIADIC;
                        break;
                }

                if (peek(p)->k == TOKEN_KIND_COMMA)
                        discard(p);
                else
                        break;
        }

        expect(p, TOKEN_KIND_R_PAREN);

        return params;
}

static stmt_proc *
parse_stmt_proc(parser *p)
{
        location                         loc;
        uint32_t                         bits;
        token                           *id;
        std::vector<parameter>           params;
        type                            *rty;
        std::optional<stmt_block *>      body;

        bits = 0x0000;

        if (peek(p)->lx == KEYWORD_EXPORT) {
                loc = next(p)->loc;
                bits |= PROC_LINKAGE_EXPORT;
        }

        if (peek(p)->lx == KEYWORD_EXTERN) {
                token *extern_ = next(p);
                if (!bits)
                        loc = extern_->loc;
                bits |= PROC_LINKAGE_EXTERN;
        }

        {location proc_loc = expectkw(p, KEYWORD_PROC)->loc;
                if (!bits) {
                        loc = proc_loc;
                        if (!bits)
                                bits |= PROC_LINKAGE_INTERNAL;
                }
        }

        id     = expect(p, TOKEN_KIND_IDENTIFIER);
        params = parse_proc_params(p, &bits);

        expect(p, TOKEN_KIND_COLON);

        rty = parse_type(p);

        if (bits & PROC_LINKAGE_EXTERN) {
                expect(p, TOKEN_KIND_SEMICOLON);
                body = {};
        }
        else
                body = parse_stmt_block(p);

        return stmt_proc_alloc(loc, bits, id, params, rty, body);
}

static stmt_let *
parse_stmt_let(parser *p)
{
        location         loc;
        const token     *id;
        type            *ty;
        expr            *e;

        loc = expectkw(p, KEYWORD_LET)->loc;
        id  = expect(p, TOKEN_KIND_IDENTIFIER);

        expect(p, TOKEN_KIND_COLON);

        ty = parse_type(p);

        expect(p, TOKEN_KIND_EQUALS);

        e = parse_expr(p);

        expect(p, TOKEN_KIND_SEMICOLON);

        return stmt_let_alloc(loc, id, ty, e);
}

static stmt *
parse_stmt_keyword(parser *p)
{
        const token *hd;

        hd = peek(p);

        if (hd->lx == KEYWORD_PROC
            || hd->lx == KEYWORD_EXTERN
            || hd->lx == KEYWORD_EXPORT)
                return (stmt *)parse_stmt_proc(p);
        if (hd->lx == KEYWORD_LET)
                return (stmt *)parse_stmt_let(p);

        throw illegal_keyword_placement(hd);
}

static stmt *
parse_stmt(parser *p)
{
        const token *hd = peek(p);
        if (hd->k == TOKEN_KIND_L_CURLY)
                return (stmt *)parse_stmt_block(p);
        if (hd->k == TOKEN_KIND_KEYWORD)
                return parse_stmt_keyword(p);
        return (stmt *)parse_stmt_expr(p);
}

parser
parse(lexer *l)
{
        parser p = parser {
                .l      = l,
                .global = 0,
                .stmts = std::vector<stmt *>(),
        };

        try {
                while (peek(&p)->k != TOKEN_KIND_EOF)
                        p.stmts.push_back(parse_stmt(&p));
        } catch (const parser_error &e) {
                std::fprintf(stderr, "%s\n", e.what());
                std::exit(1);
        }

        return p;
}
