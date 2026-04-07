#ifndef TOKEN_H_INCLUDED
#define TOKEN_H_INCLUDED

#include "loc.h"
#include "ds/array.h"
#include "ds/sv.h"

typedef enum {
        // Special
        TK_EOF = 0,
        TK_IDENT,
        TK_STRLIT,
        TK_INTLIT,
        TK_KW,
        TK_TYPE,
        // Operators
        TK_LPAREN,
        TK_RPAREN,
        TK_LCURLY,
        TK_RCURLY,
        TK_LSQR,
        TK_RSQR,
        TK_COLON,
        TK_BANG,
        TK_SEMI,
} token_kind;

typedef struct token {
        token_kind kind;
        sv         lx;
        loc        loc;
} token;

ARRAY_DEFINE(token *, tokenp_ar);

token *token_from(token_kind  k,
                  const char *st,
                  size_t      n,
                  size_t      r,
                  size_t      c,
                  const char *path);

const char *token_kind_cstr(token_kind k);

#endif // TOKEN_H_INCLUDED
