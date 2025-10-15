#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include "loc.h"

#include <forge/array.h>

#include <stddef.h>
#include <stdint.h>

#define LEXER_TRACK_SPACES   (1 << 0)
#define LEXER_TRACK_TABS     (1 << 1)
#define LEXER_TRACK_NEWLINES (1 << 2)

typedef enum {
        TOKEN_TYPE_EOF,
        TOKEN_TYPE_IDENTIFIER,
        TOKEN_TYPE_KEYWORD,
        TOKEN_TYPE_INTEGER_LITERAL,
        TOKEN_TYPE_STRING_LITERAL,
        TOKEN_TYPE_CHARACTER_LITERAL,

        // Symbols
        TOKEN_TYPE_LEFT_PARENTHESIS,
        TOKEN_TYPE_RIGHT_PARENTHESIS,
        TOKEN_TYPE_LEFT_CURLY,
        TOKEN_TYPE_RIGHT_CURLY,
        TOKEN_TYPE_LEFT_SQUARE,
        TOKEN_TYPE_RIGHT_SQUARE,
        TOKEN_TYPE_BACKTICK,
        TOKEN_TYPE_TILDE,
        TOKEN_TYPE_BANG,
        TOKEN_TYPE_AT,
        TOKEN_TYPE_HASH,
        TOKEN_TYPE_DOLLAR,
        TOKEN_TYPE_EQUALS,
        TOKEN_TYPE_PIPE,
        TOKEN_TYPE_BACKSLASH,
        TOKEN_TYPE_FORWARDSLASH,
        TOKEN_TYPE_COMMA,
        TOKEN_TYPE_PERIOD,
        TOKEN_TYPE_QUESTION,
        TOKEN_TYPE_SEMICOLON,
        TOKEN_TYPE_COLON,
        TOKEN_TYPE_ELLIPSIS,
        TOKEN_TYPE_DOUBLE_COLON,

        TOKEN_TYPE_OTHER_LEN,

        TOKEN_TYPE_PERCENT,
        TOKEN_TYPE_UPTICK,
        TOKEN_TYPE_AMPERSAND,
        TOKEN_TYPE_ASTERISK,
        TOKEN_TYPE_PLUS,
        TOKEN_TYPE_MINUS,
        TOKEN_TYPE_LESSTHAN,
        TOKEN_TYPE_GREATERTHAN,
        TOKEN_TYPE_PLUS_EQUALS,
        TOKEN_TYPE_MINUS_EQUALS,
        TOKEN_TYPE_ASTERISK_EQUALS,
        TOKEN_TYPE_FORWARDSLASH_EQUALS,
        TOKEN_TYPE_PERCENT_EQUALS,
        TOKEN_TYPE_AMPERSAND_EQUALS,
        TOKEN_TYPE_PIPE_EQUALS,
        TOKEN_TYPE_UPTICK_EQUALS,

        TOKEN_TYPE_DOUBLE_EQUALS,
        TOKEN_TYPE_GREATERTHAN_EQUALS,
        TOKEN_TYPE_LESSTHAN_EQUALS,
        TOKEN_TYPE_BANG_EQUALS,
        TOKEN_TYPE_DOUBLE_AMPERSAND,
        TOKEN_TYPE_DOUBLE_PIPE,

        TOKEN_TYPE_BINOP_LEN,

        // Only used in the preprocessor
        TOKEN_TYPE_WHITESPACE,
        TOKEN_TYPE_NEWLINE,
        TOKEN_TYPE_TAB,
        TOKEN_TYPE_CARRIAGE,
        TOKEN_TYPE_OTHER,
        TOKEN_TYPE_MACRO,
        TOKEN_TYPE_END,
} token_type;

typedef struct token {
        char *lx;
        token_type ty;
        loc loc;
        struct token *next;
} token;

DYN_ARRAY_TYPE(token *, token_array);

typedef struct {
        token *hd;
        token *tl;
        const char *src_filepath;
} lexer;

lexer lexer_create(const char *src, const char *fp, uint32_t config);
token *token_alloc(const char *st,
                   size_t      st_n,
                   token_type  ty,
                   size_t      r,
                   size_t      c,
                   const char *fp);
void lexer_dump(const lexer *l);
token *lexer_peek(const lexer *l, size_t peek);
token *lexer_next(lexer *l);
void lexer_discard(lexer *l);
const char *token_type_to_cstr(token_type ty);
void lexer_append(lexer *l, token *t);
size_t consume_while(const char *st, int (pred)(int));
int is_ignorable(int c);
int is_ident(int c);
int not_double_quote(int c);
int not_eol(int c);
int issym(int c);
token_type *determine_sym(const char *s, size_t *len);

#endif // LEXER_H_INCLUDED
