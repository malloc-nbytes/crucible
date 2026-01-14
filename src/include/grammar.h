#ifndef GRAMMAR_H_INCLUDED
#define GRAMMAR_H_INCLUDED

#include "loc.h"
#include "type.h"
#include "mem.h"
#include "ds/strv.h"
#include "ds/array.h"

typedef struct visitor visitor;

typedef enum {
        STMT_KIND_PROC,
} stmt_kind;

typedef enum {
        EXPR_KIND_INTLIT,
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

typedef struct stmt {
        stmt_kind kind;
        void *(*accept)(struct stmt *, visitor *);
        loc loc;
} stmt;

ARRAY_TYPE(stmt *, stmt_array);

typedef struct {
        stmt base;
        strv id;
        type *type;
        expr *e;
} stmt_let;

expr_intlit *expr_intlit_alloc(int i, arena *a);

#endif // GRAMMAR_H_INCLUDED
