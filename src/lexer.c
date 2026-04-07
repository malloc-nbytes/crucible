#include "lexer.h"
#include "token.h"
#include "kw.h"
#include "err.h"
#include "ds/map.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

MAP_DEFINE(const char *, token_kind, lexer_opmap);
MAP_IMPL  (const char *, token_kind, lexer_opmap);

static lexer_opmap g_opmap = {0};

static unsigned
lexer_opmap_hash(const char **s)
{
        return **s;
}

static int
lexer_opmap_cmp(const char **s0,
                const char **s1)
{
        return strcmp(*s0, *s1);
}

void
init_lexer_translation_unit(void)
{
        g_opmap = lexer_opmap_create(lexer_opmap_hash,
                                     lexer_opmap_cmp);

        lexer_opmap_insert(&g_opmap, "(", TK_LPAREN);
        lexer_opmap_insert(&g_opmap, ")", TK_RPAREN);
        lexer_opmap_insert(&g_opmap, "{", TK_LCURLY);
        lexer_opmap_insert(&g_opmap, "}", TK_RCURLY);
        lexer_opmap_insert(&g_opmap, "[", TK_LSQR);
        lexer_opmap_insert(&g_opmap, "]", TK_RSQR);
        lexer_opmap_insert(&g_opmap, ":", TK_COLON);
        lexer_opmap_insert(&g_opmap, "!", TK_BANG);
        lexer_opmap_insert(&g_opmap, ";", TK_SEMI);
        lexer_opmap_insert(&g_opmap, "+", TK_PLUS);
        lexer_opmap_insert(&g_opmap, "-", TK_MINUS);
        lexer_opmap_insert(&g_opmap, "/", TK_FORWARDSLASH);
        lexer_opmap_insert(&g_opmap, "%", TK_PERCENT);
        lexer_opmap_insert(&g_opmap, "*", TK_ASTERISK);
        lexer_opmap_insert(&g_opmap, "=", TK_EQUALS);
        lexer_opmap_insert(&g_opmap, "+=", TK_PLUS_EQUALS);
        lexer_opmap_insert(&g_opmap, "-=", TK_MINUS_EQUALS);
        lexer_opmap_insert(&g_opmap, "/=", TK_FORWARDSLASH_EQUALS);
        lexer_opmap_insert(&g_opmap, "*=", TK_ASTERISK_EQUALS);
        lexer_opmap_insert(&g_opmap, "&", TK_AMPERSAND);
        lexer_opmap_insert(&g_opmap, "|", TK_PIPE);
        lexer_opmap_insert(&g_opmap, "&&", TK_DOUBLE_AMPERSAND);
        lexer_opmap_insert(&g_opmap, "||", TK_DOUBLE_PIPE);
        lexer_opmap_insert(&g_opmap, "&=", TK_AMPERSAND_EQUALS);
        lexer_opmap_insert(&g_opmap, "|=", TK_PIPE_EQUALS);
        lexer_opmap_insert(&g_opmap, "^", TK_UPTICK);
        lexer_opmap_insert(&g_opmap, "^=", TK_UPTICK_EQUALS);
        lexer_opmap_insert(&g_opmap, ">", TK_GREATERTHAN);
        lexer_opmap_insert(&g_opmap, "<", TK_LESSTHAN);
        lexer_opmap_insert(&g_opmap, ">=", TK_GREATERTHAN_EQUALS);
        lexer_opmap_insert(&g_opmap, "<=", TK_LESSTHAN_EQUALS);
}

static size_t
consume_while(const char *s,
              int       (*predicate)(int))
{
        size_t i;

        for (i = 0; s[i] && predicate(s[i]); ++i);

        return i;
}

static int
isident(int c)
{
        return c == '_' || isalnum(c);
}

static int
isop(int c)
{
        return !isspace(c) && !isident(c) && c != '\'' && c != '"';
}

static int
notquote(int c)
{
        return c != '"';
}

#define BUF_CAP 256
static token_kind *
determine_op(const char *s, size_t *n)
{
        assert(*n < BUF_CAP);
        char buf[BUF_CAP];

        while (*n > 0) {
                memset(buf, 0, sizeof(buf));
                strncpy(buf, s, *n);
                if (lexer_opmap_contains(&g_opmap, buf))
                        return lexer_opmap_get(&g_opmap, buf);
                --(*n);
        }

        return NULL;
}
#undef BUF_CAP

void
lexer_dump(const lexer *l)
{
        for (size_t i = 0; i < l->tokens.len; ++i) {
                const token *t = l->tokens.data[i];
                printf("{ lx=%s; k=%s; loc=%s }\n", sv_cstr(t->lx),
                                                    token_kind_cstr(t->kind),
                                                    loc_cstr(t->loc));
        }
}

#define TK_PATH_CONS r, c, path
lexer
lexer_lex_file(const char *path,
               const char *src)
{

        lexer  l;
        size_t r;
        size_t c;
        size_t i;

        l = (lexer) {
                .pos    = 0,
                .tokens = array_empty(tokenp_ar),
                .src    = sv_from(src, -1),
                .path   = sv_from(path, -1),
        };
        r = 1;
        c = 1;
        i = 0;

        while (i < l.src.len) {
                char ch = src[i];

                if (ch == '-' && src[i+1] && src[i+1] == '-') {
                        assert(0);
                } else if (ch == '\n') {
                        ++i;
                        ++r;
                        c = 1;
                } else if (ch == '\t') {
                        ++i;
                        ++c;
                } else if (ch == ' ') {
                        ++i;
                        ++c;
                } else if (ch == '\'') {
                        assert(0);
                } else if (ch == '"') {
                        size_t n = consume_while(src+i+1, notquote);
                        array_append(l.tokens, token_from(TK_STRLIT, src+i+1, n, TK_PATH_CONS));
                        i += n + 2;
                        c += n + 2;
                } else if (isalpha(ch) || ch == '_') {
                        size_t n = consume_while(src+i, isident);
                        token *t = token_from(TK_IDENT, src+i, n, TK_PATH_CONS);
                        if (kw_iskw(sv_cstr(t->lx)))
                                t->kind = TK_KW;
                        array_append(l.tokens, t);
                        i += n;
                        c += n;
                } else if (isdigit(ch)) {
                        size_t n = consume_while(src+i, isdigit);
                        array_append(l.tokens, token_from(TK_INTLIT, src+i, n, TK_PATH_CONS));
                        i += n;
                        c += n;
                } else {
                        size_t n      = consume_while(src+i, isop);
                        token_kind *k = determine_op(src+i, &n);
                        if (!k)
                                fatal("unknown symbol starting at: %s", src+i);
                        array_append(l.tokens, token_from(*k, src+i, n, TK_PATH_CONS));
                        i += n;
                        c += n;
                }
        }

        static const char *eof = "EOF";
        array_append(l.tokens, token_from(TK_EOF, eof, 3, TK_PATH_CONS));

        return l;
}
#undef TK_PATH_CONS
