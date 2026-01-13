#ifndef LEX_H_INCLUDED
#define LEX_H_INCLUDED

#include "mem.h"
#include "loc.h"
#include "ds/strv.h"

typedef enum {
        TK_EOF = 0,
        TK_ID,
        TK_STRLIT,
        TK_INTLIT,
        TK_LPAREN,
        TK_RPAREN,
} token_kind;

typedef struct token {
        strv lx;
        token_kind k;
        struct token *n;
        loc loc;
} token;

typedef struct {
        token *hd;
        token *tl;
        const char *fp;
        char *src;
        arena a;
} lexer;

lexer lex_file(const char *path);
void lexer_show(const lexer *l);
void lexer_free(lexer *l);

#endif // LEX_H_INCLUDED
