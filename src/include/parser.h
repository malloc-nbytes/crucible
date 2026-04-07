#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "grammar.h"
#include "lexer.h"
#include "loc.h"
#include "ds/str.h"

typedef struct {
        lexer   *l;
        int      global;
        stmtp_ar stmts;
        struct {
                str msg;
                loc loc;
        } err;
} parser_context;

parser_context parser_parse(lexer *l);

#endif // PARSER_H_INCLUDED
