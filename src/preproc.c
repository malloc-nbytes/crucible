#include "preproc.h"
#include "lexer.h"
#include "kwds.h"
#include "ds/smap.h"

#include <forge/io.h>
#include <forge/err.h>

#include <ctype.h>
#include <string.h>
#include <assert.h>

static int
not_ignorable(int c)
{
        return !is_ignorable(c);
}

char *
preproc(const char *fp)
{
        lexer l = (lexer) {
                .hd = NULL,
                .tl = NULL,
                .src_filepath = fp,
        };

        char *src = forge_io_read_file_to_cstr(fp);
        size_t i = 0, c = 1, r = 1;

        while (src[i]) {
                char ch = src[i];
                if (ch == '_' || isalpha(ch)) {
                        size_t len = consume_while(src+i, is_ident);
                        token *t = token_alloc(src+i, len, TOKEN_TYPE_IDENTIFIER, r, c, fp);

                        if (!strcmp(t->lx, KWD_MACRO)) {
                                t->ty = TOKEN_TYPE_KEYWORD;
                                ch = src[i+len+1];
                                token *id = NULL;
                                size_t id_n = 0;
                                if (isalpha(ch) || ch == '_') {
                                        id_n = consume_while(src+i+len+1, is_ident);
                                        id = token_alloc(src+i+len+1, id_n, TOKEN_TYPE_IDENTIFIER, r, c, fp);
                                        lexer_append(&l, t);
                                } else {
                                        forge_err("expected identifier after `macro`");
                                }

                                lexer_append(&l, t);
                                lexer_append(&l, id);
                                i += len+1+id_n;
                                c += len+1+id_n;

                                token *hd = NULL, *tl = NULL;

#define APPEND(t)                                                       \
                                do {                                    \
                                        if (!hd && !tl) {               \
                                                hd = tl = t;            \
                                        } else {                        \
                                                token *tmp = tl;        \
                                                tl = t;                 \
                                                tmp->next = t;          \
                                        }                               \
                                } while (0)

                                while (src[i]) {
                                        if (isalpha(src[i])) {
                                                id_n = consume_while(src+i, is_ident);
                                                id = token_alloc(src+i, id_n, TOKEN_TYPE_IDENTIFIER, r, c, fp);
                                                if (!strcmp(id->lx, KWD_END)) {
                                                        break;
                                                } else {
                                                        i += id_n, c += id_n;
                                                        APPEND(id);
                                                }
                                        } else if (src[i] == ' ' || src[i] == '\t' || src[i] == '\r') {
                                                token *other = token_alloc(src+i, 1, TOKEN_TYPE_OTHER, r, c, fp);
                                                i += 1, c += 1;
                                                APPEND(other);
                                        } else if (src[i] == '\n') {
                                                token *other = token_alloc(src+i, 1, TOKEN_TYPE_OTHER, r, c, fp);
                                                i += 1, c = 1, r += 1;
                                                APPEND(other);
                                        } else {
                                                size_t ig_n = consume_while(src+i, not_ignorable);
                                                token *other = token_alloc(src+i, ig_n, TOKEN_TYPE_OTHER, r, c, fp);
                                                i += ig_n, c += ig_n;
                                                APPEND(other);
                                        }
                                }
#undef APPEND

                        } else {
                                i += len;
                                c += len;
                                lexer_append(&l, t);
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

        lexer_dump(&l);

        return NULL;
}
