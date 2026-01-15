#ifndef GLCONF_H_INCLUDED
#define GLCONF_H_INCLUDED

#include <stdint.h>

#define OPT_2HY_DUMP_LEXER "dump-lexer"
#define OPT_2HY_DUMP_AST   "dump-ast"

typedef enum {
        FT_DUMP_LEXER = 1 << 0,
        FT_DUMP_AST   = 1 << 1,
} flag_type;

extern struct {
        uint32_t flags;
        char *source;
} g_glconf;

#endif // GLCONF_H_INCLUDED
