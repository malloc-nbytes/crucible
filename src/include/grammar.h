#ifndef GRAMMAR_H_INCLUDED
#define GRAMMAR_H_INCLUDED

#include "loc.h"
#include "type.h"
#include "mem.h"
#include "lex.h"
#include "symbol.h"
#include "operator.h"
#include "ds/strv.h"
#include "ds/array.h"

typedef struct visitor visitor;

typedef enum {
        STMT_KIND_LET = 0,
        STMT_KIND_PROC,
        STMT_KIND_BLK,
        STMT_KIND_RETURN,
        STMT_KIND_EXIT,
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

        type *type; // final checked type
        int is_lvalue; // asssignable?
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

        operator *resolved_op;
} expr_bin;

typedef struct {
        expr base;
        const token *i;

        symbol *sym; // resolved symbol
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

        int reachable; // control-flow analysis
} stmt;

typedef struct {
        type *type;
        const token *id;

        symbol *sym;
} idty;

ARRAY_TYPE(stmt *, stmt_array);
ARRAY_TYPE(idty *, idty_array);

typedef struct {
        stmt base;
        const token *id;
        type *type;
        expr *e;

        symbol *sym; // variable symbol
} stmt_let;

typedef struct {
        stmt base;
        const token *id;
        idty_array params;
        type *rtype;
        stmt *blk;

        symbol *sym; // procedure symbol
        scope *scope; // parameter + local scope
        int returns_value; // control-flow result
} stmt_proc;

typedef struct {
        stmt base;
        stmt_array stmts;

        scope *scope;
} stmt_blk;

typedef struct {
        stmt base;
        expr *e;

        stmt_proc *proc; // enclosing procedure
} stmt_return;

typedef struct {
        stmt base;
        expr *e;

        stmt_proc *proc; // enclosing procedure
} stmt_exit;

idty *idty_alloc(const token *id, type *type, arena *a);
expr_intlit *expr_intlit_alloc(int i, arena *a);
expr_bin *expr_bin_alloc(expr *lhs, const token *op, expr *rhs, arena *a);
expr_un *expr_un_alloc(const token *op, expr *rhs, arena *a);
expr_id *expr_id_alloc(const token *i, arena *a);

stmt_let *stmt_let_alloc(const token *id,
                         type        *type,
                         expr        *e,
                         arena       *a);
stmt_proc *stmt_proc_alloc(const token *id, idty_array params, type *rtype, stmt *blk, arena *a);
stmt_blk *stmt_blk_alloc(stmt_array stmts, arena *a);
stmt_return *stmt_return_alloc(expr *e, arena *a);
stmt_exit *stmt_exit_alloc(expr *e, arena *a);

#endif // GRAMMAR_H_INCLUDED
