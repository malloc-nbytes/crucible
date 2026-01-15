#ifndef GLCONF_H_INCLUDED
#define GLCONF_H_INCLUDED

#include <stdint.h>

#define OPT_2HY_SHOW_LEXER "show-lexer"
#define OPT_2HY_SHOW_AST   "show-ast"

typedef enum {
        FT_SHOW_LEXER = 1 << 0,
        FT_SHOW_AST   = 1 << 1,
} flag_type;

extern struct {
        uint32_t flags;
        char *source;
} g_glconf;

#endif // GLCONF_H_INCLUDED
