#ifndef GRAMMAR_H_INCLUDED
#define GRAMMAR_H_INCLUDED

#include "loc.h"
#include "type.h"

typedef enum {
        STMT_KIND_PROC,
} stmt_kind;

typedef enum {
        EXPR_KIND_INTLIT,
} expr_kind;

typedef struct {
        loc loc;
        expr_kind kind;
} expr;

typedef struct {
        loc loc;
        stmt_kind kind;
} stmt;

typedef struct {
        stmt base;
        strv id;
        type *type;
        expr *e;
} stmt_let;

#endif // GRAMMAR_H_INCLUDED
