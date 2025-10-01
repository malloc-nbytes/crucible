#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "grammar.h"

#include <forge/array.h>

typedef struct {
        stmt_array stmts;
        const char *modname;
} program;

program parser_create_program(lexer *l);

void parser_dump_program(const program *p);

#endif // PARSER_H_INCLUDED
