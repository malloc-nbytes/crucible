#include "lexer.h"
#include "mem.h"
#include "kwds.h"
#include "loc.h"
#include "ds/smap.h"

#include <forge/err.h>
#include <forge/colors.h>
#include <forge/smap.h>
#include <forge/utils.h>
#include <forge/cstr.h>

#include <assert.h>
#include <string.h>
#include <ctype.h>

static smap g_syms = {0};

const char *
token_type_to_cstr(token_type ty)
{
        switch (ty) {
        case TOKEN_TYPE_EOF:                 return "TOKEN_TYPE_EOF";
        case TOKEN_TYPE_IDENTIFIER:          return "TOKEN_TYPE_IDENTIFIER";
        case TOKEN_TYPE_KEYWORD:             return "TOKEN_TYPE_KEYWORD";
        case TOKEN_TYPE_INTEGER_LITERAL:     return "TOKEN_TYPE_INTEGER_LITERAL";
        case TOKEN_TYPE_STRING_LITERAL:      return "TOKEN_TYPE_STRING_LITERAL";
        case TOKEN_TYPE_CHARACTER_LITERAL:   return "TOKEN_TYPE_CHARACTER_LITERAL";
        case TOKEN_TYPE_LEFT_PARENTHESIS:    return "TOKEN_TYPE_LEFT_PARENTHESIS";
        case TOKEN_TYPE_RIGHT_PARENTHESIS:   return "TOKEN_TYPE_RIGHT_PARENTHESIS";
        case TOKEN_TYPE_LEFT_CURLY:          return "TOKEN_TYPE_LEFT_CURLY";
        case TOKEN_TYPE_RIGHT_CURLY:         return "TOKEN_TYPE_RIGHT_CURLY";
        case TOKEN_TYPE_LEFT_SQUARE:         return "TOKEN_TYPE_LEFT_SQUARE";
        case TOKEN_TYPE_RIGHT_SQUARE:        return "TOKEN_TYPE_RIGHT_SQUARE";
        case TOKEN_TYPE_BACKTICK:            return "TOKEN_TYPE_BACKTICK";
        case TOKEN_TYPE_TILDE:               return "TOKEN_TYPE_TILDE";
        case TOKEN_TYPE_BANG:                return "TOKEN_TYPE_BANG";
        case TOKEN_TYPE_AT:                  return "TOKEN_TYPE_AT";
        case TOKEN_TYPE_HASH:                return "TOKEN_TYPE_HASH";
        case TOKEN_TYPE_DOLLAR:              return "TOKEN_TYPE_DOLLAR";
        case TOKEN_TYPE_PERCENT:             return "TOKEN_TYPE_PERCENT";
        case TOKEN_TYPE_UPTICK:              return "TOKEN_TYPE_UPTICK";
        case TOKEN_TYPE_AMPERSAND:           return "TOKEN_TYPE_AMPERSAND";
        case TOKEN_TYPE_ASTERISK:            return "TOKEN_TYPE_ASTERISK";
        case TOKEN_TYPE_PLUS:                return "TOKEN_TYPE_PLUS";
        case TOKEN_TYPE_MINUS:               return "TOKEN_TYPE_MINUS";
        case TOKEN_TYPE_EQUALS:              return "TOKEN_TYPE_EQUALS";
        case TOKEN_TYPE_PIPE:                return "TOKEN_TYPE_PIPE";
        case TOKEN_TYPE_BACKSLASH:           return "TOKEN_TYPE_BACKSLASH";
        case TOKEN_TYPE_FORWARDSLASH:        return "TOKEN_TYPE_FORWARDSLASH";
        case TOKEN_TYPE_LESSTHAN:            return "TOKEN_TYPE_LESSTHAN";
        case TOKEN_TYPE_GREATERTHAN:         return "TOKEN_TYPE_GREATERTHAN";
        case TOKEN_TYPE_COMMA:               return "TOKEN_TYPE_COMMA";
        case TOKEN_TYPE_PERIOD:              return "TOKEN_TYPE_PERIOD";
        case TOKEN_TYPE_QUESTION:            return "TOKEN_TYPE_QUESTION";
        case TOKEN_TYPE_SEMICOLON:           return "TOKEN_TYPE_SEMICOLON";
        case TOKEN_TYPE_COLON:               return "TOKEN_TYPE_COLON";
        case TOKEN_TYPE_PLUS_EQUALS:         return "TOKEN_TYPE_PLUS_EQUALS";
        case TOKEN_TYPE_MINUS_EQUALS:        return "TOKEN_TYPE_MINUS_EQUALS";
        case TOKEN_TYPE_ASTERISK_EQUALS:     return "TOKEN_TYPE_ASTERISK_EQUALS";
        case TOKEN_TYPE_FORWARDSLASH_EQUALS: return "TOKEN_TYPE_FORWARDSLASH_EQUALS";
        case TOKEN_TYPE_PERCENT_EQUALS:      return "TOKEN_TYPE_PERCENT_EQUALS";
        case TOKEN_TYPE_AMPERSAND_EQUALS:    return "TOKEN_TYPE_AMPERSAND_EQUALS";
        case TOKEN_TYPE_PIPE_EQUALS:         return "TOKEN_TYPE_PIPE_EQUALS";
        case TOKEN_TYPE_UPTICK_EQUALS:       return "TOKEN_TYPE_UPTICK_EQUALS";
        case TOKEN_TYPE_DOUBLE_EQUALS:       return "TOKEN_TYPE_DOUBLE_EQUALS";
        case TOKEN_TYPE_GREATERTHAN_EQUALS:  return "TOKEN_TYPE_GREATERTHAN_EQUALS";
        case TOKEN_TYPE_LESSTHAN_EQUALS:     return "TOKEN_TYPE_LESSTHAN_EQUALS";
        case TOKEN_TYPE_BANG_EQUALS:         return "TOKEN_TYPE_BANG_EQUALS";
        case TOKEN_TYPE_DOUBLE_AMPERSAND:    return "TOKEN_TYPE_DOUBLE_AMPERSAND";
        case TOKEN_TYPE_DOUBLE_PIPE:         return "TOKEN_TYPE_DOUBLE_PIPE";
        default: forge_err_wargs("token_type_to_cstr(): unknown token type `%d`", (int)ty);
        }
        return NULL; // unreachable
}

static void
init_syms(void)
{
        g_syms = smap_create(NULL);

        static token_type token_type_left_parenthesis    = TOKEN_TYPE_LEFT_PARENTHESIS;
        static token_type token_type_right_parenthesis   = TOKEN_TYPE_RIGHT_PARENTHESIS;
        static token_type token_type_left_curly          = TOKEN_TYPE_LEFT_CURLY;
        static token_type token_type_right_curly         = TOKEN_TYPE_RIGHT_CURLY;
        static token_type token_type_left_square         = TOKEN_TYPE_LEFT_SQUARE;
        static token_type token_type_right_square        = TOKEN_TYPE_RIGHT_SQUARE;
        static token_type token_type_backtick            = TOKEN_TYPE_BACKTICK;
        static token_type token_type_tilde               = TOKEN_TYPE_TILDE;
        static token_type token_type_bang                = TOKEN_TYPE_BANG;
        static token_type token_type_at                  = TOKEN_TYPE_AT;
        static token_type token_type_hash                = TOKEN_TYPE_HASH;
        static token_type token_type_dollar              = TOKEN_TYPE_DOLLAR;
        static token_type token_type_percent             = TOKEN_TYPE_PERCENT;
        static token_type token_type_uptick              = TOKEN_TYPE_UPTICK;
        static token_type token_type_ampersand           = TOKEN_TYPE_AMPERSAND;
        static token_type token_type_asterisk            = TOKEN_TYPE_ASTERISK;
        static token_type token_type_plus                = TOKEN_TYPE_PLUS;
        static token_type token_type_minus               = TOKEN_TYPE_MINUS;
        static token_type token_type_equals              = TOKEN_TYPE_EQUALS;
        static token_type token_type_pipe                = TOKEN_TYPE_PIPE;
        static token_type token_type_backslash           = TOKEN_TYPE_BACKSLASH;
        static token_type token_type_forwardslash        = TOKEN_TYPE_FORWARDSLASH;
        static token_type token_type_lessthan            = TOKEN_TYPE_LESSTHAN;
        static token_type token_type_greaterthan         = TOKEN_TYPE_GREATERTHAN;
        static token_type token_type_comma               = TOKEN_TYPE_COMMA;
        static token_type token_type_period              = TOKEN_TYPE_PERIOD;
        static token_type token_type_question            = TOKEN_TYPE_QUESTION;
        static token_type token_type_semicolon           = TOKEN_TYPE_SEMICOLON;
        static token_type token_type_colon               = TOKEN_TYPE_COLON;
        static token_type token_type_plus_equals         = TOKEN_TYPE_PLUS_EQUALS;
        static token_type token_type_minus_equals        = TOKEN_TYPE_MINUS_EQUALS;
        static token_type token_type_asterisk_equals     = TOKEN_TYPE_ASTERISK_EQUALS;
        static token_type token_type_forwardslash_equals = TOKEN_TYPE_FORWARDSLASH_EQUALS;
        static token_type token_type_percent_equals      = TOKEN_TYPE_PERCENT_EQUALS;
        static token_type token_type_ampersand_equals    = TOKEN_TYPE_AMPERSAND_EQUALS;
        static token_type token_type_pipe_equals         = TOKEN_TYPE_PIPE_EQUALS;
        static token_type token_type_uptick_equals       = TOKEN_TYPE_UPTICK_EQUALS;
        static token_type token_type_double_equals       = TOKEN_TYPE_DOUBLE_EQUALS;
        static token_type token_type_greaterthan_equals  = TOKEN_TYPE_GREATERTHAN_EQUALS;
        static token_type token_type_lessthan_equals     = TOKEN_TYPE_LESSTHAN_EQUALS;
        static token_type token_type_bang_equals         = TOKEN_TYPE_BANG_EQUALS;
        static token_type token_type_double_ampersand    = TOKEN_TYPE_DOUBLE_AMPERSAND;
        static token_type token_type_double_pipe         = TOKEN_TYPE_DOUBLE_PIPE;

        smap_insert(&g_syms, "(", (void*)&token_type_left_parenthesis);
        smap_insert(&g_syms, ")", (void*)&token_type_right_parenthesis);
        smap_insert(&g_syms, "{", (void*)&token_type_left_curly);
        smap_insert(&g_syms, "}", (void*)&token_type_right_curly);
        smap_insert(&g_syms, "[", (void*)&token_type_left_square);
        smap_insert(&g_syms, "]", (void*)&token_type_right_square);
        smap_insert(&g_syms, "`", (void*)&token_type_backtick);
        smap_insert(&g_syms, "~", (void*)&token_type_tilde);
        smap_insert(&g_syms, "!", (void*)&token_type_bang);
        smap_insert(&g_syms, "@", (void*)&token_type_at);
        smap_insert(&g_syms, "#", (void*)&token_type_hash);
        smap_insert(&g_syms, "$", (void*)&token_type_dollar);
        smap_insert(&g_syms, "%", (void*)&token_type_percent);
        smap_insert(&g_syms, "^", (void*)&token_type_uptick);
        smap_insert(&g_syms, "&", (void*)&token_type_ampersand);
        smap_insert(&g_syms, "*", (void*)&token_type_asterisk);
        smap_insert(&g_syms, "+", (void*)&token_type_plus);
        smap_insert(&g_syms, "-", (void*)&token_type_minus);
        smap_insert(&g_syms, "=", (void*)&token_type_equals);
        smap_insert(&g_syms, "|", (void*)&token_type_pipe);
        smap_insert(&g_syms, "\\", (void*)&token_type_backslash);
        smap_insert(&g_syms, "/", (void*)&token_type_forwardslash);
        smap_insert(&g_syms, "<", (void*)&token_type_lessthan);
        smap_insert(&g_syms, ">", (void*)&token_type_greaterthan);
        smap_insert(&g_syms, ",", (void*)&token_type_comma);
        smap_insert(&g_syms, ".", (void*)&token_type_period);
        smap_insert(&g_syms, "?", (void*)&token_type_question);
        smap_insert(&g_syms, ";", (void*)&token_type_semicolon);
        smap_insert(&g_syms, ":", (void*)&token_type_colon);
        smap_insert(&g_syms, "+=", (void*)&token_type_plus_equals);
        smap_insert(&g_syms, "-=", (void*)&token_type_minus_equals);
        smap_insert(&g_syms, "*=", (void*)&token_type_asterisk_equals);
        smap_insert(&g_syms, "/=", (void*)&token_type_forwardslash_equals);
        smap_insert(&g_syms, "%=", (void*)&token_type_percent_equals);
        smap_insert(&g_syms, "&=", (void*)&token_type_ampersand_equals);
        smap_insert(&g_syms, "|=", (void*)&token_type_pipe_equals);
        smap_insert(&g_syms, "^=", (void*)&token_type_uptick_equals);
        smap_insert(&g_syms, "==", (void*)&token_type_double_equals);
        smap_insert(&g_syms, ">=", (void*)&token_type_greaterthan_equals);
        smap_insert(&g_syms, "<=", (void*)&token_type_lessthan_equals);
        smap_insert(&g_syms, "!=", (void*)&token_type_bang_equals);
        smap_insert(&g_syms, "&&", (void*)&token_type_double_ampersand);
        smap_insert(&g_syms, "||", (void*)&token_type_double_pipe);
}

static token *
token_alloc(const char *st,
            size_t      st_n,
            token_type  ty,
            size_t      r,
            size_t      c,
            const char *fp)
{
        token *t = alloc(sizeof(token));
        t->lx    = strndup(st, st_n);
        t->ty    = ty;
        t->loc   = loc_create(fp, r, c);
        t->next  = NULL;
        return t;
}

void
lexer_append(lexer *l, token *t)
{
        if (!l->hd && !l->tl) {
                l->hd = l->tl = t;
        } else {
                token *tmp = l->tl;
                l->tl = t;
                tmp->next = t;
        }
}

static size_t
consume_while(const char *st,
              int (pred)(int))
{
        if (!st) return 0;

        size_t i = 0;
        while (st[i] && pred(st[i])) {
                ++i;
        }

        return i;
}

static int is_ignorable(int c)        { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }
static int is_ident(int c)            { return isalnum(c) || c == '_'; }
static int not_double_quote(int c)    { return c != '"'; }
static int not_eol(int c)             { return c != '\n'; }
static int issym(int c) {
        return !is_ident(c)
                && !is_ignorable(c)
                && c != '"'
                && c != '\'';
}

token_type *
determine_sym(const char *s, size_t *len)
{
        assert(*len < 256);
        char buf[256] = {0};

        while (*len > 0) {
                memset(buf, 0, 256);
                strncpy(buf, s, *len);
                if (smap_has(&g_syms, buf)) {
                        return (token_type *)smap_get(&g_syms, buf);
                }
                --(*len);
        }

        return NULL;
}

void
lexer_dump(const lexer *l)
{
        token *it = l->hd;
        while (it) {
                printf("{ lx: %s%s%s, ty: %s%s%s, fp: %s%s%s, r: %s%zu%s, c: %s%zu%s }\n",
                       YELLOW, it->lx, RESET,
                       GREEN, token_type_to_cstr(it->ty), RESET,
                       PINK, it->loc.fp, RESET,
                       PINK, it->loc.r, RESET,
                       PINK, it->loc.c, RESET);
                it = it->next;
        }
}

token *
lexer_peek(const lexer *l,
           size_t       peek)
{
        if (!l->hd) return NULL;
        token *it = l->hd;
        for (size_t i = 0; it && i < peek; ++i);
        return it;
}

token *
lexer_next(lexer *l)
{
        if (!l->hd) {
                forge_err("lexer_next(): no more tokens");
        }
        token *t = l->hd;
        l->hd = l->hd->next;
        return t;
}

void
lexer_discard(lexer *l)
{
        if (!l->hd) {
                forge_err("lexer_discard(): no more tokens");
        }
        l->hd = l->hd->next;
}

lexer
lexer_create(const char *src,
             const char *fp)
{
        if (!smap_size(&g_syms)) {
                init_syms();
        }

        lexer l = (lexer) {
                .hd = NULL,
                .tl = NULL,
        };

        int i = 0;
        size_t r = 1, c = 1;

        while (src[i]) {
                char ch = src[i];

                // TODO: comments
                if (ch == '-' && src[i+1] == '-') {
                        size_t len = consume_while(src+i, not_eol);
                        i += len+1;
                        c += len+1;
                } else if (ch == ' ') {
                        ++i, ++c;
                } else if (ch == '\n') {
                        c = 1;
                        ++r, ++i;
                } else if (ch == '\t') {
                        ++c, ++i;
                } else if (ch == '\r') {
                        c = 1;
                        ++r, ++i;
                } else if (ch == '"') {
                        size_t len = consume_while(src+i+1, not_double_quote);
                        token *t = token_alloc(src+i+1, len, TOKEN_TYPE_STRING_LITERAL, r, c, fp);
                        lexer_append(&l, t);
                        i += len+2; // +2 for each quote
                        c += len+2;
                } else if (ch == '\'') {
                        // TODO: account for escape sequences
                        // TODO: make sure you have a valid character (aka not empty)
                        token *t = token_alloc(src+i+1, 1, TOKEN_TYPE_CHARACTER_LITERAL, r, c, fp);
                        lexer_append(&l, t);
                        i += 3; // +3 for quotes + the character
                        c += 3;
                } else if (isalpha(ch) || ch == '_') {
                        size_t len = consume_while(src+i, is_ident);

                        token *t = token_alloc(src+i, len, TOKEN_TYPE_IDENTIFIER, r, c, fp);
                        if (kwds_iskw(t->lx)) {
                                t->ty = TOKEN_TYPE_KEYWORD;
                        }
                        lexer_append(&l, t);

                        i += len;
                        c += len;
                } else if (isdigit(ch)) {
                        size_t len = consume_while(src+i, isdigit);
                        token *t = token_alloc(src+i, len, TOKEN_TYPE_INTEGER_LITERAL, r, c, fp);
                        lexer_append(&l, t);
                        i += len;
                        c += len;
                } else {
                        size_t len = consume_while(src+i, issym);
                        token_type *ty = determine_sym(src+i, &len);
                        assert(ty);
                        token *t = token_alloc(src+i, len, *ty, r, c, fp);
                        lexer_append(&l, t);
                        i += len;
                        c += len;
                }
        }

        lexer_append(&l, token_alloc("EOF", 3, TOKEN_TYPE_EOF, r, c, fp));

        return l;
}
