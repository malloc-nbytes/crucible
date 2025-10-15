#include "preproc.h"
#include "lexer.h"
#include "kwds.h"
#include "ds/smap.h"

#include <forge/io.h>
#include <forge/err.h>

#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

typedef struct {
        const char   *id;
        token_array   params;
        //token_array   body;
        token *hd, *tl;
} macro_content;

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

#define APPEND(c)                                                       \
        do {                                                            \
                if (len >= cap) {                                       \
                        cap *= 2;                                       \
                        src = (char *)realloc((void *)src, cap);        \
                }                                                       \
                src[len++] = c;                                         \
        } while (0)

        while (l->hd && l->hd->ty != TOKEN_TYPE_EOF) {
                const token *tok = l->hd;
                if (tok->ty == TOKEN_TYPE_STRING_LITERAL)    APPEND('"');
                if (tok->ty == TOKEN_TYPE_CHARACTER_LITERAL) APPEND('\'');
                for (size_t i = 0; tok->lx[i]; ++i) {
                        APPEND(tok->lx[i]);
                }
                if (tok->ty == TOKEN_TYPE_STRING_LITERAL)    APPEND('"');
                if (tok->ty == TOKEN_TYPE_CHARACTER_LITERAL) APPEND('\'');
                l->hd = l->hd->next;
        }

        APPEND('\0');
#undef APPEND

        return src;
}

static void
free_macro_content(void *mc)
{
        macro_content *macro = (macro_content *)mc;
        free((void *)macro->id);
        for (size_t i = 0; i < macro->params.len; ++i) {
                token *t = macro->params.data[i];
                free(t->lx);
                free(t);
        }
        dyn_array_free(macro->params);
        /* for (size_t i = 0; i < macro->body.len; ++i) { */
        /*         token *t = macro->body.data[i]; */
        /*         free(t->lx); */
        /*         free(t); */
        /* } */
        //dyn_array_free(macro->body);
        free(macro);
}

static void
process_macro_definition(lexer *l, token **it, macro_content *mc)
{
        mc->hd     = NULL;
        mc->tl     = NULL;

#define APPEND(t) \
        do { \
                if (!mc->hd && !mc->tl) { \
                        mc->hd = mc->tl = t; \
                } else { \
                        token *tmp = mc->tl; \
                        mc->tl = t; \
                        tmp->next = t; \
                } \
        } while (0)

        *it = (*it)->next; // macro

        // Consume whitespace
        while ((*it)->ty == TOKEN_TYPE_WHITESPACE || (*it)->ty == TOKEN_TYPE_TAB || (*it)->ty == TOKEN_TYPE_NEWLINE) {
                *it = (*it)->next; // ' '
        }

        token *id = *it;
        *it = (*it)->next; // id
        token_array params = dyn_array_empty(token_array);
        token_array body = dyn_array_empty(token_array);

        if ((*it)->ty == TOKEN_TYPE_LEFT_PARENTHESIS) {
                *it = (*it)->next; // (
                while ((*it)->ty != TOKEN_TYPE_RIGHT_PARENTHESIS) {
                        dyn_array_append(params, *it);
                        *it = (*it)->next; // id
                        if ((*it)->ty == TOKEN_TYPE_COMMA) {
                                *it = (*it)->next; // ,
                        } else {
                                break;
                        }
                }
                *it = (*it)->next; // )
        }

        while ((*it)->ty != TOKEN_TYPE_END) {
                APPEND(*it);
                *it = (*it)->next;
        }
        *it = (*it)->next; // end

        mc->id     = strdup(id->lx);
        mc->params = params;

        smap_insert(&g_macros, mc->id, (void *)mc);

#undef APPEND
}

char *
preproc(const char *fp)
{
        char *src = forge_io_read_file_to_cstr(fp);
        lexer l = lexer_create(src, fp, LEXER_TRACK_SPACES | LEXER_TRACK_NEWLINES | LEXER_TRACK_TABS);
        free(src);
        lexer output = (lexer){.hd = NULL, .tl = NULL, .src_filepath = fp};
        token *it = l.hd;

        g_macros = smap_create(NULL);

        // First pass: collect macro definitions
        while (it) {
                if (it->ty == TOKEN_TYPE_MACRO) {
                        macro_content *mc = (macro_content *)malloc(sizeof(macro_content));
                        process_macro_definition(&l, &it, mc);
                } else {
                        token *t = token_alloc(it->lx, strlen(it->lx), it->ty,
                                               it->loc.r, it->loc.c, it->loc.fp);
                        lexer_append(&output, t);
                        it = it->next;
                }
        }

        // Second pass: replace macro calls with macro defs.
        it = output.hd;
        token *prev = NULL;
        while (it) {
                if (it->ty == TOKEN_TYPE_IDENTIFIER && smap_has(&g_macros, it->lx)) {
                        macro_content *mc = (macro_content *)smap_get(&g_macros, it->lx);
                        prev->next = mc->hd;
                        mc->tl->next = it->next;
                        it = mc->hd;
                }
                prev = it;
                it = it->next;
        }

        //lexer_dump(&output);

        // Convert final output to string
        char *result = (char *)malloc(1024);
        result = lexer_as_cstr(&output, result, 1024);

        printf("%s\n", result);

        return result;
        //return NULL;
}

