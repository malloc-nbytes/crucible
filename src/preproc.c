#include "preproc.h"
#include "lexer.h"
#include "kwds.h"
#include "ds/smap.h"

#include <forge/io.h>
#include <forge/err.h>

#include <ctype.h>
#include <string.h>
#include <assert.h>

smap g_macros = {0};

static int
not_ignorable(int c)
{
        return !is_ignorable(c);
}

static char *
lexer_as_cstr(lexer  *l,
              char   *src,
              size_t  fp_n)
{
        size_t cap = fp_n;
        size_t len = 0;

        while (l->hd) {
                const char *ln = l->hd->lx;
                for (size_t i = 0; ln[i]; ++i) {
                        if (len >= cap) {
                                cap *= 2;
                                src = (char *)realloc((void *)src, cap);
                        }
                        src[len++] = ln[i];
                }
                l->hd = l->hd->next;
        }

        memset(src+len, 0, cap);

        return src;
}

#define APPEND(t)                               \
        do {                                    \
                if (!hd && !tl) {               \
                        hd = tl = t;            \
                } else {                        \
                        token *tmp = tl;        \
                        tl = t;                 \
                        tmp->next = t;          \
                }                               \
        } while (0)
size_t
handle_macro(char       *src,
             size_t     *c,
             size_t     *r,
             const char *fp)
{
        assert(*src == ' ');
        ++src;

        size_t macro_id_n = consume_while(src, is_ident);
        char *macro_id = strndup(src, macro_id_n);
        src += macro_id_n;

        token *hd = NULL, *tl = NULL;
        size_t i = macro_id_n;

        while (src[i]) {
                if (isalpha(src[i])) {
                        size_t alpha_n = consume_while(src+i, is_ident);
                        token *alpha = token_alloc(src+i, alpha_n, TOKEN_TYPE_IDENTIFIER, *r, *c, fp);
                        if (!strcmp(alpha->lx, KWD_END)) {
                                smap_insert(&g_macros, macro_id, hd);
                                i += alpha_n+2, c += alpha_n+2;
                                break;
                        } else {
                                i += alpha_n, c += alpha_n;
                                APPEND(alpha);
                        }
                } else if (src[i] == ' ' || src[i] == '\t' || src[i] == '\r') {
                        token *other = token_alloc(src+i, 1, TOKEN_TYPE_OTHER, *r, *c, fp);
                        i += 1, *c += 1;
                        APPEND(other);
                } else if (src[i] == '\n') {
                        token *other = token_alloc(src+i, 1, TOKEN_TYPE_OTHER, *r, *c, fp);
                        i += 1, *c = 1, *r += 1;
                        APPEND(other);
                } else {
                        size_t ig_n = consume_while(src+i, not_ignorable);
                        token *other = token_alloc(src+i, ig_n, TOKEN_TYPE_OTHER, *r, *c, fp);
                        i += ig_n, *c += ig_n;
                        APPEND(other);
                }
        }

        return i;
}
#undef APPEND

char *
preproc(const char *fp)
{
        lexer l = (lexer) {
                .hd = NULL,
                .tl = NULL,
                .src_filepath = fp,
        };

        g_macros = smap_create(NULL);

        char *src = forge_io_read_file_to_cstr(fp);
        size_t i = 0, c = 1, r = 1;

        while (src[i]) {
                char ch = src[i];
                if (ch == '_' || isalpha(ch)) {
                        size_t len = consume_while(src+i, is_ident);
                        token *t = token_alloc(src+i, len, TOKEN_TYPE_IDENTIFIER, r, c, fp);
                        if (!strcmp(t->lx, KWD_MACRO)) {
                                i += len + handle_macro(src+i+len, &c, &r, fp);
                        } else if (smap_has(&g_macros, t->lx)) {
                                token *body = (token *)smap_get(&g_macros, t->lx);
                                while (body) {
                                        lexer_append(&l, body);
                                        body = body->next;
                                }
                                i += len;
                                c += len;
                        } else if (!strcmp(KWD_END, t->lx)) {
                                i += len;
                                c += len;
                        } else {
                                lexer_append(&l, t);
                                i += len;
                                c += len;
                        }
                } else if (ch == '(') {
                        token *t = token_alloc(src+i, 1, TOKEN_TYPE_LEFT_PARENTHESIS, r, c, fp);
                        lexer_append(&l, t);
                        i += 1, c += 1;
                } else if (ch == ')') {
                        token *t = token_alloc(src+i, 1, TOKEN_TYPE_RIGHT_PARENTHESIS, r, c, fp);
                        lexer_append(&l, t);
                        i += 1, c += 1;
                } else if (ch == ' ') {
                        token *t = token_alloc(src+i, 1, TOKEN_TYPE_WHITESPACE, r, c, fp);
                        lexer_append(&l, t);
                        i += 1, c += 1;
                } else if (ch == '\n') {
                        token *t = token_alloc(src+i, 1, TOKEN_TYPE_NEWLINE, r, c, fp);
                        lexer_append(&l, t);
                        i += 1, c = 0, r += 1;
                } else if (ch == '\t') {
                        token *t = token_alloc(src+i, 1, TOKEN_TYPE_TAB, r, c, fp);
                        lexer_append(&l, t);
                        i += 1, c += 1;
                } else if (ch == '\r') {
                        token *t = token_alloc(src+i, 1, TOKEN_TYPE_TAB, r, c, fp);
                        lexer_append(&l, t);
                        i += 1, c += 1;
                } else {
                        size_t len = consume_while(src+i, not_ignorable);
                        token *t = token_alloc(src+i, len, TOKEN_TYPE_OTHER, r, c, fp);
                        lexer_append(&l, t);
                        i += len, c += len;
                }
        }

        free(src);

        //lexer_dump(&l);

        char *res = lexer_as_cstr(&l, src, i);
        printf("%s\n", res);
        return res;
}
