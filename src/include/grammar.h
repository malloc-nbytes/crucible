#ifndef GRAMMAR_H_INCLUDED
#define GRAMMAR_H_INCLUDED

#include "types.h"
#include "lexer.h"
#include "loc.h"

#include <forge/array.h>

typedef struct sym sym;

typedef struct visitor visitor;

typedef enum {
        EXPR_KIND_BINARY = 0,
        EXPR_KIND_IDENTIFIER,
        EXPR_KIND_INTEGER_LITERAL,
        EXPR_KIND_STRING_LITERAL,
        EXPR_KIND_CHARACTER_LITERAL,
        EXPR_KIND_MUT,
        EXPR_KIND_UNARY,
        EXPR_KIND_PROCCALL,
} expr_kind;

typedef enum {
        STMT_KIND_LET = 0,
        STMT_KIND_EXPR,
        STMT_KIND_BLOCK,
        STMT_KIND_PROC,
        STMT_KIND_RETURN,
        STMT_KIND_EXIT,
        STMT_KIND_EXTERN_PROC,
        STMT_KIND_IF,
        STMT_KIND_WHILE,
} stmt_kind;

///////////////////////////////////////////
// EXPRESSIONS
///////////////////////////////////////////

typedef struct expr {
        expr_kind kind;
        type *type;
        loc loc;
        void *(*accept)(struct expr *e, visitor *v);
} expr;

DYN_ARRAY_TYPE(expr *, expr_array);

typedef struct {
        expr base;
        expr *lhs;
        expr *rhs;
        const token *op;
} expr_binary;

typedef struct {
        expr base;
        const token *id;

        sym *resolved;
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

typedef struct {
        expr base;
        expr *lhs;
        expr_array args;
} expr_proccall;

///////////////////////////////////////////
// STATEMENTS
///////////////////////////////////////////

typedef struct stmt {
        stmt_kind kind;
        loc loc;
        void *(*accept)(struct stmt *s, visitor *v);
} stmt;

DYN_ARRAY_TYPE(stmt *, stmt_array);

typedef struct {
        stmt base;
        const token *id;
        type *type;
        expr *e;

        sym *resolved;
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

        sym *resolved;
} parameter;

DYN_ARRAY_TYPE(parameter, parameter_array);

typedef struct {
        stmt base;
        int export;
        const token *id;
        parameter_array params;
        int variadic;
        type *type;
        stmt *blk;

        int rsp; // resolved in semantic analysis
} stmt_proc;

typedef struct {
        stmt base;
        expr *e; // can be NULL
} stmt_return;

typedef struct {
        stmt base;
        expr *e; // can be NULL
} stmt_exit;

typedef struct {
        stmt base;
        const token *id;
        parameter_array params;
        int variadic;
        type *type;
} stmt_extern_proc;

typedef struct {
        stmt base;
        expr *e;
        stmt *then;
        stmt *else_; // can be NULL
} stmt_if;

typedef struct {
        stmt base;
        expr *e;
        stmt *body;
} stmt_while;

expr_identifier *expr_identifier_alloc(const token *id);
expr_integer_literal *expr_integer_literal_alloc(const token *i);
expr_string_literal *expr_string_literal_alloc(const token *s);
expr_mut *expr_mut_alloc(expr *lhs, const token *op, expr *rhs);
expr_bin *expr_bin_alloc(expr *lhs, const token *op, expr *rhs);
expr_un *expr_un_alloc(expr *operand, const token *op);
expr_proccall *expr_proccall_alloc(expr *lhs, expr_array args);

stmt_let *stmt_let_alloc(const token *id, type *type, expr *e);
stmt_expr *stmt_expr_alloc(expr *e);
stmt_proc *stmt_proc_alloc(
        int export,
        const token *id,
        parameter_array params,
        int variadic,
        type *type,
        stmt *blk
);
stmt_extern_proc *stmt_extern_proc_alloc(
        const token *id,
        parameter_array params,
        int variadic,
        type *type
);
stmt_block *stmt_block_alloc(stmt_array stmts);
stmt_return *stmt_return_alloc(expr *e);
stmt_exit *stmt_exit_alloc(expr *e);
stmt_if *stmt_if_alloc(expr *e, stmt *then, stmt *else_);
stmt_while *stmt_while_alloc(expr *e, stmt *body);

#endif // GRAMMAR_H_INCLUDED
