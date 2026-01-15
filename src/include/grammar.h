#ifndef GRAMMAR_H_INCLUDED
#define GRAMMAR_H_INCLUDED

#include "loc.h"
#include "type.h"
#include "mem.h"
#include "lex.h"
#include "ds/strv.h"
#include "ds/array.h"

typedef struct visitor visitor;

typedef struct {
        type *type;
        const token *id;
} tyid;

ARRAY_TYPE(tyid *, tyid_array);

typedef enum {
        STMT_KIND_LET = 0,
        STMT_KIND_PROC,
} stmt_kind;

typedef enum {
        EXPR_KIND_INTLIT = 0,
        EXPR_KIND_BIN,
        EXPR_KIND_UN,
        EXPR_KIND_ID,
} expr_kind;

typedef struct expr {
        expr_kind kind;
        void *(*accept)(struct expr *, visitor *);
        loc loc;
} expr;

typedef struct {
        expr base;
        int i;
} expr_intlit;

typedef struct {
        expr base;
        expr *lhs;
        const token *op;
        expr *rhs;
} expr_bin;

typedef struct {
        expr base;
        const token *i;
} expr_id;

typedef struct {
        expr base;
        const token *op;
        expr *rhs;
} expr_un;

typedef struct stmt {
        stmt_kind kind;
        void *(*accept)(struct stmt *, visitor *);
        loc loc;
} stmt;

ARRAY_TYPE(stmt *, stmt_array);

typedef struct {
        stmt base;
        const token *id;
        type *type;
        expr *e;
} stmt_let;

typedef struct {
        stmt base;
        const token *id;
        tyid_array params;
        type *rtype;
        stmt *blk;
} stmt_proc;

tyid *tyid_alloc(type *type, const token *id, arena *a);
expr_intlit *expr_intlit_alloc(int i, arena *a);
expr_bin *expr_bin_alloc(expr *lhs, const token *op, expr *rhs, arena *a);
expr_un *expr_un_alloc(const token *op, expr *rhs, arena *a);
expr_id *expr_id_alloc(const token *i, arena *a);

stmt_let *stmt_let_alloc(const token *id,
                         type        *type,
                         expr        *e,
                         arena       *a);
stmt_proc *stmt_proc_alloc(const token *id, tyid_array params, type *rtype, stmt *blk, arena *a);

#endif // GRAMMAR_H_INCLUDED
