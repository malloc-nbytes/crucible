#include "lexer.h"
#include "token.h"
#include "kw.h"
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

        lexer_opmap_insert(&g_opmap, ")", TK_LPAREN);
        lexer_opmap_insert(&g_opmap, "(", TK_RPAREN);
        lexer_opmap_insert(&g_opmap, "{", TK_LCURLY);
        lexer_opmap_insert(&g_opmap, "}", TK_RCURLY);
        lexer_opmap_insert(&g_opmap, "[", TK_LSQR);
        lexer_opmap_insert(&g_opmap, "]", TK_RSQR);
        lexer_opmap_insert(&g_opmap, ":", TK_COLON);
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
                        assert(0);
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
                        assert(0);
                }
        }

        return l;
}
#undef TK_PATH_CONS
