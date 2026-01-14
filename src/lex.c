#include "lex.h"
#include "io.h"
#include "mem.h"
#include "loc.h"
#include "kwd.h"
#include "ds/strv.h"
#include "ds/map.h"

#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

MAP_TYPE(const char *, token_kind, opmap);

static const char *g_EOF   = "EOF";
static opmap       g_opmap = {0};

static unsigned
opmap_hash(const char **s)
{
        return **s;
}

static int
opmap_cmp(const char **s0,
          const char **s1)
{
        return strcmp(*s0, *s1);
}

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

static void
init_opmap(void) {
        g_opmap = opmap_create(opmap_hash, opmap_cmp);
        opmap_insert(&g_opmap, "(", TK_LPAREN);
        opmap_insert(&g_opmap, ")", TK_RPAREN);
        opmap_insert(&g_opmap, ";", TK_SEMI);
        opmap_insert(&g_opmap, "{", TK_LBRACKET);
        opmap_insert(&g_opmap, "}", TK_RBRACKET);
        opmap_insert(&g_opmap, "[", TK_LSQR);
        opmap_insert(&g_opmap, "]", TK_RSQR);
        opmap_insert(&g_opmap, "=", TK_EQ);
        opmap_insert(&g_opmap, "+", TK_PLUS);
        opmap_insert(&g_opmap, "-", TK_MINUS);
        opmap_insert(&g_opmap, "*", TK_ASTERISK);
        opmap_insert(&g_opmap, "/", TK_FORWARDSLASH);
        opmap_insert(&g_opmap, ":", TK_COLON);
}

static token_kind *
determineop(const char *s, size_t *len)
{
        assert(*len < 256);
        char buf[256] = {0};

        while (*len > 0) {
                memset(buf, 0, 256);
                strncpy(buf, s, *len);
                if (opmap_contains(&g_opmap, buf)) {
                        return opmap_get(&g_opmap, buf);
                }
                --(*len);
        }

        return NULL;
}

const char *
tk_to_cstr(token_kind k)
{
        switch (k) {
        case TK_EOF: return "EOF";
        case TK_KWD: return "keyword";
        case TK_ID: return "identifier";
        case TK_STRLIT: return "string literal";
        case TK_INTLIT: return "integer literal";
        case TK_LPAREN: return "(";
        case TK_RPAREN: return ")";
        case TK_SEMI: return ";";
        case TK_LBRACKET: return "{";
        case TK_RBRACKET: return "}";
        case TK_LSQR: return "[";
        case TK_RSQR: return "]";
        case TK_EQ: return "=";
        case TK_PLUS: return "+";
        case TK_MINUS: return "-";
        case TK_ASTERISK: return "*";
        case TK_FORWARDSLASH: return "/";
        case TK_COLON: return ":";
        default: assert(0);
        }

        // unreachable
        return NULL;
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

token *
lexer_peek(const lexer *l,
           size_t       p)
{
        token *it = l->hd;
        for (size_t i = 0; it && i < p; ++i)
                it = it->n;
        return it;
}

token *
lexer_next(lexer *l)
{
        token *t;

        if (!l->hd)
                return NULL;

        t     = l->hd;
        l->hd = l->hd->n;
        return t;
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
        l   = (lexer) {
                .hd  = NULL,
                .tl  = NULL,
                .fp  = path,
                .src = src,
                .a   = {0},
        };

        arena_init(&l.a, ARENA_DEFAULT_ALLOC_SIZE*sizeof(token));

        init_opmap();

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
                        size_t len = consume_while(src+i, isident);
                        token *t   = token_alloc(src+i, len, TK_ID,
                                                 r, c, l.fp, &l.a);
                        if (iskwd(strv_scstr(t->lx)))
                                t->k = TK_KWD;
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
                        size_t len = consume_while(src+i, issym);
                        token_kind *k = determineop(src+i, &len);
                        assert(k);
                        token *t = token_alloc(src+i, len, *k, r, c, l.fp, &l.a);
                        append(&l, t);
                        i += len;
                        c += len;
                }
        }

        append(&l, token_alloc(g_EOF, strlen(g_EOF), TK_EOF, r, c, l.fp, &l.a));

        return l;
}
