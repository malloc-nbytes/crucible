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
        token *current = macro->hd;
        while (current) {
                token *next = current->next;
                free(current->lx);
                free(current);
                current = next;
        }
        free(macro);
}

static void
process_macro_definition(lexer *l, token **it, macro_content *mc)
{
        mc->hd     = NULL;
        mc->tl     = NULL;

#define APPEND(t)                               \
        do {                                    \
                if (!mc->hd && !mc->tl) {       \
                        mc->hd = mc->tl = t;    \
                } else {                        \
                        mc->tl->next = t;       \
                        mc->tl = t;             \
                }                               \
        } while (0)

        *it = (*it)->next; // macro

        // Consume whitespace
        while ((*it)->ty == TOKEN_TYPE_WHITESPACE || (*it)->ty == TOKEN_TYPE_TAB || (*it)->ty == TOKEN_TYPE_NEWLINE) {
                *it = (*it)->next; // ' '
        }

        token *id = *it;
        *it = (*it)->next; // id
        token_array params = dyn_array_empty(token_array);

        if ((*it)->ty == TOKEN_TYPE_LEFT_PARENTHESIS) {
                *it = (*it)->next; // (
                while ((*it)->ty != TOKEN_TYPE_RIGHT_PARENTHESIS) {
                        if ((*it)->ty == TOKEN_TYPE_IDENTIFIER) {
                                dyn_array_append(params, *it);
                        }
                        *it = (*it)->next; // id or comma
                        if ((*it)->ty == TOKEN_TYPE_COMMA) {
                                *it = (*it)->next; // ,
                        }
                }
                *it = (*it)->next; // )
        }

        while ((*it)->ty != TOKEN_TYPE_END) {
                token *t = token_alloc((*it)->lx, strlen((*it)->lx), (*it)->ty, (*it)->loc.r, (*it)->loc.c, (*it)->loc.fp);
                APPEND(t);
                *it = (*it)->next;
        }
        *it = (*it)->next; // end

        mc->id     = strdup(id->lx);
        mc->params = params;

        smap_insert(&g_macros, mc->id, (void *)mc);

#undef APPEND
}

static token *
copy_token(const token *src)
{
        token *t = token_alloc(src->lx, strlen(src->lx), src->ty, src->loc.r, src->loc.c, src->loc.fp);
        return t;
}

static token *
expand_macro(macro_content *mc, token_array args, const char *fp, size_t r, size_t c)
{
        token *new_head = NULL, *new_tail = NULL;
        smap param_map = smap_create(NULL);

        // Map parameters to arguments
        for (size_t i = 0; i < mc->params.len && i < args.len; ++i) {
                smap_insert(&param_map, mc->params.data[i]->lx, copy_token(args.data[i]));
        }

        // Copy macro body, replacing parameters with arguments
        token *current = mc->hd;
        while (current) {
                token *new_token;
                if (current->ty == TOKEN_TYPE_IDENTIFIER && smap_has(&param_map, current->lx)) {
                        new_token = copy_token((token *)smap_get(&param_map, current->lx));
                } else {
                        new_token = token_alloc(current->lx, strlen(current->lx), current->ty, r, c, fp);
                }
                
                if (!new_head) {
                        new_head = new_tail = new_token;
                } else {
                        new_tail->next = new_token;
                        new_tail = new_token;
                }
                current = current->next;
        }

        // Cleanup param_map
        /* smap_iter_t iter = smap_iter(&param_map); */
        /* while (smap_iter_next(&iter)) { */
        /*         token *t = (token *)iter.value; */
        /*         free(t->lx); */
        /*         free(t); */
        /* } */
        /* smap_destroy(&param_map); */

        return new_head;
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

        // Second pass: replace macro calls with macro definitions
        it = output.hd;
        token *prev = NULL;
        while (it) {
                if (it->ty == TOKEN_TYPE_IDENTIFIER && smap_has(&g_macros, it->lx)) {
                        macro_content *mc = (macro_content *)smap_get(&g_macros, it->lx);
                        token_array args = dyn_array_empty(token_array);
                        token *next = it->next;

                        // Collect arguments if macro has parameters
                        if (mc->params.len && next && next->ty == TOKEN_TYPE_LEFT_PARENTHESIS) {
                                next = next->next; // Skip (
                                while (next && next->ty != TOKEN_TYPE_RIGHT_PARENTHESIS) {
                                        if (next->ty != TOKEN_TYPE_COMMA) {
                                                dyn_array_append(args, next);
                                        }
                                        next = next->next;
                                }
                                if (next) next = next->next; // Skip )
                        }

                        // Expand macro
                        token *expanded = expand_macro(mc, args, it->loc.fp, it->loc.r, it->loc.c);
                        dyn_array_free(args);

                        // Rewire token list
                        if (prev) {
                                prev->next = expanded;
                        } else {
                                output.hd = expanded;
                        }

                        // Find new tail
                        token *expanded_tail = expanded;
                        while (expanded_tail && expanded_tail->next) {
                                expanded_tail = expanded_tail->next;
                        }
                        if (expanded_tail) {
                                expanded_tail->next = next;
                                if (!output.tl || output.tl == it) {
                                        output.tl = expanded_tail;
                                }
                        }

                        it = next;
                } else {
                        prev = it;
                        it = it->next;
                }
        }

        // Convert final output to string
        char *result = (char *)malloc(1024);
        result = lexer_as_cstr(&output, result, 1024);
        printf("%s\n", result);

        // Cleanup output lexer
        token *current = output.hd;
        while (current) {
                token *next = current->next;
                free(current->lx);
                free(current);
                current = next;
        }

        //smap_destroy(&g_macros);

        return result;
}
