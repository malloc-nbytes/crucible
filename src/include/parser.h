#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "grammar.h"

#include <forge/array.h>

typedef struct {
        stmt_array stmts;
} program;

program parser_create_program(lexer *l);

#endif // PARSER_H_INCLUDED
