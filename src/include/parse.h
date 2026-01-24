#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

#include "lex.h"
#include "grammar.h"
#include "err.h"
#include "type.h"

typedef struct {
        lexer *l;
        arena a;
        err err;
        type_context t_ctx;
        stmt_array stmts;
} parse_context;

parse_context parse(lexer *l);

#endif // PARSE_H_INCLUDED
