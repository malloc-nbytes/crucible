#include "lex.h"
#include "io.h"
#include "mem.h"
#include "loc.h"
#include "ds/strv.h"

#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char *g_EOF = "EOF";

static token *
token_alloc(const char *st,
            size_t      len,
            token_kind  k,
            size_t      r,
            size_t      c,
            const char *fp,
            arena      *a)
{
        token *t;

        t      = arena_alloc(a, sizeof(token));
        t->k   = k;
        t->n   = NULL;
        t->loc = loc_create(fp, r, c);
        t->lx  = strv_from(st, len);

        return t;
}


static inline int
ignorable(int ch)
{
        return ch == ' '
                || ch == '\t'
                || ch == '\n';
}

static size_t
consume_while(const char *st,
              int (*pred)(int))
{
        size_t i;
        if (!st) return 0;
        for (i = 0; st[i] && pred(st[i]); ++i);
        return i;
}

static void
append(lexer *l, token *t)
{
        if (!l->hd && !l->tl) {
                l->hd = l->tl = t;
        } else {
                token *tmp = l->tl;
                l->tl = t;
                tmp->n = t;
        }
}

static int
isident(int c)
{
        return isalnum(c) || c == '_';
}

static int
notquote(int c)
{
        return c != '"';
}

static int
notsquote(int c)
{
        return c != '\'';
}

static int
noteol(int c)
{
        return c != '\n';
}

static int
issym(int c)
{
        return !isident(c)
                && !ignorable(c)
                && c != '"'
                && c != '\'';
}

void
lexer_show(const lexer *l)
{
        token *it = l->hd;
        while (it) {
                printf("{ %d, %s, %zu, %zu, %s }\n",
                       it->k, strv_scstr(it->lx),
                       it->loc.r, it->loc.c, it->loc.fp);
                it = it->n;
        }
}

void
lexer_free(lexer *l)
{
        free(l->src);
        arena_free(&l->a);
}

lexer
lex_file(const char *path)
{
        char   *src;
        lexer   l;
        size_t  r;
        size_t  c;
        size_t  i;

        src = load_file(path);
        r   = 1;
        c   = 1;
        i   = 0;
        l = (lexer) {
                .hd = NULL,
                .tl = NULL,
                .fp = path,
                .src = src,
                .a  = {0},
        };

        arena_init(&l.a, ARENA_DEFAULT_ALLOC_SIZE*sizeof(token));

        while (src[i]) {
                char ch = src[i];

                if (ch == ' ' || ch == '\t') {
                        ++c;
                        ++i;
                } else if (ch == '\n') {
                        c = 1;
                        ++r;
                        ++i;
                } else if (isident(ch) && !isdigit(ch)) {
                        size_t len = consume_while(src+i, issym);
                        token *t   = token_alloc(src+i, len, TK_ID,
                                                 r, c, l.fp, &l.a);
                        append(&l, t);
                        i += len;
                        c += len;
                } else if (isdigit(ch)) {
                        size_t len = consume_while(src+i, isdigit);
                        token *t   = token_alloc(src+i, len, TK_INTLIT,
                                                 r, c, l.fp, &l.a);
                        append(&l, t);
                        i += len;
                        c += len;
                } else if (ch == '"') {
                        size_t len = consume_while(src+i+1, notquote);
                        token *t   = token_alloc(src+i+1, len, TK_STRLIT,
                                                 r, c, l.fp, &l.a);
                        append(&l, t);
                        i += len+2;
                        c += len+2;
                } else if (ch == '\'') {
                        assert(0);
                } else {
                        assert(0);
                }
        }

        append(&l, token_alloc(g_EOF, strlen(g_EOF), TK_EOF, r, c, l.fp, &l.a));

        return l;
}
