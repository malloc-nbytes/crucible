#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "lex.hpp"
#include "grammar.hpp"

#include <vector>

typedef struct {
        lexer                   *l;
        int                      global;
        std::vector<stmt *>      stmts;
} parser;

parser parse(lexer *l);

#endif // PARSER_H_INCLUDED
