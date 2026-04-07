#ifndef GRAMMAR_H_INCLUDED
#define GRAMMAR_H_INCLUDED

#include "token.h"
#include "loc.h"
#include "types.h"
#include "ds/array.h"

#include <stdint.h>

#define PROC_META_VARIADIC (1 << 0)
#define PROC_META_EXTERN   (1 << 1)
#define PROC_META_EXPORT   (1 << 2)

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

ARRAY_DEFINE(expr *, exprp_ar);

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

typedef enum {
        STMT_KIND_BLOCK = 0,
        STMT_KIND_PROC,
        STMT_KIND_EXPR,
        STMT_KIND_LET,
} stmt_kind;

typedef struct {
        stmt_kind kind;
        loc       loc;
} stmt;

ARRAY_DEFINE(stmt *, stmtp_ar);

typedef struct {
        stmt     base;
        stmtp_ar stmts;
} stmt_block;

typedef struct {
        stmt        base;
        token      *id;
        typep_ar    ptypes;
        tokenp_ar   pids;
        type       *rtype;
        stmt_block *blk;
        uint32_t    meta;
} stmt_proc;

expr_ident  *expr_ident_alloc(token *id);
expr_intlit *expr_intlit_alloc(token *i);
expr_strlit *expr_strlit_alloc(token *s);
expr_bin    *expr_bin_alloc(expr *lhs, token *op, expr *rhs);
expr_un     *expr_un_alloc(token *prefix, expr *rhs);

stmt_block *stmt_block_alloc(stmtp_ar stmts);
stmt_proc  *stmt_proc_alloc(token      *id,
                            typep_ar    ptypes,
                            tokenp_ar   pids,
                            type       *rtype,
                            stmt_block *blk,
                            uint32_t    meta);

#endif // GRAMMAR_H_INCLUDED
