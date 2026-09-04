#ifndef GRAMMAR_H_INCLUDED
#define GRAMMAR_H_INCLUDED

#include "token.hpp"
#include "symbol.hpp"
#include "location.hpp"
#include "type.hpp"

typedef struct visitor visitor;

typedef enum {
        EXPR_KIND_INT = 0,
        EXPR_KIND_IDENTIFIER,
        EXPR_KIND_BINARY,
} expr_kind;

typedef enum {
        STMT_KIND_EXPR = 0,
        STMT_KIND_LET,
} stmt_kind;

typedef struct expr {
        expr_kind        k;
        location         loc;
        type            *ty;
        void *(*accept)(struct expr *, visitor *);
} expr;

typedef struct {
        expr             base;
        const token     *i;
} expr_int;

typedef struct {
        expr             base;
        const token     *id;
        symbol          *sym;
} expr_identifier;

typedef struct stmt {
        stmt_kind       k;
        location        loc;
        void *(*accept)(struct stmt *, visitor *);
} stmt;

typedef struct {
        stmt     base;
        expr    *e;
} stmt_expr;

typedef struct {
        stmt             base;
        const token     *id;
        type            *ty;
        expr            *e;
        symbol          *sym;
} stmt_let;

expr_int        *expr_int_alloc(const token *i);
expr_identifier *expr_identifier_alloc(const token *id);
stmt_expr       *stmt_expr_alloc(expr *e);
stmt_let        *stmt_let_alloc(location         loc,
                                const token     *id,
                                type            *ty,
                                expr            *e);

#endif // GRAMMAR_H_INCLUDED
