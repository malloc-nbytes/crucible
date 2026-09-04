#ifndef TOKEN_H_INCLUDED
#define TOKEN_H_INCLUDED

#include "location.hpp"

#include <string>

typedef enum {
        TOKEN_KIND_EOF = 0,
        TOKEN_KIND_KEYWORD,
        TOKEN_KIND_PRIMITIVE_TYPE,
        TOKEN_KIND_IDENTIFIER,
        TOKEN_KIND_INTEGER_LITERAL,
        TOKEN_KIND_STRING_LITERAL,

        TOKEN_KIND_L_PAREN,
        TOKEN_KIND_R_PAREN,
        TOKEN_KIND_L_CURLY,
        TOKEN_KIND_R_CURLY,
        TOKEN_KIND_L_SQR,
        TOKEN_KIND_R_SQR,
        TOKEN_KIND_COMMA,
        TOKEN_KIND_COLON,
        TOKEN_KIND_SEMICOLON,
        TOKEN_KIND_PIPE,
        TOKEN_KIND_UPTICK,
        TOKEN_KIND_AMPERSAND,
        TOKEN_KIND_ASTERISK,
        TOKEN_KIND_PLUS,
        TOKEN_KIND_HYPHEN,
        TOKEN_KIND_FORWARD_SLASH,
        TOKEN_KIND_BANG,
        TOKEN_KIND_PERCENT,
        TOKEN_KIND_EQUALS,
        TOKEN_KIND_GREATERTHAN,
        TOKEN_KIND_LESSTHAN,
        TOKEN_KIND_PERIOD,

        TOKEN_KIND_PLUS_EQUALS,
        TOKEN_KIND_MINUS_EQUALS,
        TOKEN_KIND_ASTERISK_EQUALS,
        TOKEN_KIND_FORWARD_SLASH_EQUALS,

        TOKEN_KIND_BANG_EQUALS,
        TOKEN_KIND_GREATERTHAN_EQUALS,
        TOKEN_KIND_LESSTHAN_EQUALS,
        TOKEN_KIND_DOUBLE_EQUALS,
        TOKEN_KIND_DOUBLE_AMPERSAND,
        TOKEN_KIND_DOUBLE_PIPE,

        TOKEN_KIND_ELIPSIS,
} token_kind;

typedef struct {
        std::string     lx;
        token_kind      k;
        location        loc;
} token;

token *
token_alloc(const char *st,
            size_t      len,
            token_kind  k,
            size_t      r,
            size_t      c,
            const char *path);

void             token_dump(const token *const &t);
const char      *token_kind_to_cstring(token_kind k);

#endif // TOKEN_H_INCLUDED
