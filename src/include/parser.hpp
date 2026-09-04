#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "lex.hpp"

typedef struct {
        lexer *l;
} parser;

parser parse(lexer *l);

#endif // PARSER_H_INCLUDED
