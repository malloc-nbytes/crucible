#include "parser.hpp"
#include "keyword.hpp"

#include <cassert>
#include <iostream>
#include <exception>
#include <format>

struct parser_error : public std::exception {};

struct expect_error : parser_error {
        token_kind              exp;
        const token *const      got;

        expect_error(token_kind         exp,
                     const token *const got)
                : exp(exp), got(got) {}

        const char *
        what(void) const noexcept override
        {
                return format("{}: expected `{}' but got `{}'",
                              location_to_string(got->loc),
                              token_kind_to_cstring(exp),
                              token_kind_to_cstring(got->k)).c_str();
        }
};

struct expect_keyword_error : parser_error {
        std::string kw;
        const token *const got;

        expect_keyword_error(std::string        kw,
                             const token *const got)
                : kw(kw), got(got) {}

        const char *
        what(void) const noexcept override
        {
                return format("%s: expected keyword `{}' but got `{}'",
                              location_to_string(got->loc),
                              kw, token_kind_to_cstring(got->k)).c_str();
        }
};

struct out_of_tokens_error : public parser_error {
        out_of_tokens_error() = default;

        const char *
        what(void) const noexcept override
        {
                return "ran out of tokens";
        }
};

struct illegal_keyword_placement : public parser_error {
        const token *const kw;

        illegal_keyword_placement(const token *const kw)
                : kw(kw) {}

        const char *
        what(void) const noexcept override
        {
                return format("%s: illegal keyword `%s'",
                              location_to_string(kw->loc),
                              kw->lx).c_str();
        }
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
parse_expr(parser *p)
{
        assert(0);
}

static type *
parse_type(parser *p)
{
        assert(0);
}

static stmt_expr *
parse_stmt_expr(parser *p)
{
        expr *e;

        e = parse_expr(p);

        return stmt_expr_alloc(e);
}

static stmt_proc *
parse_stmt_proc(parser *p)
{
        assert(0);
}

static stmt_let *
parse_stmt_let(parser *p)
{
        location         loc;
        const token     *id;
        type            *ty;
        expr            *e;

        loc = expectkw(p, KEYWORD_LET)->loc;
        id = expect(p, TOKEN_KIND_IDENTIFIER);
        expect(p, TOKEN_KIND_COLON);
        ty = parse_type(p);
        expect(p, TOKEN_KIND_EQUALS);
        e = parse_expr(p);

        return stmt_let_alloc(loc, id, ty, e);
}

static stmt *
parse_stmt_keyword(parser *p)
{
        const token *hd = peek(p);

        if (hd->lx == KEYWORD_PROC)
                return (stmt *)parse_stmt_proc(p);
        if (hd->lx == KEYWORD_LET)
                return (stmt *)parse_stmt_let(p);
        throw illegal_keyword_placement(hd);
}

static stmt *
parse_stmt(parser *p)
{
        const token *hd = peek(p);
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
