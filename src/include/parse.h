#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

#include "lex.h"
#include "grammar.h"

stmt_array parse(lexer *l);

#endif // PARSE_H_INCLUDED
