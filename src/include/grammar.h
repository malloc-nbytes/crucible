#ifndef GRAMMAR_H_INCLUDED
#define GRAMMAR_H_INCLUDED

#include "types.h"
#include "lexer.h"

#include <forge/array.h>

typedef struct visitor visitor;

typedef enum {
        EXPR_KIND_BINARY = 0,
        EXPR_KIND_IDENTIFIER,
        EXPR_KIND_INTEGER_LITERAL,
        EXPR_KIND_STRING_LITERAL,
        EXPR_KIND_CHARACTER_LITERAL,
        EXPR_KIND_MUT,
        EXPR_KIND_UNARY,
} expr_kind;

typedef enum {
        STMT_KIND_LET = 0,
        STMT_KIND_EXPR,
        STMT_KIND_BLOCK,
        STMT_KIND_PROC,
} stmt_kind;

///////////////////////////////////////////
// EXPRESSIONS
///////////////////////////////////////////

typedef struct expr {
        expr_kind kind;
        // TODO: types
        void *(*accept)(struct expr *e, visitor *v);
} expr;

typedef struct {
        expr base;
        expr *lhs;
        expr *rhs;
        const token *op;
} expr_binary;

typedef struct {
        expr base;
        const token *id;
} expr_identifier;

typedef struct {
        expr base;
        const token *i;
} expr_integer_literal;

typedef struct {
        expr base;
        const token *s;
} expr_string_literal;

typedef struct {
        expr base;
        const token *c;
} expr_character_literal;

typedef struct {
        expr base;
        expr *lhs;
        const token *op;
        expr *rhs;
} expr_mut;

typedef struct {
        expr base;
        expr *lhs;
        const token *op;
        expr *rhs;
} expr_bin;

typedef struct {
        expr base;
        expr *operand;
        const token *op;
} expr_un;

///////////////////////////////////////////
// STATEMENTS
///////////////////////////////////////////

typedef struct stmt {
        stmt_kind kind;
        void *(*accept)(struct stmt *s, visitor *v);
} stmt;

DYN_ARRAY_TYPE(stmt *, stmt_array);

typedef struct {
        stmt base;
        const token *id;
        const type *type;
        expr *e;
} stmt_let;

typedef struct {
        stmt base;
        expr *e;
} stmt_expr;

typedef struct {
        stmt base;
        stmt_array stmts;
} stmt_block;

typedef struct {
        const token *id;
        type *type;
} parameter;

DYN_ARRAY_TYPE(parameter, parameter_array);

typedef struct {
        stmt base;
        const token *id;
        parameter_array params;
        type *type;
        stmt *blk;
} stmt_proc;

expr_identifier *expr_identifier_alloc(const token *id);
expr_integer_literal *expr_integer_literal_alloc(const token *i);
expr_mut *expr_mut_alloc(expr *lhs, const token *op, expr *rhs);
expr_bin *expr_bin_alloc(expr *lhs, const token *op, expr *rhs);
expr_un *expr_un_alloc(expr *operand, const token *op);

stmt_let *stmt_let_alloc(const token *id, const type *type, expr *e);
stmt_expr *stmt_expr_alloc(expr *e);
stmt_proc *stmt_proc_alloc(const token *id, parameter_array params, type *type, stmt *blk);
stmt_block *stmt_block_alloc(stmt_array stmts);

#endif // GRAMMAR_H_INCLUDED
