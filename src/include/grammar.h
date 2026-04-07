#ifndef GRAMMAR_H_INCLUDED
#define GRAMMAR_H_INCLUDED

#include "token.h"
#include "loc.h"

typedef enum {
        EXPR_KIND_IDENT = 0,
        EXPR_KIND_INTLIT,
        EXPR_KIND_STRLIT,
        EXPR_KIND_BIN,
        EXPR_KIND_UN,
} expr_kind;   

typedef struct {
        expr_kind kind;
        loc       loc;
} expr;

typedef struct {
        expr   base;
        token *id;
} expr_ident;

typedef struct {
        expr   base;
        token *i;
} expr_intlit;

typedef struct {
        expr   base;
        token *s;
} expr_strlit;

typedef struct {
        expr   base;
        expr  *lhs;
        token *op;
        expr  *rhs;
} expr_bin;

typedef struct {
        expr   base;
        token *prefix;
        expr  *e;
} expr_un;

expr_ident  expr_ident_alloc(token *id);
expr_intlit expr_intlit_alloc(token *i);
expr_strlit expr_strlit_alloc(token *s);
expr_bin    expr_bin_alloc(expr *lhs, token *op, expr *rhs);
expr_un     expr_un_alloc(token *prefix, expr *rhs);

#endif // GRAMMAR_H_INCLUDED
