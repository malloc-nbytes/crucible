#include "lex.h"
#include "io.h"
#include "mem.h"
#include "loc.h"
#include "ds/strv.h"

#include <ctype.h>
#include <assert.h>

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
        t->lx  = strv_from(st, len);
        t->k   = k;
        t->n   = NULL;
        t->loc = loc_create(fp, r, c);

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

lexer
lex_file(const char *path)
{
        char   *src;
        lexer   l;
        size_t  r;
        size_t  c;
        size_t  i;

        l = (lexer) {
                .hd = NULL,
                .tl = NULL,
                .fp = path,
                .a  = {0},
        };

        arena_init(&l.a, 256);

        src = load_file(path);
        r   = 1;
        c   = 1;
        i   = 0;

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
                        i += len;
                        c += len;
                } else if (isdigit(ch)) {
                        size_t len = consume_while(src+i, isdigit);
                        token *t   = token_alloc(src+i, len, TK_INTLIT,
                                                 r, c, l.fp, &l.a);
                        i += len;
                        c += len;
                } else if (ch == '"') {
                        size_t len = consume_while(src+i+1, notquote);
                        token *t   = token_alloc(src+i+1, len, TK_STRLIT,
                                                 r, c, l.fp, &l.a);
                        i += len+2;
                        c += len+2;
                } else if (ch == '\'') {
                        assert(0);
                } else {
                        assert(0);
                }
        }

        return l;
}
