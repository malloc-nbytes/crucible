#include "parser.hpp"

#include <iostream>
#include <exception>
#include <format>

struct parser_error : public std::exception {};

struct expect_error : parser_error {
        token_kind       exp;
        token           *got;

        expect_error(token_kind exp, token *got)
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

struct out_of_tokens_error : public parser_error {
        out_of_tokens_error() = default;

        const char *
        what(void) const noexcept override
        {
                return "ran out of tokens";
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

static stmt_expr *
parse_stmt_expr(parser *p)
{
}

static stmt *
parse_stmt_keyword(parser *p)
{
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
