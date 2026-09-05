#ifndef GRAMMAR_H_INCLUDED
#define GRAMMAR_H_INCLUDED

#include "token.hpp"
#include "symbol.hpp"
#include "location.hpp"
#include "type.hpp"

#include <cstdint>
#include <optional>
#include <vector>

typedef struct visitor visitor;

typedef enum {
        EXPR_KIND_INT = 0,
        EXPR_KIND_IDENTIFIER,
        EXPR_KIND_BINARY,
        EXPR_KIND_UNARY,
} expr_kind;

typedef enum {
        STMT_KIND_EXPR = 0,
        STMT_KIND_LET,
        STMT_KIND_PROC,
        STMT_KIND_BLOCK,
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

typedef struct {
        expr             base;
        const token     *op;
        expr            *rhs;
} expr_unary;

typedef struct {
        expr         base;
        expr        *lhs;
        const token *op;
        expr        *rhs;
} expr_binary;

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

#define PROC_LINKAGE_INTERNAL (1 << 0)
#define PROC_LINKAGE_EXPORT   (1 << 1)
#define PROC_LINKAGE_EXTERN   (1 << 2)
#define PROC_VARIADIC         (1 << 3)

typedef struct {
        token   *id;
        type    *ty;
        symbol  *sym;
} procp;

typedef struct {
        stmt                     base;
        uint32_t                 bits;
        token                   *id;
        std::vector<procp>       params;
        type                    *rty;
        std::optional<stmt *>    body;
        symbol                  *sym;
} stmt_proc;

typedef struct {
        stmt base;
        std::vector<stmt *> stmts;
} stmt_block;

expr_int        *expr_int_alloc(const token *i);
expr_identifier *expr_identifier_alloc(const token *id);
expr_unary      *expr_unary_alloc(const token *op, expr *rhs);
expr_binary      *expr_binary_alloc(expr *lhs, const token *op, expr *rhs);
stmt_expr       *stmt_expr_alloc(expr *e);
stmt_let        *stmt_let_alloc(location         loc,
                                const token     *id,
                                type            *ty,
                                expr            *e);
stmt_proc *
stmt_proc_alloc(location                         loc,
                uint32_t                         bits,
                token                           *id,
                std::vector<procp>               params,
                type                            *rty,
                std::optional<stmt *>            body);

stmt_block *stmt_block_alloc(location loc, std::vector<stmt *> stmts);

#endif // GRAMMAR_H_INCLUDED
